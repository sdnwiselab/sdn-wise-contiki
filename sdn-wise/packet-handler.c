/*
 * Copyright (C) 2015 SDN-WISE
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 *         SDN-WISE Packet Handler.
 * \author
 *         Sebastiano Milardo <s.milardo@hotmail.it>
 */

/**
 * \addtogroup sdn-wise
 * @{
 */

#include <string.h>
#include <stdio.h>
#include "contiki.h"
#include "dev/watchdog.h"
#include "packet-handler.h"
#include "address.h"
#include "packet-buffer.h"
#include "packet-creator.h"
#include "neighbor-table.h"
#include "flowtable.h"
#include "node-conf.h"
#include "sdn-wise.h"

typedef enum conf_id{
  ID_MY_ADDRESS,
  ID_MY_NET,
  ID_BEACON_PERIOD,
  ID_REPORT_PERIOD,
  ID_UPDATE_TABLE_PERIOD,
  ID_SLEEP_PERIOD,
  ID_MAX_TTL,
  ID_MIN_RSSI,

  ID_ADD_ACCEPTED,
  ID_REMOVE_ACCEPTED,  
  ID_LIST_ACCEPTED,
  ID_ADD_RULE,
  ID_REMOVE_RULE,
  ID_REMOVE_RULE_INDEX,
  ID_GET_RULE_INDEX,
  ID_RESET,
  ID_ADD_FUNCTION,
  ID_REMOVE_FUNCTION,
} conf_id_t;

const uint8_t conf_size[ID_MIN_RSSI+1] = 
{
  sizeof(conf.my_address),          // MY_ADDRESS
  sizeof(conf.my_net),              // MY_NET
  sizeof(conf.beacon_period),       // BEACON_PERIOD
  sizeof(conf.report_period),       // REPORT_PERIOD
  sizeof(conf.update_table_period), // UPDATE_TABLE_PERIOD
  sizeof(conf.sleep_period),        // SLEEP_PERIOD
  sizeof(conf.max_ttl),             // MAX_TTL
  sizeof(conf.min_rssi)             // MIN_RSSI
};

const void* conf_ptr[ID_MIN_RSSI+1] = 
{
  &conf.my_address,           // MY_ADDRESS
  &conf.my_net,               // MY_NET
  &conf.beacon_period,        // BEACON_PERIOD
  &conf.report_period,        // REPORT_PERIOD
  &conf.update_table_period,  // UPDATE_TABLE_PERIOD
  &conf.sleep_period,         // SLEEP_PERIOD
  &conf.max_ttl,              // MAX_TTL
  &conf.min_rssi              // MIN_RSSI
};

#define CNF_READ 0
#define CNF_WRITE 1

#define DEBUG 1
#if DEBUG && (!SINK || DEBUG_SINK)
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*----------------------------------------------------------------------------*/
  static void handle_beacon(packet_t*);
  static void handle_data(packet_t*);
  static void handle_report(packet_t*);
  static void handle_response(packet_t*);
  static void handle_open_path(packet_t*);
  static void handle_config(packet_t*);
/*----------------------------------------------------------------------------*/
  void 
  handle_packet(packet_t* p)
  {
    
    if (p->info.rssi >= conf.min_rssi && p->header.net == conf.my_net){
      if (p->header.typ == BEACON){
        PRINTF("[PHD]: Beacon\n");
        handle_beacon(p);
      } else {
        if (is_my_address(&(p->header.nxh))){
          switch (p->header.typ){
            case DATA:
            PRINTF("[PHD]: Data\n");
            handle_data(p);
            break;

            case RESPONSE:
            PRINTF("[PHD]: Response\n");
            handle_response(p);
            break;

            case OPEN_PATH:
            PRINTF("[PHD]: Open Path\n");
            handle_open_path(p);
            break;

            case CONFIG:
            PRINTF("[PHD]: Config\n");
            handle_config(p);
            break;

            default:
            PRINTF("[PHD]: Request/Report\n");
            handle_report(p);
            break;
          }
        }
      }
    } else {
      packet_deallocate(p);
    }
  }
/*----------------------------------------------------------------------------*/
  void
  handle_beacon(packet_t* p)
  {
    add_neighbor(&(p->header.src),p->info.rssi);
#if !SINK  
  // TODO what if the network changes?
    
    uint8_t new_hops = get_payload_at(p, BEACON_HOPS_INDEX);
    if (new_hops <= conf.hops_from_sink-1 &&
      p->info.rssi > conf.rssi_from_sink)
    {
      conf.nxh_vs_sink = p->header.src;
      conf.hops_from_sink = new_hops+1;
      conf.rssi_from_sink = p->info.rssi;  
      conf.sink_address = p->header.nxh;
    }
#endif
    packet_deallocate(p);
  }
/*----------------------------------------------------------------------------*/
  void
  handle_data(packet_t* p)
  {
    if (is_my_address(&(p->header.dst)))
    {      
      packet_deallocate(p);
    } else {
      match_packet(p);
    }
  }
/*----------------------------------------------------------------------------*/
  void
  handle_report(packet_t* p)
  {
#if SINK
    print_packet_uart(p);
#else 
    
    p->header.nxh = conf.nxh_vs_sink;
    rf_unicast_send(p);
#endif  
  }
