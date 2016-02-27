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
 *         Header file for the SDN-WISE Neighbor's table.
 * \author
 *         Sebastiano Milardo <s.milardo@hotmail.it>
 */

/**
 * \addtogroup sdn-wise
 * @{
 */

#ifndef NEIGHBOR_TABLE_H_
#define NEIGHBOR_TABLE_H_

#include "address.h"
#include "packet-buffer.h"

#define NEIGHBOR_LENGTH       (ADDRESS_LENGTH + 1)

  typedef struct neighbor_struct {
    struct neighbor_struct *next;
    address_t address;
    uint8_t rssi;
  } neighbor_t;

  /* Header API. */
  void add_neighbor(address_t*, uint8_t rssi);
  void purge_neighbor_table(void);
  void fill_payload_with_neighbors(packet_t*);
  void neighbor_table_init(void);
  void print_neighbor_table(void);
  void test_neighbor_table(void);
  neighbor_t* neighbor_table_contains(address_t*);
  uint8_t neighbor_cmp(neighbor_t*, neighbor_t*);
#endif /* NEIGHBOR_TABLE_H_ */
/** @} */