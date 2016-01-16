//
//  topology_discovery.h
//  Prova
//
//  Created by Mario Brischetto on 25/10/15.
//  Copyright (c) 2015 Mario Brischetto. All rights reserved.
//

#ifndef __Prova__topology_discovery__
#define __Prova__topology_discovery__

#include <stdio.h>
#include "neighbor_table.h"
#include "lib/list.h"
#include "dev/cc2420/cc2420.h"
#include "dev/battery-sensor.h"

PROCESS_NAME(td_incoming_beacon_packet);
PROCESS_NAME(td_send_beacon_packet);
PROCESS_NAME(td_recv_report_packet);

#define INCOMING_BEACON_PACKET_EVENT		100
/*************************************************************************/

#define NEIGHBOR_TO_SINK_DFLT_ADDRESS_H   0
#define NEIGHBOR_TO_SINK_DFLT_ADDRESS_L   0
#define NEIGHBOR_TO_SINK_DFLT_RSSI      0
#define NEIGHBOR_TO_SINK_DFLT_BATT      255
#define NEIGHBOR_TO_SINK_DFLT_NHOP      255

//Payload Beacon Packet
#define NHOP_POS    0
#define BATT_POS    1
#define BEACON_LEN	2

static short_neighbor_table neighbor_table;
static boolean neighbor_table_init = 0;
static neighbor neighbor2sink;
static address	sink_address;
/*************************************************************************/
int find_best_neighbor(neighbor n1, neighbor n2){
	n1.nhop = 255 - n1.nhop;
	n1.batt = 0;
	n2.nhop = 255 - n2.nhop;
	n2.batt = 0;

    uint8_t n1a[NEIGHBOR_LEN];
    neighbor2array(n1, n1a, 1);
    uint8_t n2a[NEIGHBOR_LEN];
    neighbor2array(n2, n2a, 1);

    uint8_t n1_a[NEIGHBOR_LEN];
    uint8_t n2_a[NEIGHBOR_LEN];
    int i;
    for (i=0; i<NEIGHBOR_LEN; i++) {
        n1_a[i] = n1a[NEIGHBOR_LEN -1 -i];
        n2_a[i] = n2a[NEIGHBOR_LEN -1 -i];
    }
    return arrayCmp(n1_a, n2_a, NEIGHBOR_LEN);
}
/*************************************************************************/


/*************************************************************************/
neighbor getNeighbor2sink(){
    return neighbor2sink;
}
/*************************************************************************/
short_neighbor_table getNeighborTable(){
    return neighbor_table;
}
/*************************************************************************/
void neighbor2sink_reset(){
	neighbor2sink.rssi = NEIGHBOR_TO_SINK_DFLT_RSSI;
	neighbor2sink.batt = NEIGHBOR_TO_SINK_DFLT_BATT;
	neighbor2sink.nhop = NEIGHBOR_TO_SINK_DFLT_NHOP;
neighbor2sink.address.addr_h = 0;
neighbor2sink.address.addr_l = 0;
}
/*************************************************************************/
void neighbor2sink_init(){
	neighbor2sink.address.addr_h = NEIGHBOR_TO_SINK_DFLT_ADDRESS_H;
	neighbor2sink.address.addr_l = NEIGHBOR_TO_SINK_DFLT_ADDRESS_L;
	neighbor2sink.rssi = NEIGHBOR_TO_SINK_DFLT_RSSI;
	neighbor2sink.batt = NEIGHBOR_TO_SINK_DFLT_BATT;
	neighbor2sink.nhop = NEIGHBOR_TO_SINK_DFLT_NHOP;
}
/*************************************************************************/
void table_init(){
    if (neighbor_table_init == 0) {
    	neighbor_table.root_table = NULL;
    	neighbor_table.table_len = 0;

    	neighbor2sink_init();

        neighbor_table_init = 1;
    }
}
/*************************************************************************/



#endif /* defined(__Prova__topology_discovery__) */
