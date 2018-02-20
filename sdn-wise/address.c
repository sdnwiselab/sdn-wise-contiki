/*
* Copyright (C) 2015 SDN-WISE
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
* \file
*         SDN-WISE Address.
* \author
*         Sebastiano Milardo <s.milardo@hotmail.it>
*/

/**
* \addtogroup sdn-wise
* @{
*/

#include <string.h>
#include <stdio.h>

#include "lib/memb.h"
#include "lib/list.h"

#include "address.h"
#include "node-conf.h"

#ifndef SDN_WISE_DEBUG
#define SDN_WISE_DEBUG 0
#endif
#if SDN_WISE_DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*----------------------------------------------------------------------------*/
  LIST(address_list);
  MEMB(addresses_memb, accepted_address_t, 4);
/*----------------------------------------------------------------------------*/
  static void accepted_address_free(accepted_address_t*);
  static accepted_address_t * accepted_address_allocate(void);
/*----------------------------------------------------------------------------*/
  void swap_addresses(address_t* a, address_t* b)
  {
    address_t tmp = *a;
    *a = *b;
    *b = tmp;
  }
/*----------------------------------------------------------------------------*/  
  uint8_t 
  is_my_address(address_t* a)
  {
    return (address_cmp(&(conf.my_address), a) || address_list_contains(a));
  }
/*----------------------------------------------------------------------------*/
  void 
  print_address(address_t* a)
  {
    uint16_t i = 0;
    for (i=0;i<ADDRESS_LENGTH-1;++i){
      PRINTF("%d.",a->u8[i]);
    }
    PRINTF("%d ",a->u8[i]);
  }
/*----------------------------------------------------------------------------*/
  void 
  print_address_list(void)
  {
    accepted_address_t *a;
    for(a = list_head(address_list); a != NULL; a = a->next) {
      PRINTF("[ADR]: ");
      print_address(&(a->address));
      PRINTF("\n");  
    }
  }
/*----------------------------------------------------------------------------*/
  static accepted_address_t *
  accepted_address_allocate(void)
  {
    accepted_address_t* p;
    p = memb_alloc(&addresses_memb);
    if(p == NULL) {
      PRINTF("[AAL]: Failed to allocate an address\n");
    }
    return p;
  }
/*----------------------------------------------------------------------------*/
  static void
  accepted_address_free(accepted_address_t* n)
  {
    list_remove(address_list, n);
    int res = memb_free(&addresses_memb, n); 
    if (res !=0){
      PRINTF("[AAL]: Failed to free an address. Reference count: %d\n",res);
    }
  }
/*----------------------------------------------------------------------------*/
  accepted_address_t*
  address_list_contains(address_t* a)
  {
    accepted_address_t* tmp;
    for(tmp = list_head(address_list); tmp != NULL; tmp = tmp->next) {
      if(address_cmp(&(tmp->address),a)){
        return tmp;
      }  
    }
    return NULL;
  }

/*----------------------------------------------------------------------------*/
  void
  add_accepted_address(address_t* address)
  {
    accepted_address_t* res = address_list_contains(address);
    if (res == NULL){ 
      accepted_address_t* a = accepted_address_allocate();
      if (a != NULL){
        memset(a, 0, sizeof(*a));
        a->address = *address;
        list_add(address_list,a);
      } 
    } 
  }
/*----------------------------------------------------------------------------*/
  void
  purge_address_list(void)
  {
    accepted_address_t* n;
    accepted_address_t* next;

    for(n = list_head(address_list); n != NULL;) {
      next = n->next;
      accepted_address_free(n);
      n = next;
    }
  }
/*----------------------------------------------------------------------------*/
  void
  fill_array_with_address(uint8_t* array, address_t* address)
  {
    uint16_t i;
    for (i = 0; i < ADDRESS_LENGTH; ++i){
      array[i] = address->u8[i];
    }
  }
/*----------------------------------------------------------------------------*/
  address_t 
  get_address_from_int(uint16_t value)
  {
    address_t address;
    address.u8[ADDRESS_LENGTH-2] = value >> 8;
    address.u8[ADDRESS_LENGTH-1] = value & 0xFF;
    return address;
  }
/*----------------------------------------------------------------------------*/
  address_t 
  get_address_from_array(uint8_t* array)
  {
    uint16_t i;
    address_t address;
    for (i = 0; i < ADDRESS_LENGTH; ++i){
      address.u8[i] = array[i];
    }
    return address;
  }
/*----------------------------------------------------------------------------*/
  void
  set_broadcast_address(address_t* address)
  {
    uint16_t i;
    for (i = 0; i < ADDRESS_LENGTH; ++i){
      address->u8[i] = 255;
    }
  }
/*----------------------------------------------------------------------------*/
  uint8_t 
  is_broadcast(address_t* a)
  {
    uint16_t i;
    for (i = 0; i < ADDRESS_LENGTH; ++i){
      if (a->u8[i] != 255){
        return 0;
      }
    }
    return 1;
  }
/*----------------------------------------------------------------------------*/
  uint8_t 
  address_cmp(address_t* a, address_t* b)
  {
    uint16_t i;
    for (i = 0; i < ADDRESS_LENGTH; ++i){
      if (a->u8[i] != b->u8[i]){
        return 0;
      }
    }
    return 1;
  }
/*----------------------------------------------------------------------------*/
  void 
  address_list_init(void)
  {
    list_init(address_list);
    memb_init(&addresses_memb);
  }
/*----------------------------------------------------------------------------*/
  void
  test_address_list(void)
  {
    address_t addr1;
    addr1.u8[0] = 10;
    addr1.u8[1] = 10;

    address_t addr3;
    addr3.u8[0] = 255;
    addr3.u8[1] = 255;

    address_t addr2;
    addr2.u8[0] = 10;
    addr2.u8[1] = 10;

    if (!list_length(address_list)){
      add_accepted_address(&addr1);
      add_accepted_address(&addr2);
      add_accepted_address(&addr3);
      print_address_list();
    }else{
      purge_address_list();
      print_address_list();
    }
  }
/*----------------------------------------------------------------------------*/
/** @} */
