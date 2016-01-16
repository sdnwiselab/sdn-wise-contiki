//
//  util.h
//  Prova
//
//  Created by Mario Brischetto on 21/10/15.
//  Copyright (c) 2015 Mario Brischetto. All rights reserved.
//

#ifndef Prova_util_h
#define Prova_util_h
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
        //printf("%d\t ", array[i]);
    }
    //printf("\n");
}
/*************************************************************************/
void split(uint8_t *old_array, uint8_t *new_array, uint8_t first_byte, uint8_t len){
    int i,j;
    for (j=first_byte, i=0; j<first_byte + len; j++, i++){
        new_array[i] = old_array[j];
    }
}
/*************************************************************************/
int arrayCmp(uint8_t *array1, uint8_t *array2, uint8_t dim){                //0=(array1=array2), +1=(array1>array2), -1=(array1<array2)
    int r, i;
    for (i=0; i<dim; i++) {
        if (array1[i] > array2[i]) {
            r = 1;
            break;
        } else if (array1[i] < array2[i]) {
            r = -1;
            break;
        } else if (array1[i] == array2[i]) {
            if (i == dim -1) {
                r = 0;
                break;
            }
        }
    }
    return r;
}
/*************************************************************************/


#endif
