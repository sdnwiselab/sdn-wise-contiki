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
  RESET,
  MY_NET,
  MY_ADDRESS,
  PACKET_TTL,
  RSSI_MIN,
  BEACON_PERIOD,
  REPORT_PERIOD,
  RESET_PERIOD,
  RULE_TTL,
  ADD_ALIAS,
  REM_ALIAS,  
  GET_ALIAS,
  ADD_RULE,
  REM_RULE,
  GET_RULE,
  ADD_FUNCTION,
  REM_FUNCTION,
  GET_FUNCTION
} conf_id_t;

const uint8_t conf_size[RULE_TTL+1] = 
{
  0,
  sizeof(conf.my_net),              
  sizeof(conf.my_address),          
  sizeof(conf.packet_ttl),
  sizeof(conf.rssi_min),
  sizeof(conf.beacon_period),       
  sizeof(conf.report_period),  
  sizeof(conf.reset_period),       
  sizeof(conf.rule_ttl)
};

const void* conf_ptr[RULE_TTL+1] = 
{
  NULL,
  &conf.my_net,              
  &conf.my_address,          
  &conf.packet_ttl,
  &conf.rssi_min,
  &conf.beacon_period,       
  &conf.report_period, 
  &conf.reset_period,      
  &conf.rule_ttl,              
};

#define CNF_READ 0
#define CNF_WRITE 1

#ifndef SDN_WISE_DEBUG
#define SDN_WISE_DEBUG 0
#endif
#if SDN_WISE_DEBUG
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
    if (p->info.rssi >= conf.rssi_min && p->header.net == conf.my_net){
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
    uint8_t new_hops = get_payload_at(p, BEACON_HOPS_INDEX);
    uint8_t new_distance = p->info.rssi;

    if (address_cmp(&(conf.nxh_vs_sink), &(p->header.src)) ||
#if MOBILE
       (new_distance < conf.distance_from_sink)
#else
       (new_hops <= conf.hops_from_sink-1 && new_distance < conf.distance_from_sink)
#endif
    )
    {
      conf.nxh_vs_sink = p->header.src;
      conf.distance_from_sink = new_distance;
      conf.sink_address = p->header.nxh;
      conf.hops_from_sink = new_hops+1;
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
      PRINTF("[PHD]: Consuming Packet...\n");
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
    int my_index = -1;
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

    if (my_index == -1){
	printf("[PHD]: Nothing to learn, matching...\n");
	match_packet(p);
    } else {
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
          case RESET:
          case GET_ALIAS:
          case GET_FUNCTION:
          break;

	  case GET_RULE:
	    p->header.len += get_array_from_entry_id(&p->payload[i+2],p->payload[i+1]);
          break;

          case MY_NET:
          case MY_ADDRESS:
          case PACKET_TTL:
          case RSSI_MIN:
          case BEACON_PERIOD:
          case REPORT_PERIOD:
          case RULE_TTL:
          // TODO check payload size
          if (conf_size[id] == 1){
            memcpy(&(p->payload[i+1]), conf_ptr[id], conf_size[id]);
          } else if (conf_size[id] == 2) {
            uint16_t value = *((uint16_t*)conf_ptr[id]);
            p->payload[i+1] = value >> 8;
            p->payload[i+2] = value & 0xFF; 
          }
	  p->header.len += conf_size[id];
          break;


          default:
          break;
        }
        swap_addresses(&(p->header.src),&(p->header.dst));      
#if !SINK
        match_packet(p);
#else
	print_packet_uart(p);
#endif
      } else {
        //WRITE
        switch (id)
        {
          // TODO
          case ADD_ALIAS:
          case REM_ALIAS:
          case ADD_RULE:
          case REM_RULE:
          case ADD_FUNCTION:
          case REM_FUNCTION:
          break;

          case RESET:
          watchdog_reboot();
          break;

          case MY_NET:
          case MY_ADDRESS:
          case PACKET_TTL:
          case RSSI_MIN:
          case BEACON_PERIOD:
          case REPORT_PERIOD:
          case RESET_PERIOD:
          case RULE_TTL:
          if (conf_size[id] == 1){
            memcpy((uint8_t*)conf_ptr[id], &(p->payload[i+1]), conf_size[id]);
          } else if (conf_size[id] == 2) {
            uint16_t h = p->payload[i+1] << 8;
            uint16_t l = p->payload[i+2];            
            *((uint16_t*)conf_ptr[id]) = h + l;
          }
          break;

          default:
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
