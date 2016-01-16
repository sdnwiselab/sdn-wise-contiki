/*
 * functions.h
 *
 *  Created on: 07 dic 2015
 *      Author: Mario Brischetto
 */

#ifndef EXAMPLES_SDN_WISE_FUNCTIONS_H_
#define EXAMPLES_SDN_WISE_FUNCTIONS_H_

#include <stdio.h>
#include "util.h"
#include "contiki.h"


#define FORWARD_UP_PROCESS_EVENT	70

/*************************************************************************/
typedef struct functions_element{
	uint8_t id_h;
	uint8_t id_l;
	//void (*foo) (uint8_t *, int);
	struct process * process;
	struct functions_element *next;
} functions_element;

typedef struct functions_root{
	int len;
	functions_element * root;
} functions_root;
/*************************************************************************/
static functions_root functionsRoot;
/*************************************************************************/
static boolean functions_root_init_var = 0;
void functions_root_init(){
	if (functions_root_init_var == 0){
		functionsRoot.len = 0;
		functionsRoot.root = NULL;
		functions_root_init_var = 1;
	}
}
/*************************************************************************/
functions_element * functions_root_search_by_id(uint8_t id_h, uint8_t id_l){
	if (functionsRoot.len == 0) return NULL;
	int i;
	functions_element *fep;
	fep = functionsRoot.root;
	for (i=0; i<functionsRoot.len; i++){
		if (fep->id_h == id_h && fep->id_l == id_l){
			return fep;
		}
		fep = fep->next;
	}
	return NULL;
}
/*************************************************************************/
void functions_root_add(uint8_t id_h, uint8_t id_l, struct process * process){
	functions_element *fep;
	fep = functions_root_search_by_id(id_h, id_l);
	if (fep != NULL) return;
	fep = malloc(sizeof(functions_element));
	fep->id_h = id_h;
	fep->id_l = id_l;
	fep->process = process;
	fep->next = functionsRoot.root;
	functionsRoot.root = fep;
	functionsRoot.len ++;
	//printf("Functions Root Add\n");
}
/*************************************************************************/
void functions_root_remove(uint8_t id_h, uint8_t id_l){
	functions_element *fep;
	fep = functionsRoot.root;
	if (fep->id_h == id_h && fep->id_l == id_l){
		functionsRoot.root = fep->next;
		process_exit(fep->process);
		free(fep);
		functionsRoot.len --;
		//printf("Functions Root Remove\n");
		return;
	} else {
		functions_element *fep_before;
		fep_before = fep;
		fep = fep->next;
		int i;
		for (i=1; i<functionsRoot.len; i++){
			if (fep->id_h == id_h && fep->id_l == id_l){
				fep_before->next = fep->next;
				process_exit(fep->process);
				free(fep);
				functionsRoot.len --;
				//printf("Functions Root Remove\n");
				return;
			}
			fep_before = fep;
			fep = fep->next;
		}
	}

}
/*************************************************************************/

#endif /* EXAMPLES_SDN_WISE_FUNCTIONS_H_ */