/*----------------------------------------------------------------------------*/
  void
  handle_response(packet_t* p)
  {
    if (is_my_address(&(p->header.dst)))
    {    
      entry_t* e = get_entry_from_array(p->payload, p->header.len - PLD_INDEX);
      if (e != NULL)
      {
        add_entry(e);
      }
      packet_deallocate(p);
    } else {
      match_packet(p);
    } 
  }
/*----------------------------------------------------------------------------*/
  void
  handle_open_path(packet_t* p)
  {
    int i;
    
    
    uint8_t n_windows = get_payload_at(p,OPEN_PATH_WINDOWS_INDEX);
    uint8_t start = n_windows*WINDOW_SIZE + 1;
    uint8_t path_len = (p->header.len - (start + PLD_INDEX))/ADDRESS_LENGTH;
    uint8_t my_index = 0;
    uint8_t my_position = 0;
    uint8_t end = p->header.len - PLD_INDEX;
    
    for (i = start; i < end; i += ADDRESS_LENGTH)
    {
      address_t tmp = get_address_from_array(&(p->payload[i]));
      if (is_my_address(&tmp))
      {
        my_index = i;
        break;
      }
      my_position++;
    }


    if (my_position > 0)
    { 
      uint8_t prev = my_index - ADDRESS_LENGTH;
      uint8_t first = start;
      entry_t* e = create_entry();
      
      window_t* w = create_window();
      w->operation = EQUAL;
      w->size = SIZE_2;
      w->lhs = DST_INDEX;
      w->lhs_location = PACKET;
      w->rhs = MERGE_BYTES(p->payload[first], p->payload[first+1]);
      w->rhs_location = CONST;

      add_window(e,w);
      
      for (i = 0; i<n_windows; ++i)
      {
        add_window(e, get_window_from_array(&(p->payload[i*WINDOW_SIZE + 1])));
      }

      action_t* a = create_action(FORWARD_U, &(p->payload[prev]), ADDRESS_LENGTH); 
      add_action(e,a);

      PRINTF("[PHD]: ");
      print_entry(e);
      PRINTF("\n");
      
      add_entry(e);
    }
   
    if (my_position < path_len-1)
    { 
      uint8_t next = my_index + ADDRESS_LENGTH;
      uint8_t last = end - ADDRESS_LENGTH;
      entry_t* e = create_entry();

      window_t* w = create_window();
      w->operation = EQUAL;
      w->size = SIZE_2;
      w->lhs = DST_INDEX;
      w->lhs_location = PACKET;
      w->rhs = MERGE_BYTES(p->payload[last], p->payload[last+1]);
      w->rhs_location = CONST;

      add_window(e,w);
      
      for (i = 0; i<n_windows; ++i)
      {
        add_window(e, get_window_from_array(&(p->payload[i*WINDOW_SIZE + 1])));
      }

      action_t* a = create_action(FORWARD_U, &(p->payload[next]), ADDRESS_LENGTH);
      add_action(e,a);

      PRINTF("[PHD]: ");
      print_entry(e);
      PRINTF("\n");
      
      add_entry(e);

      address_t next_address = get_address_from_array(&(p->payload[next]));
      p->header.nxh = next_address;
      p->header.dst = next_address;
      rf_unicast_send(p);
    }

    if (my_position == path_len-1){
      packet_deallocate(p);
    }
  }
/*----------------------------------------------------------------------------*/
  void
  handle_config(packet_t* p)
  {
    if (is_my_address(&(p->header.dst)))
    {    
#if SINK
      if (!is_my_address(&(p->header.src))){
        print_packet_uart(p);
      } else {
#endif
      uint8_t i = 0;
      uint8_t id = p->payload[i] & 127;
      if ((p->payload[i] & 128) == CNF_READ)
      {
        //READ
        switch (id)
        {
          // TODO
          case ID_LIST_ACCEPTED:
          case ID_GET_RULE_INDEX:
          break;

          // TODO adattare le api alla dimensione variabile del payload
          default:
          memcpy(&(p->payload[i+1]), conf_ptr[id], conf_size[id]);
          break;
        }
        swap_addresses(&(p->header.src),&(p->header.dst));
        match_packet(p);
      } else {
        //WRITE
        switch (id)
        {
          // TODO
          case ID_ADD_ACCEPTED:
          case ID_REMOVE_ACCEPTED:
          case ID_ADD_RULE:
          case ID_REMOVE_RULE:
          case ID_REMOVE_RULE_INDEX:
          case ID_ADD_FUNCTION:
          case ID_REMOVE_FUNCTION:
          break;

          case ID_RESET:
          watchdog_reboot();
          break;

          // TODO adattare le api alla dimensione variabile del payload
          default:
          memcpy((uint8_t*)conf_ptr[id], &(p->payload[i+1]), conf_size[id]);
          break;
        }
        packet_deallocate(p);
      }
#if SINK
    }
#endif      
    }
    else {
      match_packet(p);
    }   
  }
/*----------------------------------------------------------------------------*/
  void 
  test_handle_open_path(void)
  {
    uint8_t array[19] = {
      1, 19, 0, 1, 0, 2, 5, 100, 0, 1, 0, 0, 1, 0, 2, 0, 3, 0, 4,
    };

    packet_t* p = get_packet_from_array(array);
    handle_open_path(p);
  }
/*----------------------------------------------------------------------------*/
/** @} */
