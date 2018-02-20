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
 *         SDN-WISE Packet buffer.
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

#include "packet-buffer.h"
#include "address.h"

#define MAX_TTL   100

#ifndef SDN_WISE_DEBUG
#define SDN_WISE_DEBUG 0
#endif
#if SDN_WISE_DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*----------------------------------------------------------------------------*/
  MEMB(packets_memb, packet_t, 4);
/*----------------------------------------------------------------------------*/
  static packet_t * packet_allocate(void);
/*----------------------------------------------------------------------------*/
  void 
  print_packet_uart(packet_t* p)
  {
    uint16_t i = 0;
    putchar(122);
    uint8_t* tmp = (uint8_t*)p;
    for (i = 0; i< p->header.len; ++i){
      putchar(tmp[i]);
    }
    putchar(126);
    putchar('\n');
    packet_deallocate(p);
  }
/*----------------------------------------------------------------------------*/
  void 
  print_packet(packet_t* p)
  {
    uint16_t i = 0;
    PRINTF("%d %d ", p->header.net, p->header.len);
    print_address(&(p->header.dst));
    print_address(&(p->header.src));
    PRINTF("%d %d ", p->header.typ, p->header.ttl);
    print_address(&(p->header.nxh));
    for (i=0; i < (p->header.len - PLD_INDEX); ++i){
      PRINTF("%d ",get_payload_at(p,i));
    }
  }
/*----------------------------------------------------------------------------*/
  static packet_t *
  packet_allocate(void)
  {
    packet_t *p = NULL;
    p = memb_alloc(&packets_memb);
    if(p == NULL) {
      PRINTF("[PBF]: Failed to allocate a packet\n");
    }
    return p;
  }
/*----------------------------------------------------------------------------*/
  void
  packet_deallocate(packet_t* p)
  {
    int res = memb_free(&packets_memb, p); 
    if (res !=0){
      PRINTF("[FLT]: Failed to deallocate a packet. Reference count: %d\n",res);
    }
  }
/*----------------------------------------------------------------------------*/
  packet_t* 
  create_packet_payload(uint8_t net, address_t* dst, address_t* src, 
    packet_type_t typ, address_t* nxh, uint8_t* payload, uint8_t len)
  {
    packet_t* p = create_packet(net, dst, src, typ, nxh);
    if (p != NULL){
      uint8_t i;

      for (i = 0; i < len; ++i){
        set_payload_at(p, i, payload[i]);  
      }
    }
    return p;
  }
/*----------------------------------------------------------------------------*/
  packet_t* 
  get_packet_from_array(uint8_t* array)
  {
    // TODO fragmentation
    // This works if the compiler complies with __attribute__((__packed__))
    packet_t* p = packet_allocate();
    if (p != NULL){
      memcpy((uint8_t*)p, array, array[LEN_INDEX]);
    }
    return p;
  }
/*----------------------------------------------------------------------------*/
  uint8_t 
  get_payload_at(packet_t* p, uint8_t index)
  {
    if (index < MAX_PACKET_LENGTH){
      return p->payload[index];
    } else {
      return 0;
    }
  }
/*----------------------------------------------------------------------------*/
  void 
  set_payload_at(packet_t* p, uint8_t index, uint8_t value)
  {
    if (index < MAX_PACKET_LENGTH){
      p->payload[index] = value;
      if (index + PLD_INDEX + 1 > p->header.len){
        p->header.len = index + PLD_INDEX + 1;
      }
    }
  }
/*----------------------------------------------------------------------------*/
  void 
  restore_ttl(packet_t* p)
  {
    p->header.ttl = MAX_TTL;
  }
/*----------------------------------------------------------------------------*/
  packet_t* 
  create_packet_empty(void)
  {
    packet_t* p = packet_allocate();
    if (p != NULL){
      memset(&(p->header), 0, sizeof(p->header));
      memset(&(p->info), 0, sizeof(p->info));
      restore_ttl(p);
    } 
    return p;
  }
/*----------------------------------------------------------------------------*/
  packet_t* 
  create_packet(uint8_t net, address_t* dst, address_t* src, packet_type_t typ, 
    address_t* nxh)
  {
    packet_t* p = packet_allocate();
    if (p != NULL){
      memset(&(p->header), 0, sizeof(p->header));
      memset(&(p->info), 0, sizeof(p->info));
      p->header.net=net;
      p->header.dst=*dst;
      p->header.src=*src;
      p->header.typ=typ;
      p->header.nxh=*nxh;
      restore_ttl(p);
    } 
    return p;
  }
/*----------------------------------------------------------------------------*/
  void 
  packet_buffer_init(void)
  {
    memb_init(&packets_memb);
  }

/*----------------------------------------------------------------------------*/
  void 
  test_packet_buffer(void)
  {
    uint8_t array[73] = {1, 73, 0, 0, 0, 2, 4, 100, 0, 0, 20, 18, 0, 6, 0, 10, 
      18, 0, 50, 0, 1, 90, 0, 10, 0, 1, 122, 0, 12, 0, 5, 1, 4, 8, 6, 2, 0, 10, 
      0, 40, 0, 0, 8, 5, 1, 0, 0, 0, 0, 0, 0, 1, 3, 3, 2, 255, 255, 3, 1, 0, 3, 
      1, 7, 8, 6, 132, 0, 11, 0, 12, 0, 13, 254};

      packet_t* second = get_packet_from_array(array);
      print_packet(second);
  }
/*----------------------------------------------------------------------------*/
/** @} */
