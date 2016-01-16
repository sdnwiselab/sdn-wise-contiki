//
//  main.c
//  Prova
//
//  Created by Mario Brischetto on 21/10/15.
//  Copyright (c) 2015 Mario Brischetto. All rights reserved.
//

#include <stdio.h>
#include "contiki.h"
#include "net/rime/rime.h"
#include "dev/watchdog.h"
#include "cfs/cfs.h"
#include "loader/elfloader.h"

#include "configure.h"
#include "header.h"
#include "low_layer.h"
#include "topology_discovery.h"
#include "forwarding.h"
#include "inout.h"
#include "functions.h"

#include "sniffer.h"

/*************************************************************************/
PROCESS(init_process, "Init Process");
PROCESS(low_layer_process, "Low Layer - Process");
PROCESS(low_layer_forward_up, "Low Layer - Forward Up");
PROCESS(low_layer_analyzer, "low Layer - Analyzer");
PROCESS(config_incoming_packet, "Config - Incoming Packet");
PROCESS(td_incoming_beacon_packet, "TD - Incoming Beacon Packet");
PROCESS(td_send_beacon_packet, "TD - Send Beacon Packet");
PROCESS(td_send_report_packet, "TD - Send Report Packet");
PROCESS(forwarding_packet_to_sink, "Forwarding Packet to Sink");
PROCESS(forwarding_data_packet, "Forwarding Data Packet");
PROCESS(forwarding_incoming_response_packet, "Forwarding Incoming Reponse Packet");
PROCESS(forwarding_to_controller, "Forwarding to Controller");
PROCESS(forwarding_update_table, "Forwarding Update Table");
PROCESS(test_serial, "Test Serial Process");
PROCESS(prova, "Prova");
AUTOSTART_PROCESSES(
		&init_process,
		&low_layer_process,
		&low_layer_forward_up,
		&low_layer_analyzer,
		&config_incoming_packet,
		&td_incoming_beacon_packet,
		&td_send_beacon_packet,
		&td_send_report_packet,
		&forwarding_packet_to_sink,
		&forwarding_data_packet,
		&forwarding_incoming_response_packet,
		&forwarding_to_controller,
		&forwarding_update_table,
		&test_serial,
		&prova);
