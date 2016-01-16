//
//  neighbor_table.h
//  Prova
//
//  Created by Mario Brischetto on 25/10/15.
//  Copyright (c) 2015 Mario Brischetto. All rights reserved.
//

#ifndef __Prova__neighbor_table__
#define __Prova__neighbor_table__

#include <stdio.h>
#include "header.h"
#include <stdlib.h>

/*************************************************************************/
struct structNeighbor{
    address address;
    uint8_t rssi;
    uint8_t batt;
    uint8_t nhop;
};
typedef struct structNeighbor neighbor;

#define NEIGHBOR_LEN        5
#define NEIGHBOR_ADDR_POS   0
#define NEIGHBOR_RSSI_POS   2
#define NEIGHBOR_BATT_POS   3
#define NEIGHBOR_NHOP_POS   4
/*************************************************************************/
void neighbor2array(neighbor neighbor, uint8_t array[], boolean include_batt_nhop){
    array[NEIGHBOR_ADDR_POS] = neighbor.address.addr_h;
    array[NEIGHBOR_ADDR_POS +1] = neighbor.address.addr_l;
    array[NEIGHBOR_RSSI_POS] = neighbor.rssi;
    if (include_batt_nhop == 1) {
        array[NEIGHBOR_BATT_POS] = neighbor.batt;
        array[NEIGHBOR_NHOP_POS] = neighbor.nhop;
    }
}
/*************************************************************************/
void array2neighbor(uint8_t array[], neighbor *neighbor, boolean include_batt_nhop){
    neighbor->address.addr_h = array[NEIGHBOR_ADDR_POS];
    neighbor->address.addr_l = array[NEIGHBOR_ADDR_POS +1];
    neighbor->rssi = array[NEIGHBOR_RSSI_POS];
    if (include_batt_nhop == 1) {
        neighbor->batt = array[NEIGHBOR_BATT_POS];
        neighbor->nhop = array[NEIGHBOR_NHOP_POS];
    }
}
/*************************************************************************/
void printNeighbor(neighbor neighbor, boolean include_batt_nhop){
    int dim;
    if (include_batt_nhop == 1) {
        dim = NEIGHBOR_LEN;
    } else {
        dim = NEIGHBOR_LEN -2;
    }
    uint8_t neighbor_array[dim];
    neighbor2array(neighbor, neighbor_array, include_batt_nhop);
    ////printf("Print Neighbor\t");
    printArray(neighbor_array, dim);
}
/*************************************************************************/

/*************************************************************************/
typedef struct short_neighbor_element{
    uint8_t array[NEIGHBOR_LEN -2];
    struct short_neighbor_element * next;
} short_neighbor_element;

typedef struct short_neighbor_table{
	short_neighbor_element * root_table;
    uint8_t table_len;
} short_neighbor_table;
/*************************************************************************/
void neighbor_table_add(short_neighbor_table *t, uint8_t *array_p){
    //creo nuovo elemento
	short_neighbor_element *e;
    e = malloc(sizeof(short_neighbor_element));
    int i;
    for (i=0; i < NEIGHBOR_LEN -2; i++) {
        (*e).array[i] = array_p[i];
    }
    e->next = NULL;

    //aggiungo elemento in coda alla lista
    if (t->root_table == NULL) {
        t->root_table = e;
    } else {
    	short_neighbor_element *last;
        for (last = (*t).root_table; (*last).next != NULL; last = (*last).next) {
        }
        (*last).next = e;
    }
    t->table_len++;
}
/*************************************************************************/
void neighbor_table_remove (short_neighbor_table *t, short_neighbor_element short_neighbor){
	uint8_t neighbor_array[NEIGHBOR_LEN -2];
	int j;
	for (j=0; j<NEIGHBOR_LEN -2; j++){
		neighbor_array[j] = short_neighbor.array[j];
	}

	short_neighbor_element *e;
	e = (*t).root_table;
	if (arrayCmp((*e).array, neighbor_array, NEIGHBOR_LEN -2) == 0 ){
		(*t).root_table = NULL;
		free(e);
		//printf("short_neighbor_element - eliminato (1)\n");
		return;
	} else {
		short_neighbor_element *e_before;
		e_before = e;
		e = e->next;
		int i;
		for (i=1; i<(*t).table_len; i++ ){
			if (arrayCmp((*e).array, neighbor_array, NEIGHBOR_LEN -2) == 0 ){
				e_before->next = e->next;
				free(e);
				(*t).table_len --;
				//printf("short_neighbor_element - eliminato (2)\n");
				return;
			}
			e_before = e;
			e = e->next;
		}
	}
	//printf("short_neighbor_element - problema\n");
}
/*************************************************************************/
void neighbor_table_remove_all (short_neighbor_table *t){
	int i;
	for (i=0; i<t->table_len; i++){
		neighbor_table_remove(t, *((*t).root_table) );
	}
	t->table_len = 0;
}
/*************************************************************************/
void printTable(short_neighbor_table t){
	//printf("printTable: \n");
	//printf("---------------\n");
	short_neighbor_element *e;
    e = t.root_table;
    int i;
    for (i=0; i<t.table_len; i++) {
        printArray(e->array, NEIGHBOR_LEN -2);
        e = e->next;
    }
    //printf("---------------\n");
}
/*************************************************************************/
short_neighbor_element* find_element_by_address(short_neighbor_table neighbor_table, address addr){
	short_neighbor_element *e;
	e = neighbor_table.root_table;
	int i;
	for (i=0; i<neighbor_table.table_len; i++) {
		address asne = {e->array[0], e->array[1]};
		if (addressCmp(addr, asne) == 0){
			return e;
		}
	    e = e->next;
	}
	return NULL;
}
/*************************************************************************/

#endif /* defined(__Prova__neighbor_table__) */

