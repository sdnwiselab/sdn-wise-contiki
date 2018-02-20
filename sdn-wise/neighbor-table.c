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
 *         SDN-WISE Neighbor's table.
 * \author
 *         Sebastiano Milardo <s.milardo@hotmail.it>
 */

/**
 * \addtogroup sdn-wise
 * @{
 */

#include <string.h>

#include "lib/memb.h"
#include "lib/list.h"

#include "packet-buffer.h"
#include "neighbor-table.h"
#include "packet-creator.h"

#ifndef SDN_WISE_DEBUG
#define SDN_WISE_DEBUG 0
#endif
#if SDN_WISE_DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*----------------------------------------------------------------------------*/
  LIST(neighbor_table);
  MEMB(neighbors_memb, neighbor_t, (MAX_PAYLOAD_LENGTH) / (NEIGHBOR_LENGTH));
/*----------------------------------------------------------------------------*/
  static neighbor_t * neighbor_allocate(void);
  static void neighbor_free(neighbor_t *);
  static void print_neighbor(neighbor_t*);
/*----------------------------------------------------------------------------*/
  static void 
  print_neighbor(neighbor_t* n)
  {
    print_address(&(n->address));
    PRINTF("%d",n->rssi);
  }
/*----------------------------------------------------------------------------*/
  void 
  print_neighbor_table(void)
  {
    neighbor_t *n;
    for(n = list_head(neighbor_table); n != NULL; n = n->next) {
      PRINTF("[NGT]: ");
      print_neighbor(n);
      PRINTF("\n");  
    }
  }
/*----------------------------------------------------------------------------*/
  void 
  purge_neighbor_table(void)
  {
    neighbor_t *n;
    neighbor_t *next;

    for(n = list_head(neighbor_table); n != NULL;) {
      next = n->next;
      neighbor_free(n);
      n = next;
    }
  }
/*----------------------------------------------------------------------------*/
  static neighbor_t *
  neighbor_allocate(void)
  {
    neighbor_t *p;
    p = memb_alloc(&neighbors_memb);
    if(p == NULL) {
      PRINTF("[NGT]: Failed to allocate a neighbor\n");
    }
    return p;
  }
/*----------------------------------------------------------------------------*/
  static void
  neighbor_free(neighbor_t* n)
  {
    list_remove(neighbor_table, n);
    int res = memb_free(&neighbors_memb, n); 
    if (res !=0){
      PRINTF("[NGT]: Failed to free a neighbor. Reference count: %d\n",res);
    }
  }
/*----------------------------------------------------------------------------*/
  uint8_t
  neighbor_cmp(neighbor_t* a, neighbor_t* b)
  {
    return address_cmp(&(a->address),&(b->address));
  }
/*----------------------------------------------------------------------------*/
  neighbor_t*
  neighbor_table_contains(address_t* a)
  {
    neighbor_t* tmp;
    for(tmp = list_head(neighbor_table); tmp != NULL; tmp = tmp->next) {
      if(address_cmp(&(tmp->address),a)){
        return tmp;
      }  
    }
    return NULL;
  }
/*----------------------------------------------------------------------------*/
  void
  add_neighbor(address_t* address, uint8_t rssi)
  {
    neighbor_t* res = neighbor_table_contains(address);
    if (res == NULL){ 
      neighbor_t* n = neighbor_allocate();
      if (n != NULL){
        memset(n, 0, sizeof(*n));
        n->address = *address;
        n->rssi = rssi;
        list_add(neighbor_table,n);
      } 
    } else {
      res->rssi = rssi;
    }  
  }
/*----------------------------------------------------------------------------*/
  void
  fill_payload_with_neighbors(packet_t* p)
  {
    uint8_t i = REPORT_INIT_INDEX;
    set_payload_at(p,i,(uint8_t)(list_length(neighbor_table) & 0xFF));
    i++;
    neighbor_t *n;
    for(n = list_head(neighbor_table); n != NULL; n = n->next) { 
      uint8_t j = 0;
      for (j = 0; j < ADDRESS_LENGTH; ++j){ 
        set_payload_at(p,i,n->address.u8[j]);
        ++i;
      }
      set_payload_at(p,i,n->rssi);
      ++i;
    }
    purge_neighbor_table();
  }
/*----------------------------------------------------------------------------*/
  void 
  neighbor_table_init(void)
  {
    list_init(neighbor_table);
    memb_init(&neighbors_memb);
  }
/*----------------------------------------------------------------------------*/
  void 
  test_neighbor_table(void)
  {
    address_t addr1;
    addr1.u8[0] = 1;
    addr1.u8[1] = 1;
    uint8_t rssi1 = 100;

    address_t addr3;
    addr3.u8[0] = 1;
    addr3.u8[1] = 1;
    uint8_t rssi3 = 50;

    address_t addr2;
    addr2.u8[0] = 2;
    addr2.u8[1] = 2;
    uint8_t rssi2 = 200;

    if (!list_length(neighbor_table)){
      add_neighbor(&addr1,rssi1);
      add_neighbor(&addr2,rssi2);
      add_neighbor(&addr3,rssi3);
      print_neighbor_table();
    }else{
      purge_neighbor_table();
      print_neighbor_table();
    }
  }
/*----------------------------------------------------------------------------*/
/** @} */