/*************************************************************************/
static const struct unicast_callbacks unicast_callbacks = {unicast_recv_packet};
static struct unicast_conn unicast_connection;
static const struct broadcast_callbacks broadcast_callbaks = {broadcast_recv_packet};
static struct broadcast_conn broadcast_connection;
/*************************************************************************/
PROCESS_THREAD(init_process, ev, data) {
	PROCESS_BEGIN();
	conf_init();		//inizializza cf
	function_list_root_init();	//inizializza functionListRoot
	table_init();		//inizializza neighbor table
	flow_table_init();	//inizializza flow table
	accepted_table_init();	//inizializza accepted table
	input_stream_init();	//inizializza inputStream
	elfloader_init();
	PROCESS_END();
}
/*************************************************************************/
PROCESS_THREAD(low_layer_process, ev, data) {
	PROCESS_EXITHANDLER(unicast_close(&unicast_connection);)
    PROCESS_BEGIN();
    unicast_open(&unicast_connection, LOW_LAYER_UNICAST_CONNECTION_NUMBER, &unicast_callbacks);
    broadcast_open(&broadcast_connection, LOW_LAYER_BROADCAST_CONNECTION_NUMBER, &broadcast_callbaks);
    static uint8_t *packet;
    while (1) {
        static linkaddr_t addr;
        PROCESS_WAIT_EVENT();
        if (ev == SEND_PACKET_EVENT) {
        	packet = data;
        	addr.u8[0] = packet[HEADER_NEXT_HOP_POS];
        	addr.u8[1] = packet[HEADER_NEXT_HOP_POS +1];
        	printf("%s: try to send packet\n", process_current->name);
        	printArray(packet, packet[HEADER_LEN_POS]);
        	//
        	printf("hl %d %d\n",packet[0], packet[HEADER_LEN_POS]);
        	if (packet[HEADER_NEXT_HOP_POS] == 0 && packet[HEADER_NEXT_HOP_POS +1] == 0) {
        	        		printf("%s: nhop 0.0 no!\n", process_current->name);
        	        		continue;
        	}
        	//
        	if (packet[HEADER_TTL_POS]>0){
        		address dst = {packet[HEADER_DST_POS], packet[HEADER_DST_POS +1]};
        		packetbuf_copyfrom(packet, packet[HEADER_LEN_POS]);
        		if (addressCmp(dst, BROADCAST) != 0){
        		    if(!linkaddr_cmp(&addr, &linkaddr_node_addr)) {
        		    	unicast_send(&unicast_connection, &addr);
        		    }
        		} else {
        			broadcast_send(&broadcast_connection);
        		}
        		printf("%s: packet sended\n", process_current->name);
        	} else {
        		printf("%s: packet ttl=0\n", process_current->name);
        	}
        }
    }
    PROCESS_END();
}
/*************************************************************************/
PROCESS_THREAD(low_layer_forward_up, ev, data){
	static uint8_t *packet;
	PROCESS_BEGIN();
	while (1){
		PROCESS_WAIT_EVENT_UNTIL(ev == FORWARD_UP_EVENT);
		packet = data;
		printf("%s: new packet\n", process_current->name);
		printArray(packet, packet[HEADER_LEN_POS]);
		//////////////////////////////////////////////////////
	}
	PROCESS_END();
}
/*************************************************************************/
PROCESS_THREAD(low_layer_analyzer, ev, data){
	static uint8_t *packet;
	PROCESS_BEGIN();
	while(1){
		PROCESS_WAIT_EVENT_UNTIL(ev == ANALYZER_EVENT);
		packet = data;
		uint8_t len = packet[0];
		uint8_t l2p[len];
		uint8_t i;
		for (i=0; i<len-1; i++){
			l2p[i] = packet[i+1];
		}
printf("Sniffer0...");
printArray(l2p, len);
		uint8_t l2ps;
		l2ps=0;
		boolean fcfIntraPAN;
		fcfIntraPAN = ((l2p[0] >> 6) & 0x01) != 0;
		uint8_t fcfDestAddrMode;
		fcfDestAddrMode = (l2p[1] >> 2) & 0x03;
		uint8_t fcfSrcAddrMode;
		fcfSrcAddrMode = (l2p[1] >> 6) & 0x03;

		if (fcfDestAddrMode > 0){
			l2ps +=2;
			if (fcfDestAddrMode == 2){
				l2ps +=2;
			} else if (fcfDestAddrMode == 3){
				l2ps +=8;
			} else continue;
		}
		if (fcfSrcAddrMode > 0){
			if (!fcfIntraPAN){
				l2ps +=2;
			}
			if (fcfSrcAddrMode == 2){
				l2ps +=2;
			} else if (fcfSrcAddrMode == 3){
				l2ps +=8;
			} else continue;
		}

		l2ps +=3; //+FCF+SeqN
printf("l2ps=%d, l2p[l2ps]=%d\n", l2ps, l2p[l2ps]);
		if (l2p[l2ps] > 63){
//printf("l2ps=%d, l2p[l2ps]=%d\n", l2ps, l2p[l2ps]);
			printf("%s: is a 6LoWPAN packet\n", process_current->name);
			header h;
			h.len = HEADER_LENGTH + len +3;
			h.net_id = cf.NET_ID;
			h.src = *(cf.MY_ADDRESS);
			//h.dst.addr_h = 0;
			//h.dst.addr_l = 0;
			h.dst = sink_address;
			h.type = REQUEST + DATA;
			h.ttl = cf.TTL_MAX;
			h.next_hop = neighbor2sink.address;
			static uint8_t new_packet[PACKET_LEN];
			header2array(h, new_packet);
//
			new_packet[10] = 1;
			new_packet[11] = 0;
			new_packet[12] = 1;
			//
			for (i=HEADER_LENGTH +3; i < h.len; i++){
				new_packet[i] = l2p[i - HEADER_LENGTH -3];
			}
printf("Sniffer...");
printArray(new_packet, new_packet[HEADER_LEN_POS]);
			if ((cf.MY_ADDRESS->addr_h == 1) && (cf.MY_ADDRESS->addr_l == 0)){	//////////////IF SINK
				forwarding2controller(new_packet);
			} else {															/////////////////////
				send_packet2(new_packet);
			}
		} else {
			printf("%s: is not a 6LoWPAN packet\n", process_current->name);
			continue;
		}

	}
	PROCESS_END();
}
/*************************************************************************/

/*************************************************************************/

