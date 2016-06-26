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
 *         SDN-WISE Packet Creator.
 * \author
 *         Sebastiano Milardo <s.milardo@hotmail.it>
 */

/**
 * \addtogroup sdn-wise
 * @{
 */
#include <stdio.h>

#include "dev/battery-sensor.h"
#include "packet-buffer.h"
#include "packet-creator.h"
#include "node-conf.h"
#include "address.h"
#include "neighbor-table.h"
#include "sdn-wise.h"

#define DEBUG 1
#if DEBUG && (!SINK || DEBUG_SINK)
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*----------------------------------------------------------------------------*/
packet_t* 
create_beacon(void)
{
  packet_t* p = create_packet_empty();
  if (p != NULL){
    p->header.net = conf.my_net;
    set_broadcast_address(&(p->header.dst));
    p->header.src = conf.my_address; 
    p->header.typ = BEACON;
    p->header.nxh = conf.sink_address;
  
    set_payload_at(p, BEACON_HOPS_INDEX, conf.hops_from_sink);
    
    SENSORS_ACTIVATE(battery_sensor);
    set_payload_at(p, BEACON_BATT_INDEX, battery_sensor.value(0));
    SENSORS_DEACTIVATE(battery_sensor);
  }
  return p;
}
/*----------------------------------------------------------------------------*/
packet_t* 
create_data(void)
{
  // TODO 
  return NULL;
}
/*----------------------------------------------------------------------------*/
packet_t* 
create_report(void)
{  
  packet_t* p = create_packet_empty();
  if (p != NULL){
    p->header.net = conf.my_net;
    p->header.dst = conf.sink_address;
    p->header.src = conf.my_address; 
    p->header.typ = REPORT;
    p->header.nxh = conf.nxh_vs_sink;
    
    set_payload_at(p, BEACON_HOPS_INDEX, conf.hops_from_sink);
                
    SENSORS_ACTIVATE(battery_sensor);
    set_payload_at(p, BEACON_BATT_INDEX, battery_sensor.value(0));
    SENSORS_DEACTIVATE(battery_sensor);

    fill_payload_with_neighbors(p);
  }
  return p;
}
/*----------------------------------------------------------------------------*/
packet_t* 
create_reg_proxy(void)
{  
  uint8_t payload[] = {
    48, 48, 48, 48, 48, 48, 48, 49,
     0,  1,  2,  3,  4,  5,  0,  0,
     0,  0,  0,  0,  0,  1,-64,-88,
     1, 108, 39, 6
  }; 

  packet_t* p = create_packet_payload(
    conf.my_net, 
    &conf.sink_address, 
    &conf.my_address, 
    REG_PROXY, 
    &conf.nxh_vs_sink,
    payload, 
    28);
  return p;
}
/*----------------------------------------------------------------------------*/
void
create_and_send_request(packet_t* p)
{
  
  uint8_t i = 0;    
    
  if (p->header.len < MAX_PAYLOAD_LENGTH){  
    packet_t* r = create_packet_empty();
    if (r != NULL){
      r->header.net = conf.my_net;
      r->header.dst = conf.sink_address;
      r->header.src = conf.my_address; 
      r->header.typ = REQUEST;
      r->header.nxh = conf.nxh_vs_sink;

      uint8_t* a = (uint8_t*)p;
      set_payload_at(r, 0, conf.requests_count);
      set_payload_at(r, 1, 0);
      set_payload_at(r, 2, 1);     
      for (i = 0; i < (p->header.len); ++i){
        set_payload_at(r, i+3, a[i]);
      }
    rf_unicast_send(r);
    conf.requests_count++;

    }
  } else {   
    packet_t* r1 = create_packet_empty();
    packet_t* r2 = create_packet_empty();
    
    if (r1 != NULL && r2 != NULL){
      r1->header.net = conf.my_net;
      r1->header.dst = conf.sink_address;
      r1->header.src = conf.my_address; 
      r1->header.typ = REQUEST;
      r1->header.nxh = conf.nxh_vs_sink;
      
      r2->header.net = conf.my_net;
      r2->header.dst = conf.sink_address;
      r2->header.src = conf.my_address; 
      r2->header.typ = REQUEST;
      r2->header.nxh = conf.nxh_vs_sink;

      set_payload_at(r1, 0, conf.requests_count);
      set_payload_at(r1, 1, 0);
      set_payload_at(r1, 2, 2);

      set_payload_at(r2, 0, conf.requests_count);
      set_payload_at(r2, 1, 1);
      set_payload_at(r2, 2, 2);     
      
      uint8_t* a = (uint8_t*)p;     
      for (i = 0; i < MAX_PAYLOAD_LENGTH; ++i){
        set_payload_at(r1, i+3, a[i]);
      }

      for (i = 0; i < (p->header.len - MAX_PAYLOAD_LENGTH); ++i){
        set_payload_at(r2, i+3, a[i + MAX_PAYLOAD_LENGTH]);
      }

      rf_unicast_send(r1);
      rf_unicast_send(r2);

      conf.requests_count++;
    } else {
      if (r1 == NULL){
        packet_deallocate(r1);
      } else {
        packet_deallocate(r2);
      }
    }
  }
  packet_deallocate(p); 
}
/*----------------------------------------------------------------------------*/
packet_t* 
create_config(void)
{
  // TODO 
  return NULL;
}
/*----------------------------------------------------------------------------*/
/** @} */
