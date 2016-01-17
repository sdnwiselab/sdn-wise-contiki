/*
 *  low_layer.h
 *
 *  Created on: 21 nov 2015
 *      Author: Mario Brischetto
 */

#ifndef _SDN_WISE_LOW_LAYER_H_
#define _SDN_WISE_LOW_LAYER_H_
/*************************************************************************/
#include <stdio.h>
#include "net/rime/rime.h"
#include "net/netstack.h"
#include "header.h"
#include "contiki.h"
#include "topology_discovery.h"
#include "forwarding.h"
#include "functions.h"
#include "configure.h"
/*************************************************************************/
#define LOW_LAYER_UNICAST_CONNECTION_NUMBER 29
#define LOW_LAYER_BROADCAST_CONNECTION_NUMBER 30
#define SEND_PACKET_EVENT	50
#define FORWARD_UP_EVENT	51
/*************************************************************************/
PROCESS_NAME(low_layer_process);
PROCESS_NAME(low_layer_forward_up);
/*************************************************************************/
typedef struct accepted_element {
	struct accepted_element * next;
	address address;
} accepted_element;
typedef struct accepted_table {
	accepted_element * root;
	uint8_t len;
} accepted_table;
/*************************************************************************/
static accepted_table acceptedTable;
static boolean accepted_table_init_var = 0;
/*************************************************************************/
void send_packet2 (uint8_t *packet_array);
void forward_up (uint8_t *packet);
/*************************************************************************/
void send_packet (uint8_t len, uint8_t net_id, address src, address dst, uint8_t type, uint8_t ttl, address next_hop, uint8_t payload[]){
    static uint8_t packet_array[PACKET_LEN];
    header h;
    h.len = len;
    h.net_id = net_id;
    h.src = src;
    h.dst = dst;
    h.type = type;
    h.ttl = ttl;
    h.next_hop = next_hop;
    header2array(h, packet_array);
    int i;
    for (i=HEADER_LENGTH; i<h.len; i++) {
        packet_array[i] = payload[i - HEADER_LENGTH];
    }
    process_post(&low_layer_process, SEND_PACKET_EVENT ,packet_array);
}
/*************************************************************************/
void send_packet2 (uint8_t *packet_array){
	process_post(&low_layer_process, SEND_PACKET_EVENT ,packet_array);
}
/*************************************************************************/
void forward_up (uint8_t *packet){
	process_post(&low_layer_forward_up, FORWARD_UP_EVENT, packet);
}