/*************************************************************************/
PROCESS_THREAD(config_incoming_packet, ev, data){
	static uint8_t *packet;
	PROCESS_BEGIN();
	while(1){
		PROCESS_WAIT_EVENT_UNTIL(ev == CONFIG_PACKET_EVENT);
		packet = data;
		header h;
		array2header(packet, &h);
		printf("%s: new packet\n", process_current->name);
		int i = HEADER_LENGTH;
		uint8_t new_payload[h.len - HEADER_LENGTH];
		uint8_t i_new_payload = 0;

		//
		uint8_t new_payload_la[acceptedTable.len * 2];
		uint8_t new_payload_gri[3 + RULE_LEN];
		uint8_t tmp[RULE_LEN];
		uint8_t tmp2[RULE_LEN];
		uint8_t tmp_array[PACKET_LEN];
		accepted_element *ae;
		flow_table_element * fe;
		uint8_t j, k, nrule;
		address a;
		//

		while (i<h.len){
			if (packet[i] & 128 == CNF_READ){
				//READ
				switch (packet[i] & 127){
				case CNF_ID_ADDR:
					printf("%s - READ - ID ADDR\n", process_current->name);
					new_payload[i_new_payload] = packet[i];
					new_payload[i_new_payload + 1] = (cf.MY_ADDRESS)->addr_h;
					new_payload[i_new_payload + 2] = (cf.MY_ADDRESS)->addr_l;
					i_new_payload = i_new_payload + 3;
					i = i + 3;
					break;
				case CNF_ID_NET_ID:
					printf("%s - READ - ID NET\n", process_current->name);
					new_payload[i_new_payload] = packet[i];
					new_payload[i_new_payload + 1] = 0;
					new_payload[i_new_payload + 2] = cf.NET_ID;
					i_new_payload = i_new_payload + 3;
					i = i + 3;
					break;
				case CNF_ID_CNT_BEACON_MAX:
					printf("%s - READ - BEACON MAX\n", process_current->name);
					new_payload[i_new_payload] = packet[i];
					new_payload[i_new_payload + 1] = 0;
					new_payload[i_new_payload + 2] = cf.SEND_BEACON_PACKET_INTERVALL;
					i_new_payload = i_new_payload + 3;
					i = i + 3;
					break;
				case CNF_ID_CNT_REPORT_MAX:
					printf("%s - READ - REPORT MAX\n", process_current->name);
					new_payload[i_new_payload] = packet[i];
					new_payload[i_new_payload + 1] = 0;
					new_payload[i_new_payload + 2] = cf.SEND_REPORT_PACKET_INTERVALL;
					i_new_payload = i_new_payload + 3;
					i = i + 3;
					break;
				case CNF_ID_CNT_UPDTABLE_MAX:
					printf("%s - READ - UPDTABLE MAX\n", process_current->name);
					new_payload[i_new_payload] = packet[i];
					new_payload[i_new_payload + 1] = 0;
					new_payload[i_new_payload + 2] = cf.UPDTABLE_MAX;
					i = i + 3;
					break;
				case CNF_ID_TTL_MAX:
					printf("%s - READ - TTL MAX\n", process_current->name);
					new_payload[i_new_payload] = packet[i];
					new_payload[i_new_payload + 1] = 0;
					new_payload[i_new_payload + 2] = cf.TTL_MAX;
					i_new_payload = i_new_payload + 3;
					i = i + 3;
					break;
				case CNF_ID_RSSI_MIN:
					printf("%s - READ - RSSI MAX\n", process_current->name);
					new_payload[i_new_payload] = packet[i];
					new_payload[i_new_payload + 1] = 0;
					new_payload[i_new_payload + 2] = cf.RSSI_MIN;
					i_new_payload = i_new_payload + 3;
					i = i + 3;
					break;
				case CNF_LIST_ACCEPTED:
					k=0;
					ae = acceptedTable.root;
					for (j=0; j<acceptedTable.len; j++){
						new_payload_la[k] = ae->address.addr_h;
						new_payload_la[k+1] = ae->address.addr_l;
						k++;
						ae = ae->next;
					}
					send_packet(
							HEADER_LENGTH + (acceptedTable.len*2),
							cf.NET_ID,
							*cf.MY_ADDRESS,
							sink_address,
							CONFIG,
							cf.TTL_MAX,
							neighbor2sink.address,
							new_payload_la);
					i = i + 3;
					break;
				case CNF_GET_RULE_INDEX:
					new_payload_gri[0] = packet[i];
					new_payload_gri[1] = packet[i + 1];
					new_payload_gri[2] = packet[i + 2];
					int jji, jj;
					jji = (packet[i +1] << 8) | packet[i + 2];
					fe = flowTable.root;
					for (jj=0; jj<=jji && jji < flowTable.len; jj++){
						fe = fe->next;
					}
					rule2array(fe->rule, tmp, 1);
					for (k=0; k<RULE_LEN; k++){
						new_payload_gri[k + 3] = tmp[k];
					}
					send_packet(
							HEADER_LENGTH + (3 + RULE_LEN),
							cf.NET_ID,
							*cf.MY_ADDRESS,
							sink_address,
							CONFIG,
							cf.TTL_MAX,
							neighbor2sink.address,
							new_payload_gri);
					i = i + 3;
					break;
				}
			} else {
				//WRITE
				printf("%s - WRITE\n", process_current->name);
				switch (packet[i] & 127){
				case CNF_ID_ADDR:
					printf("%s - WRITE - ID ADDR\n", process_current->name);
					cf.MY_ADDRESS->addr_h = packet[i +1];
					cf.MY_ADDRESS->addr_l = packet[i +2];
					//////////////////////////////////////// set rime address
					linkaddr_t linkaddr;
					linkaddr.u8[0] = cf.MY_ADDRESS->addr_h;
					linkaddr.u8[1] = cf.MY_ADDRESS->addr_l;
					linkaddr_set_node_addr(&linkaddr);
					i = i + 3;
					break;
				case CNF_ID_NET_ID:
					printf("%s - WRITE - ID NET\n", process_current->name);
					cf.NET_ID = packet[i +2];
					i = i + 3;
					break;
				case CNF_ID_CNT_BEACON_MAX:
					printf("%s - WRITE - BEACON MAX\n", process_current->name);
					cf.SEND_BEACON_PACKET_INTERVALL = packet[i +2];
					i = i + 3;
					break;
				case CNF_ID_CNT_REPORT_MAX:
					printf("%s - WRITE - REPORT MAX\n", process_current->name);
					cf.SEND_REPORT_PACKET_INTERVALL = packet[i + 2];
					i = i + 3;
					break;
				case CNF_ID_CNT_UPDTABLE_MAX:
					printf("%s - WRITE - UPDTABLE MAX\n", process_current->name);
					cf.UPDTABLE_MAX = packet[i + 2];
					i = i + 3;
					break;
				case CNF_ID_TTL_MAX:
					printf("%s - WRITE - TTL MAX\n", process_current->name);
					cf.TTL_MAX = packet[i + 2];
					i = i + 3;
					break;
				case CNF_ID_RSSI_MIN:
					printf("%s - WRITE - RSSI MAX\n", process_current->name);
					cf.RSSI_MIN = packet[i + 2];
					i = i + 3;
					break;
				case CNF_ADD_ACCEPTED:
					printf("%s - WRITE - ADD ACCEPTED\n", process_current->name);
					a.addr_h = packet[i + 1];
					a.addr_l = packet[i + 2];
					accepted_table_add(&acceptedTable, a);
					i = i + 3;
					break;
				case CNF_REMOVE_ACCEPTED:
					printf("%s - WRITE - REMOVE ACCEPTED\n", process_current->name);
					a.addr_h = packet[i + 1];
					a.addr_l = packet[i + 2];
					accepted_table_remove(&acceptedTable, a);
					i = i + 3;
					break;
				case CNF_ADD_RULE:
					printf("%s - WRITE - ADD RULE\n", process_current->name);
					nrule = packet[i + 1];
					i = i + 2;
					for (j = 0; j<nrule ; j++){
						split(packet, tmp, i+j*(RULE_LEN), RULE_LEN);
						flow_table_add(&flowTable, tmp);
						i = i + RULE_LEN;
					}
					break;
				case CNF_REMOVE_RULE:
					printf("%s - WRITE - REMOVE RULE\n", process_current->name);
					nrule = packet[i + 1];
					i = i + 2;
					for (j=0; j<nrule; j++){
						split(packet, tmp, i+j*(RULE_LEN), RULE_LEN);
						fe = flowTable.root;
						for (k=0; k<flowTable.len; k++){
							rule2array(fe->rule, tmp2, 0);
							if (arrayCmp(tmp2, tmp, 0) == 0){
								flow_table_remove(&flowTable, *fe);
								break;
							}
							fe = fe->next;
						}
						i = i + RULE_LEN;
					}
					break;
				case CNF_REMOVE_RULE_INDEX:
					printf("%s - WRITE - REMOVE RULE INDEX\n", process_current->name);
					fe = flowTable.root;
					for (k=0; k<packet[i + 2]; k++){
						fe = fe->next;
					}
					flow_table_remove(&flowTable, *fe);
					i = i + 3;
					break;
				case CNF_RESET:
					printf("%s - WRITE - RESET\n", process_current->name);
					watchdog_reboot();
					break;
				case CNF_ADD_FUNCTION:
					printf("%s - WRITE - ADD FUNCTION\n", process_current->name);
					uint8_t id_h;
					id_h = packet[i + 1];
					uint8_t id_l;
					id_l = packet[i + 2];
					uint8_t position;
					position = packet[i + 3];
					uint8_t total_elements;
					total_elements = packet[i + 4];

					//cerca o crea nuovo function_root
					function_root *fr;
					fr = function_list_root_search_by_id(id_h, id_l);
					if (fr == NULL){
						fr = function_list_root_new_element(id_h, id_l, total_elements);
					}

					//aggiungi elemento alla function
					split(packet, tmp_array, HEADER_LENGTH + 5, h.len - HEADER_LENGTH - 5);
					function_root_add_element(fr, position, tmp_array, h.len - HEADER_LENGTH - 5);

					//se ho ricevuto tutta la funzione
					if (fr->elements_number == fr->total_elements){
						int fd;
						char file[16];
						sprintf(file, "f_%d_%d.ce", fr->id_h, fr->id_l);
						function_root_compose_packet(fr);

						/* Kill any old processes. */
						  if(elfloader_autostart_processes != NULL) {
						    autostart_exit(elfloader_autostart_processes);
						  }

						  fd = cfs_open(file, CFS_READ | CFS_WRITE);
						  if(fd < 0) {
							printf("exec: could not open %s\n", file);
						  } else {
						    int ret;
						    char *print, *symbol;

						    ret = elfloader_load(fd);
						    cfs_close(fd);
						    symbol = "";

						    switch(ret) {
						    case ELFLOADER_OK:
						      print = "OK";
						      break;
						    case ELFLOADER_BAD_ELF_HEADER:
						      print = "Bad ELF header";
						      break;
						    case ELFLOADER_NO_SYMTAB:
						      print = "No symbol table";
						      break;
						    case ELFLOADER_NO_STRTAB:
						      print = "No string table";
						      break;
						    case ELFLOADER_NO_TEXT:
						      print = "No text segment";
						      break;
						    case ELFLOADER_SYMBOL_NOT_FOUND:
						      print = "Symbol not found: ";
						      symbol = elfloader_unknown;
						      break;
						    case ELFLOADER_SEGMENT_NOT_FOUND:
						      print = "Segment not found: ";
						      symbol = elfloader_unknown;
						      break;
						    case ELFLOADER_NO_STARTPOINT:
						      print = "No starting point";
						      break;
						    default:
						      print = "Unknown return code from the ELF loader (internal bug)";
						      break;
						    }
							printf("%s %s\n", print, symbol);

						    if(ret == ELFLOADER_OK) {
						      int i;
						      for(i = functionsRoot.len; elfloader_autostart_processes[i] != NULL; ++i) {
						    	  printf("exec: starting process %s\n",
									 elfloader_autostart_processes[i]->name);
						    	  functions_root_add(fr->id_h, fr->id_l, elfloader_autostart_processes[i]);
						      }
						      autostart_start(elfloader_autostart_processes);
						      cfs_remove(file);
						    }

						  }
						function_list_root_remove_element_by_id(id_h, id_l);
					}
					i = i + h.len - HEADER_LENGTH;
					////
					printf("functionsRoot - LEN = %d\n", functionsRoot.len);
					if (functionsRoot.len > 0){
						process_post(functionsRoot.root->process, FORWARD_UP_PROCESS_EVENT, packet);
					}
					////
					break;
				case CNF_REMOVE_FUNCTION:
					printf("%s - WRITE - REMOVE FUNCTION\n", process_current->name);
					break;
				}
			}
		}
		if (i_new_payload > 0) {
			send_packet(
					HEADER_LENGTH + i_new_payload,
					cf.NET_ID,
					*cf.MY_ADDRESS,
					sink_address,
					CONFIG,
					cf.TTL_MAX,
					neighbor2sink.address,
					new_payload);
		}
	}
	PROCESS_END();
}
/*************************************************************************/

