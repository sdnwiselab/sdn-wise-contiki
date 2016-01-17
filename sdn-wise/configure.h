/*
 *  configure.h
 *
 *  Created on: 09 nov 2015
 *      Author: Mario Brischetto
 */

#ifndef _SDN_WISE_CONFIGURE_H_
#define _SDN_WISE_CONFIGURE_H_
/*************************************************************************/
#include "header.h"
#include <string.h>
#include "cfs/cfs.h"
/*************************************************************************/
#define CONFIG_PACKET_EVENT				52
#define DFLT_NET_ID					1;
#define DFLT_SEND_BEACON_PACKET_INTERVALL		5;
#define DFLT_SEND_REPORT_PACKET_INTERVALL		21;
#define DFLT_UPDTABLE_MAX				1;
#define DFLT_TTL_MAX					100;
#define DFLT_RSSI_MIN					0;
#define CNF_READ					0
#define CNF_WRITE					1
#define CNF_ID_ADDR					0
#define CNF_ID_NET_ID					1
#define CNF_ID_CNT_BEACON_MAX				2		
#define CNF_ID_CNT_REPORT_MAX				3		
#define CNF_ID_CNT_UPDTABLE_MAX				4		
#define CNF_ID_CNT_SKEEP_MAX				5		
#define CNF_ID_TTL_MAX					6		
#define CNF_ID_RSSI_MIN					7		
#define CNF_ADD_ACCEPTED				8		
#define CNF_REMOVE_ACCEPTED				9		
#define CNF_LIST_ACCEPTED				10		
#define CNF_ADD_RULE					11		
#define CNF_REMOVE_RULE					12		
#define CNF_REMOVE_RULE_INDEX				13		
#define CNF_GET_RULE_INDEX				14		
#define CNF_RESET					15		
#define CNF_ADD_FUNCTION				16		
#define CNF_REMOVE_FUNCTION				17		
/*************************************************************************/
PROCESS_NAME(config_incoming_packet);
/*************************************************************************/
typedef struct conf {
	address * MY_ADDRESS;
	uint8_t NET_ID;
	uint8_t SEND_BEACON_PACKET_INTERVALL;
	uint8_t SEND_REPORT_PACKET_INTERVALL;
	uint8_t UPDTABLE_MAX;
	uint8_t TTL_MAX;
	uint8_t RSSI_MIN;
} conf;
typedef struct function_element {
	uint8_t position;
	//uint8_t array[PACKET_LEN];
	uint8_t len_array;
	struct function_element * next;
} function_element;
typedef struct function_root {
	uint8_t id_h;
	uint8_t id_l;
	uint8_t elements_number;
	uint8_t total_elements;
	int total_bytes;
	function_element * root;
} function_root;
typedef struct function_list_element{
	function_root * functionRoot;
	struct function_list_element * next;
} function_list_element;
typedef struct function_list_root{
	uint8_t len;
	function_list_element *root_list;
} function_list_root;
/*************************************************************************/
static conf cf;
static boolean conf_init_var = 0;
static boolean function_list_root_init_var = 0;
static function_list_root functionListRoot;
/*************************************************************************/
void conf_init(){
	if (conf_init_var == 0){
		cf.MY_ADDRESS = NULL;
		cf.NET_ID = DFLT_NET_ID;
		cf.SEND_BEACON_PACKET_INTERVALL = DFLT_SEND_BEACON_PACKET_INTERVALL;
		cf.SEND_REPORT_PACKET_INTERVALL = DFLT_SEND_REPORT_PACKET_INTERVALL;
		cf.UPDTABLE_MAX = DFLT_UPDTABLE_MAX;
		cf.TTL_MAX = DFLT_TTL_MAX;
		cf.RSSI_MIN = DFLT_RSSI_MIN;

		conf_init_var = 1;
	}
}
/*************************************************************************/
function_root * new_function_root(uint8_t id_h, uint8_t id_l, uint8_t total_elements){
	function_root *fr;
	fr = malloc(sizeof(function_root));
	fr->id_h = id_h;
	fr->id_l = id_l;
	fr->total_elements = total_elements;
	fr->elements_number = 0;
	fr->total_bytes = 0;
	fr->root = NULL;
	return fr;
}
/*************************************************************************/
function_element ** function_root_search_by_position(function_root *fr, uint8_t position){
	if (fr->root == NULL || fr->root->position >= position){
		return (&(fr->root));
	}
	uint8_t i;
	function_element * fep;
	function_element * fep_before;
	fep_before = fr->root;
	fep = fr->root->next;
	for (i=1; i<fr->elements_number; i++){
		if (fep->position >= position){
			return (&(fep_before->next));
		}
		fep_before = fep;
		fep = fep->next;
	}
	return (&(fep_before->next));
}
/*************************************************************************/
void function_root_add_element(function_root *fr, uint8_t position, uint8_t *array, uint8_t len_array){
	function_element **fe_before;
	fe_before = function_root_search_by_position(fr, position);
	if (*fe_before != NULL && (*fe_before)->next != NULL && (*fe_before)->next->position == position) return;

	function_element *fe;
	fe = malloc(sizeof(function_element));
	fe->position = position;
	fe->len_array = len_array;
	//////
	int fd;
	char file[20];
	sprintf(file, "f_%d_%d_%d", fr->id_h, fr->id_l, position);
	fd = cfs_open(file, CFS_WRITE);
	cfs_write(fd, array, len_array);
	cfs_close(fd);
	//////
	/*
	uint8_t i;
	for (i=0; i<len_array; i++){
		(fe->array)[i] = array[i];
	}
	*/
	fe->next = (*fe_before);
	(*fe_before) = fe;
	fr->elements_number = fr->elements_number + 1;
	fr->total_bytes = fr->total_bytes + (int) fe->len_array;
}
/*************************************************************************/
void function_root_remove_all (function_root *fr){
	function_element *fep;
	fep = fr->root;
	char file[20];
	uint8_t i;
	for (i=0; i<fr->elements_number; i++){
		sprintf(file, "f_%d_%d_%d", fr->id_h, fr->id_l, fep->position);
		cfs_remove(file);
		fr->root = fep->next;
		free(fep);
		fep = fr->root;
	}
	free(fr);
}
/*************************************************************************/
void function_root_compose_packet(function_root *fr){
	int fd_file;
	char file[16];
	sprintf(file, "f_%d_%d.ce", fr->id_h, fr->id_l);
	fd_file = cfs_open(file, CFS_WRITE);
	cfs_seek(fd_file, 0, CFS_SEEK_SET);

	int fd_file_tmp;
	char file_tmp[20];
	char tmp[PACKET_LEN];

	function_element *fep;
	fep = fr->root;
	uint8_t i, j;
	int r, v;
	uint8_t zero[1];
	zero[0] = 0;
	for (i=0; i<fr->elements_number; i++){
		sprintf(file_tmp, "f_%d_%d_%d", fr->id_h, fr->id_l, fep->position);
		fd_file_tmp = cfs_open(file_tmp, CFS_READ);
		cfs_seek(fd_file_tmp, 0, CFS_SEEK_SET);
		r = cfs_read(fd_file_tmp, tmp, fep->len_array);
		cfs_write(fd_file, tmp, r);

		v = fep->len_array - r;
		for (j=0; j<v; j++){
			cfs_write(fd_file, zero, 1);
		}

		cfs_close(fd_file_tmp);
		fep = fep->next;
	}
	cfs_close(fd_file);
}
/*************************************************************************/
void function_list_root_init(){
	if (function_list_root_init_var == 0){
		functionListRoot.len = 0;
		functionListRoot.root_list = NULL;
		function_list_root_init_var = 1;
	}
}
/*************************************************************************/
function_root * function_list_root_search_by_id(uint8_t id_h, uint8_t id_l){
	if (functionListRoot.len == 0) return NULL;
	uint8_t i;
	function_list_element *flep;
	flep = functionListRoot.root_list;
	for (i=0; i<functionListRoot.len; i++){
		if (flep->functionRoot->id_h == id_h && flep->functionRoot->id_l == id_l) {
			return flep->functionRoot;
		}
		flep = flep->next;
	}
	return NULL;
}
/*************************************************************************/
function_root * function_list_root_new_element(uint8_t id_h, uint8_t id_l, uint8_t total_elements){
	function_list_element *flep;
	flep = malloc(sizeof(function_list_element));
	flep->next = NULL;
	flep->functionRoot = new_function_root(id_h, id_l, total_elements);

	function_list_element *last;
	if (functionListRoot.len == 0) {
		functionListRoot.root_list = flep;
	} else {
		for (last = functionListRoot.root_list; last->next != NULL; last = last->next){
		}
		last->next = flep;
	}
	functionListRoot.len = functionListRoot.len + 1;
	return flep->functionRoot;
}
/*************************************************************************/
void function_list_root_remove_element_by_id(uint8_t id_h, uint8_t id_l){
	if (functionListRoot.len == 0) return;
	function_list_element *flep;
	flep = functionListRoot.root_list;
	if (flep->functionRoot->id_h == id_h && flep->functionRoot->id_l == id_l){
		functionListRoot.root_list->next = flep->next;
		function_root_remove_all(flep->functionRoot);
		free(flep);
		return;
	}
	uint8_t i;
	function_list_element *before;
	before = functionListRoot.root_list;
	flep = flep->next;
	for (i=1; i<functionListRoot.len; i++){
		if (flep->functionRoot->id_h == id_h && flep->functionRoot->id_l == id_l){
			before->next = flep->next;
			function_root_remove_all(flep->functionRoot);
			free(flep);
			return;
		}
		before = flep;
		flep = flep->next;
	}
}
/*************************************************************************/
#endif // _SDN_WISE_CONFIGURE_H_
