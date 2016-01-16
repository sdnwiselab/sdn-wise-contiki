/*
 * forwarding.h
 *
 *  Created on: 09 nov 2015
 *      Author: Mario Brischetto
 */

#ifndef EXAMPLES_SDN_WISE_FORWARDING_H_
#define EXAMPLES_SDN_WISE_FORWARDING_H_

#include "util.h"
#include "flow_table.h"
#include "header.h"
#include "low_layer.h"
#include "functions.h"

#define FORWARDING_PACKET_TO_SINK_EVENT		101
#define FORWARDING_DATA_PACKET_EVENT		102
#define FORWARDING_RESPONSE_PACKET_EVENT	103
#define FORWARDING_TO_CONTROLLER_EVENT		104

PROCESS_NAME(forwarding_packet_to_sink);
PROCESS_NAME(forwarding_data_packet);
PROCESS_NAME(forwarding_incoming_response_packet);
PROCESS_NAME(forwarding_to_controller);
PROCESS_NAME(forwarding_update_table);

/*************************************************************************/
#define STATUS_LEN	2
static uint8_t status[STATUS_LEN];
uint8_t* getStatus(){
	return status;
}
/*************************************************************************/
typedef struct flow_table_element {
	struct flow_table_element * next;
	rule rule;
} flow_table_element;

typedef struct flow_table {
	flow_table_element * root;
	uint8_t len;
} flow_table;
/*************************************************************************/
static flow_table flowTable;
flow_table getFlowTable(){
	return flowTable;
}
/*************************************************************************/
void flow_table_add (flow_table *t, uint8_t *rule_array){
	/// devo controllare se la regola già esiste
	flow_table_element *e;
	e = flowTable.root;
	int i;
	boolean found = 0;
	uint8_t rule_array_in_list[RULE_LEN];
	for (i=0; i<flowTable.len; i++){
		rule2array(e->rule, rule_array_in_list, 0);
		if (arrayCmp(rule_array_in_list, rule_array, WINDOWS_LEN) == 0){
			found = 1;
			break;
		}
		e = e->next;
	}
	if (found != 1){
		//creo nuovo elemento
		e = malloc(sizeof(flow_table_element));
		array2rule(rule_array, &(e->rule), 0);
		e->rule.stats.ttl = rule_array[WINDOWS_LEN + ACTION_LEN];
		e->rule.stats.count = 0;
		e->next = NULL;

		//aggiungo elemento in coda alla lista
		if (t->root == NULL){
			t->root = e;
		} else {
			flow_table_element *last;
			for (last = t->root; last->next != NULL; last = last->next){
			}
			last->next = e;
		}
		t->len ++;
		//printf("%s: rule add\n", process_current->name);
	} else {
		//printf("%s: rule already exists\n", process_current->name);
	}
}
/*************************************************************************/
void flow_table_remove(flow_table *t, flow_table_element element){
	flow_table_element *e;
	e = t->root;
	if (ruleCmp(element.rule, e->rule, 0) ==0){
		/*
		t->root = NULL;
		free(e);
		*/
		//printf("flow_table_element - eliminato -(1)\n");
		return;
	} else {
		flow_table_element *e_before;
		e_before = e;
		e = e->next;
		int i;
		for (i=1; i<t->len; i++){
			if (ruleCmp(element.rule, e->rule, 0) ==0){
				e_before->next = e->next;
				free(e);
				t->len --;
				//printf("flow_table_element - eliminato (2)\n");
				return;
			}
			e_before = e;
			e = e->next;
		}
	}
	//printf("flow_table_element - problema\n");
}
/*************************************************************************/
void flow_table_remove_all (flow_table *t){
	int i;
	for (i=0; i<t->len; i++){
		flow_table_remove(t, *(t->root->next));
	}
	t->len = 0;
}
/*************************************************************************/

