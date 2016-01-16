//
//  header.h
//  Prova
//
//  Created by Mario Brischetto on 25/10/15.
//  Copyright (c) 2015 Mario Brischetto. All rights reserved.
//

#ifndef __Prova__header__
#define __Prova__header__

#include <stdio.h>
#include "util.h"

#define PACKET_LEN          128
#define HEADER_LENGTH       10
#define ADDRESS_LENGHT      2

#define HEADER_LEN_POS          1
#define HEADER_NETID_POS        0
#define HEADER_SRC_POS          2
#define HEADER_DST_POS          4
#define HEADER_TYPE_POS         6
#define HEADER_TTL_POS          7
#define HEADER_NEXT_HOP_POS     8
/*************************************************************************/
struct structAddress {
    uint8_t addr_h;
    uint8_t addr_l;
};
typedef struct structAddress address;
/*************************************************************************/
void address2array(address address, uint8_t *array){
    array[0] = address.addr_h;
    array[1] = address.addr_l;
}
void array2address(uint8_t array[], address *address){
    address->addr_h = array[0];
    address->addr_l = array[1];
}
int addressCmp(address address1, address address2){		//0=(add1=add2), +1=(add1>add2), -1=(add1<add2)
    uint8_t add1[ADDRESS_LENGHT];
    address2array(address1, add1);
    uint8_t add2[ADDRESS_LENGHT];
    address2array(address2, add2);
    return arrayCmp(add1, add2, ADDRESS_LENGHT);
}
/*************************************************************************/
address BROADCAST = {255, 255};
/*************************************************************************/
struct structHeader {
    uint8_t len;
    uint8_t net_id;
    address src;
    address dst;
    uint8_t type;
    uint8_t ttl;
    address next_hop;
};
typedef struct structHeader header;
/*************************************************************************/
void header2array(header h, uint8_t *header_array){
    header_array[HEADER_LEN_POS] = h.len;
    header_array[HEADER_NETID_POS] = h.net_id;
    header_array[HEADER_SRC_POS] = h.src.addr_h;
    header_array[HEADER_SRC_POS +1] = h.src.addr_l;
    header_array[HEADER_DST_POS] = h.dst.addr_h;
    header_array[HEADER_DST_POS +1] = h.dst.addr_l;
    header_array[HEADER_TYPE_POS] = h.type;
    header_array[HEADER_TTL_POS] = h.ttl;
    header_array[HEADER_NEXT_HOP_POS] = h.next_hop.addr_h;
    header_array[HEADER_NEXT_HOP_POS +1] = h.next_hop.addr_l;
    return;
}
void array2header(uint8_t header_array[], header *h){
    h->len = header_array[HEADER_LEN_POS];
    h->net_id = header_array[HEADER_NETID_POS];
    h->src.addr_h = header_array[HEADER_SRC_POS];
    h->src.addr_l = header_array[HEADER_SRC_POS +1];
    h->dst.addr_h = header_array[HEADER_DST_POS];
    h->dst.addr_l = header_array[HEADER_DST_POS +1];
    h->type = header_array[HEADER_TYPE_POS];
    h->ttl = header_array[HEADER_TTL_POS];
    h->next_hop.addr_h = header_array[HEADER_NEXT_HOP_POS];
    h->next_hop.addr_l = header_array[HEADER_NEXT_HOP_POS +1];
    return;
}
void printHeader(header h){
    uint8_t header_array[HEADER_LENGTH];
    header2array(h, header_array);
    //printf("Print Header\t ");
    printArray(header_array, (uint8_t)HEADER_LENGTH);
}
/*************************************************************************/
enum packet_types{
    DATA        = 0,
    BEACON      = 1,
    REPORT      = 2,
    REQUEST     = 128,
    RESPONSE    = 4,
    OPEN_PATH   = 5,
    CONFIG      = 6
};




#endif /* defined(__Prova__header__) */
