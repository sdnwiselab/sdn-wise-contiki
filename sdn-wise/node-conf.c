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
 *         SDN-WISE Node Configurations.
 * \author
 *         Sebastiano Milardo <s.milardo@hotmail.it>
 */

/**
 * \addtogroup sdn-wise
 * @{
 */

#include <string.h>

#include "address.h"
#include "node-conf.h" 
#include "net/rime/rime.h"

#ifndef SDN_WISE_DEBUG
#define SDN_WISE_DEBUG 0
#endif
#if SDN_WISE_DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*----------------------------------------------------------------------------*/
  node_conf_t conf;
/*----------------------------------------------------------------------------*/
  void
  node_conf_init(void)
  {

#if COOJA
    conf.my_address.u8[1] = linkaddr_node_addr.u8[0]; 
    conf.my_address.u8[0] = linkaddr_node_addr.u8[1];
#else
    conf.my_address = get_address_from_int(_MY_ADDRESS);
    linkaddr_node_addr.u8[1] = conf.my_address.u8[0];
    linkaddr_node_addr.u8[0] = conf.my_address.u8[1]; 
#endif
    conf.requests_count = 0;
    conf.my_net = _NET;
    conf.beacon_period = _BEACON_PERIOD;
    conf.report_period = _REPORT_PERIOD;
    conf.rule_ttl = _RULE_TTL;
    conf.rssi_min = _RSSI_MIN;
    conf.packet_ttl = _PACKET_TTL;
#if SINK
    conf.is_active = 1;
    conf.nxh_vs_sink = conf.my_address;
    conf.sink_address = conf.my_address;
    conf.distance_from_sink = _MIN_DISTANCE;
    conf.hops_from_sink = 0;
#else
    conf.is_active = 0;
    set_broadcast_address(&(conf.nxh_vs_sink));
    set_broadcast_address(&(conf.sink_address));
    conf.distance_from_sink = _MAX_DISTANCE;
    conf.hops_from_sink = _PACKET_TTL;
    conf.reset_period = _RESET_PERIOD;
#endif 
  }
/*----------------------------------------------------------------------------*/
  void
  print_node_conf(void){
    PRINTF("[CFG]: NODE: ");
    print_address(&(conf.my_address));
    PRINTF("\n");
    PRINTF("[CFG]: - Network ID: %d\n[CFG]: - Beacon Period: %d\n[CFG]: - "
      "Report Period: %d\n[CFG]: - Rules TTL: %d\n[CFG]: - Min RSSI: "
      "%d\n[CFG]: - Packet TTL: %d\n[CFG]: - Next Hop -> Sink: ",
      conf.my_net, conf.beacon_period, conf.report_period, 
      conf.rule_ttl, conf.rssi_min, conf.packet_ttl);
    print_address(&(conf.nxh_vs_sink));
    PRINTF(" (hops: %d, distance: %d)\n", conf.hops_from_sink, conf.distance_from_sink);
    PRINTF("[CFG]: - Sink: ");
    print_address(&(conf.sink_address));
    PRINTF("\n");
  }
/*----------------------------------------------------------------------------*/
/** @} */