/*************************************************************************/
boolean checkWindow(uint8_t *packet, ruleWindow window){
	uint8_t dim;
	uint8_t value_array[2];

	switch (window.op & (SDN_WISE_SIZE_0 + SDN_WISE_SIZE_1 + SDN_WISE_SIZE_2)){
	case SDN_WISE_SIZE_0:
		return 1;
	case SDN_WISE_SIZE_1:
		dim = 1;
		value_array[0] = window.value_l;
		break;
	case SDN_WISE_SIZE_2:
		dim = 2;
		value_array[0] = window.value_h;
		value_array[1] = window.value_l;
		break;
	}
	uint8_t data[dim];

	switch (window.op & (SDN_WISE_PACKET + SDN_WISE_STATUS)){
	case SDN_WISE_PACKET:
		if (window.pos > packet[HEADER_LEN_POS] -1) return 1;
		split(packet, data, window.pos, dim);
		break;
	case SDN_WISE_STATUS:
		if (window.pos > STATUS_LEN -1) return 1;
		split(status, data, window.pos, dim);
		break;
	}

	uint8_t result;
	result = arrayCmp(data, value_array, dim);

	switch (window.op & (7<<3)){
	case SDN_WISE_EQUAL:
		if (result == 0) return 1;
		break;
	case SDN_WISE_NOT_EQUAL:
		if (result != 0) return 1;
		break;
	case SDN_WISE_BIGGER:
		if (result > 0) return 1;
		break;
	case SDN_WISE_LESS:
		if (result < 0) return 1;
		break;
	case SDN_WISE_EQUAL_OR_BIGGER:
		if (result >= 0) return 1;
		break;
	case SDN_WISE_EQUAL_OR_LESS:
		if (result <= 0) return 1;
		break;
	}
	return 0;
}
/*************************************************************************/
boolean runAction(uint8_t *packet, ruleAction action){ //return = FALSE (NO_MULTI), TRUE (MULTI)
	static uint8_t *data;
	int x = rand() % 256;

	switch (action.act & 1){
	case SDN_WISE_PACKET:
		data = packet;
		break;
	case SDN_WISE_STATUS:
		data = status;
		break;
	}

	switch (action.act & (7<<2)){
	case SDN_WISE_FORWARD_UNICAST:
		//printf("RUN ACTION - forward unicast\n");
		data[HEADER_NEXT_HOP_POS] = action.value_h;
		data[HEADER_NEXT_HOP_POS +1] = action.value_l;
		data[HEADER_TTL_POS] = data[HEADER_TTL_POS] --;
		send_packet2(data);
		break;
	case SDN_WISE_FORWARD_BROADCAST:
		//printf("RUN ACTION - forward broadcast\n");
		data[HEADER_NEXT_HOP_POS] = BROADCAST.addr_h;
		data[HEADER_NEXT_HOP_POS +1] = BROADCAST.addr_l;
		data[HEADER_DST_POS] = BROADCAST.addr_h;	//////////////////////////
		data[HEADER_DST_POS +1] = BROADCAST.addr_l;	//////////////////////////
		data[HEADER_TTL_POS] = data[HEADER_TTL_POS] --;
		send_packet2(data);
		break;
	case SDN_WISE_DROP:
		//printf("RUN ACTION - drop\n");
		if (x < action.value_l){
			//scarta
			//printf("RUN ACTION - droppato\n");
			return 0;
		} else {
			//printf("RUN ACTION - non droppato\n");
			return 1;
		}
		break;
	case SDN_WISE_MODIFY:
		//printf("RUN ACTION - modify\n");
		/////////////////////////////// la dim?
		data[action.pos] = action.value_h;
		data[action.pos +1] = action.value_l;
		break;
	case SDN_WISE_AGGREGATE:
		//printf("RUN ACTION - aggregate\n");
		///////////////////////////////////////////////////////
		break;
	case SDN_WISE_FORWARD_UP:
		//printf("RUN ACTION - forward up\n");
		forward_up(data);
		break;
	}

	if ((action.act & 2) == SDN_WISE_MULTI){
		//printf("RUN ACTION - multi\n");
		return 1;
	} else {
		//printf("RUN ACTION - no multi\n");
		return 0;
	}
}
/*************************************************************************/
#define RULE2SINK_TTL_NO_DEC	255
/*************************************************************************/
void updateStats(ruleStats *stats){
	stats->count ++;
	return;
}
/*************************************************************************/

/*************************************************************************/
rule flow_table_new_default_rule(){
	int i;
	rule r;
	for (i=0; i<WINDOWS_MAX; i++){
		r.windows[i].op = SDN_WISE_PACKET + SDN_WISE_SIZE_0 + SDN_WISE_EQUAL;
		r.windows[i].pos = HEADER_DST_POS;
		r.windows[i].value_h = 0;
		r.windows[i].value_l = 0;
	}
	r.action.act = SDN_WISE_PACKET + SDN_WISE_MULTI + SDN_WISE_DROP;
	r.action.value_h = 0;
	r.action.value_l = 255;
	r.stats.ttl = RULE2SINK_TTL_NO_DEC;
	r.stats.count = 0;
	return r;
}
/*************************************************************************/
static rule * rule2sink;
/*************************************************************************/
static boolean flow_table_init_var = 0;
void flow_table_init(){
	if (flow_table_init_var == 0){
		rule r;
		r = flow_table_new_default_rule();
		r.windows[0].op = SDN_WISE_PACKET + SDN_WISE_SIZE_2 + SDN_WISE_EQUAL;
		uint8_t tmp[RULE_LEN];
		rule2array(r, tmp, 1);
		flow_table_add(&flowTable, tmp);

		(*(flowTable.root)).rule.stats.ttl = RULE2SINK_TTL_NO_DEC;
		rule2sink = &((*(flowTable.root)).rule);
		flow_table_init_var = 1;
		//printf("FORWARDING - flowTable init\n");
	}
}
/*************************************************************************/
void update_rule2sink(address sink, address next_hop){
	rule2sink->windows[0].op = SDN_WISE_PACKET + SDN_WISE_SIZE_2 + SDN_WISE_EQUAL;
	rule2sink->windows[0].pos = HEADER_DST_POS;
	rule2sink->windows[0].value_h = sink.addr_h;
	rule2sink->windows[0].value_l = sink.addr_l;
	rule2sink->action.act = SDN_WISE_PACKET + SDN_WISE_NOT_MULTI + SDN_WISE_FORWARD_UNICAST;
	rule2sink->action.value_h = next_hop.addr_h;
	rule2sink->action.value_l = next_hop.addr_l;
	rule2sink->stats.count = 0;
	//printf("FORWARDING - rule2sink updated\n");
}
/*************************************************************************/

void forwarding2controller(uint8_t *packet){
	process_post(&forwarding_to_controller, FORWARDING_TO_CONTROLLER_EVENT, packet);
}
/*************************************************************************/



#endif /* EXAMPLES_SDN_WISE_FORWARDING_H_ */