/*************************************************************************/

/*************************************************************************/
PROCESS_THREAD(td_incoming_beacon_packet, ev, data){
    PROCESS_BEGIN();
    static uint8_t *packet;
    while (1) {
        PROCESS_WAIT_EVENT();
        if (ev == INCOMING_BEACON_PACKET_EVENT) {
        	packet = data;
            header header;
            uint8_t headerarray[HEADER_LENGTH];
            split(packet, headerarray, 0, HEADER_LENGTH);
            array2header(packet, &header);
            uint8_t payload[header.len - HEADER_LENGTH];
            split(packet, payload, HEADER_LENGTH, header.len - HEADER_LENGTH);
            printf("%s: new packet\n", process_current->name);

            neighbor new_neighbor;
            new_neighbor.address.addr_h = header.src.addr_h;
            new_neighbor.address.addr_l = header.src.addr_l;
            new_neighbor.rssi = (uint8_t) (- packetbuf_attr(PACKETBUF_ATTR_RSSI));    //// -80:0 -> 0:80
        /*http://sourceforge.net/p/contiki/mailman/message/31805752/*//////////////////////////////////////
            new_neighbor.batt = payload[BATT_POS];
            new_neighbor.nhop = payload[NHOP_POS];

            if (new_neighbor.rssi >= cf.RSSI_MIN ) {

            	printTable(neighbor_table);

            	//aggiorno la neighbor_table
            	short_neighbor_element *sne = find_element_by_address(neighbor_table, new_neighbor.address);
            	if (sne != NULL) {
            		if ((sne->array)[2] != new_neighbor.rssi){
            			neighbor2array(new_neighbor, sne->array, 0);
            	        printf("%s: aggiorno neighbor\n", process_current->name);
            		}
            	} else {
            		uint8_t tmp[NEIGHBOR_LEN -2];
            	    neighbor2array(new_neighbor, tmp, 0);
            	    neighbor_table_add(&neighbor_table, tmp);
            	    printf("%s: aggiungo nuovo neighbor\n", process_current->name);
            	}

            	printTable(neighbor_table);

            	printNeighbor(neighbor2sink, 1);

            	//aggiorno il neighbor2sink
            	if (addressCmp(header.next_hop, *cf.MY_ADDRESS)!=0 && addressCmp(header.next_hop, BROADCAST)!=0 ){
            		if (find_best_neighbor(new_neighbor, neighbor2sink) > 0) {
            			// aggiorna neighbor2sink e flow_table
            	        neighbor2sink = new_neighbor;
            	        sink_address = header.next_hop;
            	        printf("%s: aggiorno neighbor2sink\n", process_current->name);
            	        update_rule2sink(header.next_hop, header.src);
            		} else printf("%s: Non aggiorno neighbor2sink\n", process_current->name);
            		printNeighbor(neighbor2sink, 1);
            	}

            } else {
            	printf("%s: rssi < RSSI_MIN\n", process_current->name);
            }

            ///
            int i;
            flow_table_element *e;
            e=flowTable.root;
            for (i=0; i<flowTable.len; i++){
            	printRule(e->rule, 1);
            	e = e->next;
            }
            ///
        }
    }
    PROCESS_END();
}
/*************************************************************************/
PROCESS_THREAD(td_send_beacon_packet, ev, data){
	static struct etimer et;
    PROCESS_BEGIN();
    etimer_set(&et, cf.SEND_BEACON_PACKET_INTERVALL * CLOCK_SECOND);
    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        if (neighbor2sink.nhop < 255){
        	uint8_t payload[BEACON_LEN];
        	payload[NHOP_POS] = neighbor2sink.nhop + 1;
        	SENSORS_ACTIVATE(battery_sensor);
        	payload[BATT_POS] = battery_sensor.value(0);	////////////////////////////
        	SENSORS_DEACTIVATE(battery_sensor);
        	header h;
        	h.len = HEADER_LENGTH + BEACON_LEN;
        	h.net_id = cf.NET_ID;
        	h.src = *cf.MY_ADDRESS;
        	h.dst = BROADCAST;
        	h.type = BEACON;
        	h.ttl = cf.TTL_MAX;
        	h.next_hop = sink_address;
        	printf("%s: send beacon packet\n", process_current->name);
        	send_packet(h.len, h.net_id, h.src, h.dst, h.type, h.ttl, h.next_hop, payload);
        }
        etimer_set(&et, cf.SEND_BEACON_PACKET_INTERVALL * CLOCK_SECOND);
    }
    PROCESS_END();
}
/*************************************************************************/
PROCESS_THREAD(td_send_report_packet, ev, data){
	static struct etimer et;
    PROCESS_BEGIN();
    etimer_set(&et, cf.SEND_REPORT_PACKET_INTERVALL * CLOCK_SECOND);
    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        if (neighbor_table.table_len > 0) {
        	uint8_t payload[3 + (neighbor_table.table_len * 3)];
        	payload[0] = neighbor2sink.nhop;
        	SENSORS_ACTIVATE(battery_sensor);
        	payload[1] = battery_sensor.value(0);	////////////////////////////
        	SENSORS_DEACTIVATE(battery_sensor);
        	payload[2] = neighbor_table.table_len;

        	int k=3;
        	short_neighbor_element *e;
        	e = neighbor_table.root_table;
        	int i, j;
        	for (i=0; i<neighbor_table.table_len; i++) {
        		for (j=0; j < NEIGHBOR_LEN -2; j++){
        			payload[k + j] = (e->array)[j];
        	        }
        		k = k + NEIGHBOR_LEN -2;
        	    e = e->next;
        	}

        	header h;
        	h.len = HEADER_LENGTH + 3 + (neighbor_table.table_len * 3);
        	h.net_id = cf.NET_ID;
        	h.src = *cf.MY_ADDRESS;
        	h.dst = sink_address;
        	h.type = REPORT;
        	h.ttl = cf.TTL_MAX;
        	h.next_hop = neighbor2sink.address;
        	printf("%s: send report packet\n", process_current->name);
        	send_packet(h.len, h.net_id, h.src, h.dst, h.type, h.ttl, h.next_hop, payload);
        	neighbor_table_remove_all(&neighbor_table);
        	//neighbor2sink_reset();
        	/////////////////////////////////////////// if == SINK
        	if (cf.MY_ADDRESS->addr_h == 1 && cf.MY_ADDRESS->addr_l == 0){
        		neighbor2sink.nhop = 0;
        		sink_address = *cf.MY_ADDRESS;	//sink_address = *SRC;
        	    //
        	    static uint8_t packet[PACKET_LEN];
        	    header2array(h, packet);
        	    int i;
        	    for (i=0; i<h.len; i++){
        	    	packet[i + HEADER_LENGTH] = payload[i];
        	    }
        	    forwarding2controller(packet);
        	    //
        	}
        	//////////////////////////////////////////
        }
        etimer_set(&et, cf.SEND_REPORT_PACKET_INTERVALL * CLOCK_SECOND);
    }
    PROCESS_END();
}
/*************************************************************************/

