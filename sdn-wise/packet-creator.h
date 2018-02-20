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
 *         Header file for SDN-WISE Packet Creator.
 * \author
 *         Sebastiano Milardo <s.milardo@hotmail.it>
 */

/**
 * \addtogroup sdn-wise
 * @{
 */

#ifndef PACKET_CREATOR_H_
#define PACKET_CREATOR_H_

#define BEACON_HOPS_INDEX 0
#define BEACON_BATT_INDEX 1
#define REPORT_INIT_INDEX 2
#define OPEN_PATH_WINDOWS_INDEX 0

#include "packet-buffer.h"

/* packets API. */
  packet_t* create_beacon(void);
  packet_t* create_data(uint8_t);
  packet_t* create_report(void);
  packet_t* create_reg_proxy(void);
  void create_and_send_request(packet_t*);
  packet_t* create_config(void);
#endif /* PACKET_CREATOR_H_ */
/** @} */
