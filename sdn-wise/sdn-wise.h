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
 *         Header file for the SDN-WISE Contiki porting.
 * \author
 *         Sebastiano Milardo <s.milardo@hotmail.it>
 */

/**
 * \defgroup sdn-wise SDN-WISE porting
 * @{
 *
 * The SDN-WISE module implements SDN-WISE in Contiki.
 */

#ifndef SDN_WISE_H_
#define SDN_WISE_H_

#include "lib/list.h"
#include "lib/memb.h"
#include "packet-buffer.h"

void rf_unicast_send(packet_t*);
void rf_broadcast_send(packet_t*);

#endif /* SDN_WISE_H_ */
/** @} */
