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
 *         Header file for the SDN-WISE Packet buffer.
 * \author
 *         Sebastiano Milardo <s.milardo@hotmail.it>
 */

/**
 * \addtogroup sdn-wise
 * @{
 */

#ifndef PACKET_BUFFER_H_
#define PACKET_BUFFER_H_

#include "address.h"

#define ADDRESS_LENGTH        2

#define NET_INDEX (0)
#define LEN_INDEX (NET_INDEX + 1)
#define DST_INDEX (LEN_INDEX + 1)
#define SRC_INDEX (DST_INDEX + ADDRESS_LENGTH)
#define TYP_INDEX (SRC_INDEX + ADDRESS_LENGTH)
#define TTL_INDEX (TYP_INDEX + 1)
#define NXH_INDEX (TTL_INDEX + 1)
#define PLD_INDEX (NXH_INDEX + ADDRESS_LENGTH)

#define MAX_PAYLOAD_LENGTH  106
#define MAX_PACKET_LENGTH       (MAX_PAYLOAD_LENGTH + PLD_INDEX)

  typedef enum __attribute__((__packed__)) packet_type{
    DATA,
    BEACON,
    REPORT,
    REQUEST,
    RESPONSE,
    OPEN_PATH,
    CONFIG,
    REG_PROXY
  } packet_type_t;

  typedef struct __attribute__((__packed__)) packet_info {
    uint8_t rssi;
  } packet_info_t;

  typedef struct __attribute__((__packed__)) packet_header {
    uint8_t net;
    uint8_t len;
    address_t dst;
    address_t src;
    packet_type_t typ;
    uint8_t ttl;
    address_t nxh;
  } header_t;

  typedef struct __attribute__((__packed__)) packet_struct {
    header_t header;
    uint8_t payload[MAX_PAYLOAD_LENGTH];
    packet_info_t info;
    struct packet_struct *next;
  } packet_t;

/* Header API. */
  packet_t* get_packet_from_array(uint8_t*);
  uint8_t get_payload_at(packet_t*, uint8_t);
  void set_payload_at(packet_t*, uint8_t, uint8_t);
  void restore_ttl(packet_t*);

  packet_t* create_packet(uint8_t, address_t*, address_t*, packet_type_t, 
    address_t*);
  packet_t* create_packet_empty(void);
  packet_t* create_packet_payload(uint8_t, address_t*, address_t*, packet_type_t, 
    address_t*, uint8_t*, uint8_t);
  void packet_deallocate(packet_t*);

  void packet_buffer_init(void);
  void print_packet(packet_t*);
  void print_packet_uart(packet_t*);

  void test_packet_buffer(void);

#endif /* PACKET_BUFFER_H_ */
/** @} */