/*************************************************************************/

/*************************************************************************/
PROCESS_THREAD(forwarding_packet_to_sink, ev, data){
	static uint8_t *packet;
	PROCESS_BEGIN();
	while (1){
		PROCESS_WAIT_EVENT_UNTIL(ev == FORWARDING_PACKET_TO_SINK_EVENT);
		packet = data;
		header h;
		array2header(packet, &h);
		printf("%s: from %d.%d\n", process_current->name, h.src.addr_h, h.src.addr_l);
		h.next_hop = neighbor2sink.address;
		h.ttl --;
		updateStats(&(flowTable.root->rule.stats));
		header2array(h, packet);
		send_packet2(packet);
	}
	PROCESS_END();
}
/*************************************************************************/
PROCESS_THREAD(forwarding_data_packet, ev, data){
	static uint8_t *packet;
	static flow_table_element *e;
	static boolean found = 0;
	static boolean multi = 0;
	PROCESS_BEGIN();
	while (1){
		PROCESS_WAIT_EVENT_UNTIL(ev == FORWARDING_DATA_PACKET_EVENT);
		packet = data;
		header h;
		array2header(packet, &h);
		int i, j;
		e = flowTable.root;
		for (i=0; i<flowTable.len; i++){
			for (j=0; j<WINDOWS_MAX; j++){
				if (checkWindow(packet, e->rule.windows[j]) == 1){
					found = 1;
				} else {
					found = 0;
					break;
				}
			}
			if (found == 1){
				printf("%s: rule founded\n", process_current->name);
				multi = runAction(packet, e->rule.action);
				updateStats(&(e->rule.stats));
				if (multi == 1){
					printf("%s: multi = TRUE\n", process_current->name);
					i=0;
					e = flowTable.root;
				}
			} else {
				e = e->next;
			}
		}
		if (found == 0){
			printf("%s: send REQUEST packet\n", process_current->name);
			h.type = h.type + REQUEST;
			h.next_hop = neighbor2sink.address;
			h.ttl = cf.TTL_MAX;
			header2array(h, packet);

				/////////////////////////////////////////// if == SINK
			if (cf.MY_ADDRESS->addr_h == 1 && cf.MY_ADDRESS->addr_l == 0){
				forwarding2controller(packet);
				///////////////////////////////////////////
			} else {
				send_packet2(packet);
			}
		}
	}
	PROCESS_END();
}
/*************************************************************************/
PROCESS_THREAD(forwarding_incoming_response_packet, ev, data){
	static uint8_t *packet;
	PROCESS_BEGIN();
	while(1){
		PROCESS_WAIT_EVENT_UNTIL(ev == FORWARDING_RESPONSE_PACKET_EVENT);
		packet = data;
		header h;
		array2header(packet, &h);
		uint8_t nrul = (h.len - HEADER_LENGTH) / (RULE_LEN -1);
		if ((nrul * (RULE_LEN -1)) + HEADER_LENGTH == h.len){
			static uint8_t *tmp;
			int i, j;
			for (i=0, j=0; i<nrul; i++, j=j+RULE_LEN-1){
				split(packet, tmp, HEADER_LENGTH + j, RULE_LEN -1);
				flow_table_add(&flowTable, tmp);
			}
		} else {
			printf("%s: packet error\n", process_current->name);
		}

	}
	PROCESS_END();
}
/*************************************************************************/
PROCESS_THREAD(forwarding_to_controller, ev, data){
	static uint8_t *packet;
	PROCESS_BEGIN();
	while (1){
		PROCESS_WAIT_EVENT_UNTIL(ev == FORWARDING_TO_CONTROLLER_EVENT);
		packet = data;
		int i;
		for (i=0; i<packet[HEADER_LEN_POS]; i++){
			printf("%c", packet[i]);
		}
		//printf("\n");
		//printf("%s: new packet\n", process_current->name);
		//printArray(packet, packet[HEADER_LEN_POS]);
		///////////////////////////////////////////////////////////////
	}
	PROCESS_END();
}
/*************************************************************************/
PROCESS_THREAD(forwarding_update_table, ev, data){
	static struct etimer et;
	PROCESS_BEGIN();
	etimer_set(&et, cf.UPDTABLE_MAX * CLOCK_SECOND);
	while (1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		flow_table_element *e;
		e = flowTable.root;
		int i;
		for (i=0; i<flowTable.len; i++){
			if (e->rule.stats.ttl != RULE2SINK_TTL_NO_DEC){
				printf("%s: ttl update\n", process_current->name);
				e->rule.stats.ttl--;
			}
			e = e->next;
		}
		etimer_set(&et, cf.UPDTABLE_MAX * CLOCK_SECOND);
	}
	PROCESS_END();
}
/*************************************************************************/

