/*
 *  inout.h
 *
 *  Created on: 21 nov 2015
 *      Author: Mario Brischetto
 */

#ifndef _SDN_WISE_INOUT_H_
#define _SDN_WISE_INOUT_H_
/*************************************************************************/
#include <stdio.h>
#include "util.h"
#include "dev/uart1.h"
/*************************************************************************/
#define INPUT_STREAM_LEN_MAX	4096
/*************************************************************************/
typedef struct input_stream_element {
	struct input_stream_element * next;
	uint8_t x;
} input_stream_element;
typedef struct input_strema {
	input_stream_element * root;
	int len;
	input_stream_element * last;
} input_stream;
/*************************************************************************/
static input_stream inputStream;
static boolean input_stream_init_var = 0;
/*************************************************************************/
input_stream getInputStream(){
	return inputStream;
}
/*************************************************************************/
void input_stream_push (input_stream *is, uint8_t x){
	if (is->len == INPUT_STREAM_LEN_MAX -1) return;
	input_stream_element *e;
	e = malloc(sizeof(input_stream_element));
	e->x = x;
	e->next = NULL;

	//aggiungo elemento in coda alla lista
	if (is->root == NULL){
		is->root = e;
		is->last = e;
	} else {
		is->last->next = e;
		is->last = e;
	}
	is->len ++;
}
/*************************************************************************/
uint8_t input_stream_pop (input_stream *is){
	if (is->len == 0) return NULL;
	input_stream_element *e = is->root;
	uint8_t x = e->x;
	if (is->len == 1){
		is->root = NULL;
		is->last = NULL;
	} else {
		is->root = e->next;
	}
	free(e);
	is->len --;
	return x;
}
/*************************************************************************/
static void uart_rx_callback(unsigned char c) {
     uint8_t x = (uint8_t)c;
     input_stream_push(&inputStream, x);
}
/*************************************************************************/
void input_stream_init(){
	if (input_stream_init_var == 0){
		inputStream.len = 0;
		inputStream.root = NULL;
		inputStream.last = NULL;
		input_stream_init_var = 1;

		uart1_init(BAUD2UBR(115200)); //set the baud rate as necessary
		uart1_set_input(uart_rx_callback); //set the callback function
	}
}
/*************************************************************************/
#endif //_SDN_WISE_INOUT_H_