/*************************************************************************/
accepted_table* getAcceptedTable(){
	return &acceptedTable;
}
/*************************************************************************/
void accepted_table_add(accepted_table *t, address address){
	accepted_element *e;
	e = malloc(sizeof(accepted_element));
	e->address = address;
	e->next = NULL;

	if (t->root == NULL){
		t->root = e;
	} else {
		accepted_element *last;
		for (last = t->root; last->next != NULL; last = last->next){
		}
		last->next = e;
	}
	t->len ++;
}
/*************************************************************************/
boolean accepted_table_search(accepted_table t, address address){
	accepted_element *e;
	e = t.root;
	int i;
	for (i=0; i<t.len; i++){
		if (addressCmp(address, e->address)==0 ){
			return 1;
		}
		e = e->next;
	}
	return 0;
}
/*************************************************************************/
void accepted_table_remove(accepted_table *t, address address){
	accepted_element *e;
	e = t->root;
	if (addressCmp(address, e->address)==0){
		/*
		t->root = NULL;
		free(e);
		*/
		cf.MY_ADDRESS->addr_h = linkaddr_node_addr.u8[0];
		cf.MY_ADDRESS->addr_l = linkaddr_node_addr.u8[1];
		return;
	} else {
		accepted_element *e_before;
		e_before = e;
		e = e->next;
		int i;
		for (i=2; i<t->len; i++){	//0=SRC, 1=BROADCAST
			if (addressCmp(address, e->address)==0){
				e_before->next = e->next;
				free(e);
				t->len --;
				return;
			}
			e_before = e;
			e = e->next;
		}
		return;
	}
}
/*************************************************************************/
void accepted_table_init(){
	if (accepted_table_init_var == 0){
		address myself;
		myself.addr_h = linkaddr_node_addr.u8[0];
		myself.addr_l = linkaddr_node_addr.u8[1];
		accepted_table_add(&acceptedTable, myself);
		cf.MY_ADDRESS = &((*(acceptedTable.root)).address);
		accepted_table_add(&acceptedTable, BROADCAST);
		accepted_table_init_var = 1;
	}
}
/*************************************************************************/
static void recv_packet(struct unicast_conn *c, const linkaddr_t *from, uint8_t data[]) {
	static uint8_t * packet;
	if (c==NULL) {
		packet = data;
	} else {
		packet = (uint8_t *)packetbuf_dataptr();
		if ( ((uint8_t) (- packetbuf_attr(PACKETBUF_ATTR_RSSI))) < cf.RSSI_MIN) return;
	}
    header h;
    uint8_t tmp[HEADER_LENGTH];
    split(packet, tmp, 0, HEADER_LENGTH);
    array2header(tmp, &h);
    if (h.net_id != cf.NET_ID) return;
    uint8_t payload[h.len - HEADER_LENGTH];
    int i;
    for (i=0; i<h.len - HEADER_LENGTH; i++) {
        payload[i] = packet[i+HEADER_LENGTH];
    }

#if !SINK
    printf("LOW_LAYER: print new packet\n");
    printHeader(h);
    printArray(payload, h.len - HEADER_LENGTH);
#endif

    switch (h.type){
    case DATA:
    	if (accepted_table_search(acceptedTable, h.next_hop)) {
    		if (accepted_table_search(acceptedTable, h.dst)) {
#if !SINK
    			printf("LOW_LAYER - DATA : dst = myself - forward up\n");
#endif
    			if (functionsRoot.len > 0) {
    				struct process * p;
    				p = functionsRoot.root->process;
    				process_post(p, FORWARD_UP_PROCESS_EVENT, packet);
    			} else {
    				process_post(&low_layer_forward_up, FORWARD_UP_EVENT, packet);
    			}
    		} else {
#if !SINK
    			printf("LOW_LAYER: DATA packet received \n");
#endif
    			process_post(&forwarding_data_packet, FORWARDING_DATA_PACKET_EVENT, packet);
    		}
    	} else {
#if !SINK
    		printf("LOW_LAYER - DATA - error: next_hop = other\n");
#endif
    	}
    	break;
    case BEACON:
#if !SINK
    		printf("LOW_LAYER: BEACON packet received\n");
#endif
    		process_post(&td_incoming_beacon_packet, INCOMING_BEACON_PACKET_EVENT, packet);
    	break;
    case REPORT:
    	if (accepted_table_search(acceptedTable, h.next_hop)) {
    		if (accepted_table_search(acceptedTable, h.dst)) {
    			process_post(&forwarding_to_controller, FORWARDING_TO_CONTROLLER_EVENT, packet);
    		} else {
#if !SINK
    			printf("LOW_LAYER: REPORT packet received\n");
#endif
    			process_post(&forwarding_packet_to_sink, FORWARDING_PACKET_TO_SINK_EVENT, packet);
    		}
    	} else {
#if !SINK
    		printf("LOW_LAYER - REPORT - error: next_hop = other\n");
#endif
    	}
    	break;
    case RESPONSE:
    	if (accepted_table_search(acceptedTable, h.next_hop)) {
    		if (accepted_table_search(acceptedTable, h.dst)) {
#if !SINK
    			printf("LOW_LAYER: RESPONSE packet received\n");
#endif
    			process_post(&forwarding_incoming_response_packet, FORWARDING_PACKET_TO_SINK_EVENT, packet);
    		} else {
#if !SINK
    			printf("LOW_LAYER: RESPONSE packet forwarding\n");
#endif
    			process_post(&forwarding_data_packet, FORWARDING_DATA_PACKET_EVENT, packet);
    		}
    	} else {
#if !SINK
    		printf("LOW_LAYER - RESPONSE - error: next_hop = other\n");
#endif
    	}
    	break;
    case OPEN_PATH:
    	if (accepted_table_search(acceptedTable, h.next_hop)
    			&& accepted_table_search(acceptedTable, h.dst)){
#if !SINK
    		printf("LOW_LAYER: OPEN_PATH packet received\n");
#endif
    		//aggiungo le regole
    		uint8_t n_windows;
    		uint8_t tmp[RULE_LEN];
    		n_windows = payload[0];
    		for (i=0; i<n_windows; i++){
    			split(payload, tmp, i*(RULE_LEN -1) + 1, RULE_LEN -1);
    			flow_table_add(&flowTable, tmp);
    		}

    		//creo lista degli indirizzi
    		int n_addresses;
    		n_addresses = (h.len - HEADER_LENGTH - 1 - (n_windows * (RULE_LEN-1))) / ADDRESS_LENGHT;
    		uint8_t addr_array_tmp[ADDRESS_LENGHT];
    		address addr_list[n_addresses];
    		uint8_t index = -1;
    		for (i=0; i<n_addresses; i++){
    			split(payload, addr_array_tmp, (n_windows * (RULE_LEN-1)) + 1 + i*ADDRESS_LENGHT, ADDRESS_LENGHT);
    			array2address(addr_array_tmp, &(addr_list[i]));
    			if (addressCmp(*(cf.MY_ADDRESS), addr_list[i]) ==0) index = i;	//if (addressCmp(*SRC, addr_list[i]) ==0) index = i;
    		}

    		rule rule;
    		uint8_t rule_array[RULE_LEN];
    		//regola di andata
    		if (index != n_addresses -1){	//se non sono l'ultimo
    			rule = flow_table_new_default_rule();
    			rule.windows[0].op = SDN_WISE_PACKET + SDN_WISE_SIZE_2 + SDN_WISE_EQUAL;
    			rule.windows[0].pos = HEADER_DST_POS;
    			rule.windows[0].value_h = addr_list[n_addresses -1].addr_h;
    			rule.windows[0].value_l = addr_list[n_addresses -1].addr_l;
    			rule.action.act = SDN_WISE_PACKET + SDN_WISE_NOT_MULTI + SDN_WISE_FORWARD_UNICAST;
    			rule.action.value_h = addr_list[index +1].addr_h;
    			rule.action.value_l = addr_list[index +1].addr_l;
    			rule.stats.ttl = RULE2SINK_TTL_NO_DEC -1;
    			rule2array(rule, rule_array, 1);
    			flow_table_add(&flowTable, rule_array);
    		}
    		//regola di ritorno
    		if (index != 0){	//se non sono il primo
    			rule = flow_table_new_default_rule();
    			rule.windows[0].op = SDN_WISE_PACKET + SDN_WISE_SIZE_2 + SDN_WISE_EQUAL;
    			rule.windows[0].pos = HEADER_DST_POS;
    			rule.windows[0].value_h = addr_list[0].addr_h;
    			rule.windows[0].value_l = addr_list[0].addr_l;
    			rule.action.act = SDN_WISE_PACKET + SDN_WISE_NOT_MULTI + SDN_WISE_FORWARD_UNICAST;
    			rule.action.value_h = addr_list[index -1].addr_h;
    			rule.action.value_l = addr_list[index -1].addr_l;
    			rule.stats.ttl = RULE2SINK_TTL_NO_DEC -1;
    			rule2array(rule, rule_array, 1);
    			flow_table_add(&flowTable, rule_array);
    		}

    		//inoltro
    		if (index != n_addresses -1){
    			h.dst = addr_list[index +1];
    			h.next_hop = addr_list[index +1];
    			header2array(h, packet);
    			send_packet2(packet);
    		}
    	} else {
#if !SINK
    		printf("LOW_LAYER - RESPONSE - error: next_hop = other\n");
#endif
    	}
    	break;
    case CONFIG:
    	if (accepted_table_search(acceptedTable, h.next_hop)) {
    		if (accepted_table_search(acceptedTable, h.dst)) {
#if !SINK
    			printf("LOW_LAYER: CONFIG packet received\n");
#endif
    			process_post(&config_incoming_packet, CONFIG_PACKET_EVENT, packet);
    		} else {
#if !SINK
    			printf("LOW_LAYER: CONFIG packet forwarding\n");
#endif
    			process_post(&forwarding_data_packet, FORWARDING_DATA_PACKET_EVENT, packet);
    		}
    	} else {
#if !SINK
    		printf("LOW_LAYER - CONFIG - error: next_hop = other\n");
#endif
    	}

    	break;
    case (REQUEST+DATA):
    case (REQUEST+RESPONSE):
    case (REQUEST+CONFIG):
		if (accepted_table_search(acceptedTable, h.next_hop)) {
			if (!addressCmp(sink_address, h.next_hop)) {
#if !SINK
				printf("LOW_LAYER - REQUEST :  sono il sink, inoltra al controller?\n");
#endif
				process_post(&forwarding_to_controller, FORWARDING_TO_CONTROLLER_EVENT, packet);
			} else {
#if !SINK
				printf("LOW_LAYER: REQUEST packet received\n");
#endif
				process_post(&forwarding_packet_to_sink, FORWARDING_PACKET_TO_SINK_EVENT, packet);
			}
		} else {
#if !SINK
			printf("LOW_LAYER - REQUEST - error: next_hop = other\n");
#endif
		}
    	break;
    }

}
/*************************************************************************/
static void unicast_recv_packet(struct unicast_conn *c, const linkaddr_t *from, uint8_t data[]){
#if !SINK
	printf("LOW_LAYER : unicast message received from %d.%d\n", from->u8[0], from->u8[1]);
#endif
	recv_packet(c, from, data);
}
/*************************************************************************/
static void broadcast_recv_packet(struct unicast_conn *c, const linkaddr_t *from, uint8_t data[]){
#if !SINK
	printf("LOW_LAYER : broadcast message received from %d.%d\n", from->u8[0], from->u8[1]);
#endif
	recv_packet(c, from, data);
}
/*************************************************************************/
#endif // _SDN_WISE_LOW_LAYER_H_