/*************************************************************************/

/*************************************************************************/
PROCESS_THREAD(test_serial, ev, data){
	static uint8_t packet[PACKET_LEN];
	static struct etimer et;
	PROCESS_BEGIN();
	etimer_set(&et, CLOCK_SECOND / 100);
	uint8_t x;
	while(1){
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		if (inputStream.len > 0){
			static int i;
			/////
			for (i=0; i<=HEADER_LEN_POS; i++){
				x = input_stream_pop(&inputStream);
				packet[i] = x;
				etimer_set(&et, CLOCK_SECOND / 100);
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			}
			/////
			for (i=HEADER_LEN_POS + 1; i<packet[HEADER_LEN_POS]; i++){
				x = input_stream_pop(&inputStream);
				packet[i] = x;
				etimer_set(&et, CLOCK_SECOND / 100);
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			}
				recv_packet(NULL, NULL, packet);
		}
		etimer_set(&et, CLOCK_SECOND / 100);
	}
	PROCESS_END();
}
/*************************************************************************/
PROCESS_THREAD(prova, ev, data) {
    PROCESS_BEGIN();
    if (cf.MY_ADDRESS->addr_h == 1 && cf.MY_ADDRESS->addr_l == 0){
    	neighbor2sink.nhop = 0;
    	cf.MY_ADDRESS->addr_h = 1;	//
    	cf.MY_ADDRESS->addr_l = 0;	//
    	//////////////////////////////////////// set rime address
    	linkaddr_t linkaddr;
    	linkaddr.u8[0] = cf.MY_ADDRESS->addr_h;
    	linkaddr.u8[1] = cf.MY_ADDRESS->addr_l;
    	linkaddr_set_node_addr(&linkaddr);
    	sink_address = *cf.MY_ADDRESS;
    	neighbor2sink.address = *cf.MY_ADDRESS;
    } else {
    	//neighbor2sink.nhop = rand() % 256;
    }
    printf("neighbor2sink n hop: %d\n", neighbor2sink.nhop);
    PROCESS_END();
}
/*************************************************************************/
