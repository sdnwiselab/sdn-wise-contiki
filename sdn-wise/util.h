/*
 *  util.h
 *
 *  Created on: 09 nov 2015
 *      Author: Mario Brischetto
 */

#ifndef _SDN_WISE_UTIL_H_
#define _SDN_WISE_UTIL_H_
/*************************************************************************/
#include <stdint.h>
/*************************************************************************/
typedef enum boolean {
    TRUE = 1,
    FALSE = 0
} boolean;
/*************************************************************************/
void printArray(uint8_t array[], uint8_t dim){
	int i;
    for (i=0; i<dim; i++) {
        printf("%d\t ", array[i]);
    }
    printf("\n");
}
/*************************************************************************/
void split(uint8_t *old_array, uint8_t *new_array, uint8_t first_byte, uint8_t len){
    int i,j;
    for (j=first_byte, i=0; j<first_byte + len; j++, i++){
        new_array[i] = old_array[j];
    }
}
/*************************************************************************/
int arrayCmp(uint8_t *array1, uint8_t *array2, uint8_t dim){                
//0=(array1=array2), +1=(array1>array2), -1=(array1<array2)
    int r = 0, i;
    for (i=0; i<dim; i++) {
        if (array1[i] > array2[i]) {
            r = 1;
            break;
        } else if (array1[i] < array2[i]) {
            r = -1;
            break;
        } 
    }
    return r;
}
/*************************************************************************/
#endif
