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
 *         Header file for the SDN-WISE Address.
 * \author
 *         Sebastiano Milardo <s.milardo@hotmail.it>
 */

/**
 * \addtogroup sdn-wise
 * @{
 */

#ifndef ADDRESS_H_
#define ADDRESS_H_

#include <stdint.h>

#define ADDRESS_LENGTH  2

  typedef struct __attribute__((__packed__)) address_struct {
    uint8_t u8[ADDRESS_LENGTH];
  } address_t;

  typedef struct accepted_address_struct {
    struct accepted_address_struct* next;
    address_t address;
  } accepted_address_t;

  /* Address API. */
  uint8_t is_broadcast(address_t*);
  uint8_t is_my_address(address_t*);
  void set_broadcast_address(address_t*);
  address_t get_address_from_array(uint8_t*);
  address_t get_address_from_int(uint16_t);
  void fill_array_with_address(uint8_t*, address_t*);
  uint8_t address_cmp(address_t*, address_t*);
  void print_address(address_t*);
  void swap_addresses(address_t*,address_t*);

  /* Accepted Address API. */  
  void address_list_init(void);
  void add_accepted_address(address_t*);
  accepted_address_t* address_list_contains(address_t*);
  void purge_address_list(void);
  void print_address_list(void);
  
  /* Test API. */
  void test_address_list(void);
  

#endif /* ADDRESS_H_ */
/** @} */