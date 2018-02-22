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
 *         FlowTable for Contiki.
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

#include "flowtable.h"
#include "packet-buffer.h"
#include "sdn-wise.h"
#include "packet-creator.h"

#define W_OP_BIT 5
#define W_OP_LEN 3
#define W_OP_INDEX  0

#define W_LEFT_BIT 3
#define W_LEFT_LEN 2
#define W_LEFT_INDEX 1

#define W_RIGHT_BIT  1
#define W_RIGHT_LEN  W_LEFT_LEN
#define W_RIGHT_INDEX  3

#define W_SIZE_BIT 0
#define W_SIZE_LEN 1

#define S_OP_INDEX 0 
#define S_OP_BIT 3
#define S_OP_LEN 3

#define S_LEFT_BIT 1
#define S_LEFT_LEN 2
#define S_LEFT_INDEX 3  

#define S_RIGHT_BIT 6
#define S_RIGHT_LEN S_LEFT_LEN
#define S_RIGHT_INDEX 5

#define S_RES_BIT 0
#define S_RES_LEN 1
#define S_RES_INDEX 1 

#define STATS_SIZE  2
#define STATUS_REGISTER_SIZE  20

#define GET_BITS(b,s,n) (((b) >> (s)) & ((1 << (n)) - 1))
#define SET_BITS(b,s) ((b) << (s))  

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
  LIST(flowtable);
  MEMB(entries_memb, entry_t, 10);
  MEMB(windows_memb, window_t, 30);
  MEMB(actions_memb, action_t, 20);
  MEMB(bytes_memb, byte_t, 50);
  uint8_t status_register[STATUS_REGISTER_SIZE];
/*----------------------------------------------------------------------------*/
  static void print_window(window_t*);
  static void print_action(action_t*);
  static window_t* window_allocate(void);
  static void window_free(window_t*);
  static action_t* action_allocate(void);
  static void action_free(action_t*);
  static entry_t* entry_allocate(void);
  static void entry_free(entry_t*);
  static void purge_flowtable(void);
  static void entry_init(entry_t*);
  static int compare(operator_t, uint16_t, uint16_t);
  static uint16_t get_operand(packet_t*, uint8_t*, operator_size_t, 
    operator_location_t, uint16_t);
  static uint16_t do_operation(set_operator_t, uint16_t, uint16_t);
  static byte_t* byte_allocate(void);
  static void set_action(packet_t*, uint8_t*, uint8_t*);
/*----------------------------------------------------------------------------*/
// TODO check what happens when memory is full
/*----------------------------------------------------------------------------*/
  entry_t*
  create_entry(void)
  {
    return entry_allocate();
  }
/*----------------------------------------------------------------------------*/
  window_t* 
  create_window(void)
  {
    return window_allocate();
  }
/*----------------------------------------------------------------------------*/  
  action_t* 
  create_action(action_type_t type, uint8_t* array, uint8_t len)
  {
    action_t* a = action_allocate();
    if (a != NULL)
    {
      if(len > 0){
        uint8_t i;
        for(i = 0; i < len; ++i)
        {
          byte_t* b = byte_allocate();
          if (b != NULL){
            b->value = array[i];
            list_add(a->bytes,b);
          }
        }
      }
      a->type = type;   
    }
    return a;
  }
/*----------------------------------------------------------------------------*/    
  void 
  add_window(entry_t* entry, window_t* w)
  {
    list_add(entry->windows,w);
  }
/*----------------------------------------------------------------------------*/    
  void 
  add_action(entry_t* entry, action_t* a)
  {
    list_add(entry->actions,a); 
  }
/*----------------------------------------------------------------------------*/
  static void 
  print_window(window_t* w)
  {
    switch(w->lhs_location)
    {
      case NULL_LOC: break;
      case CONST: break;
      case PACKET: PRINTF("P."); break;
      case STATUS: PRINTF("R."); break;
    }

    PRINTF("%d",w->lhs);

    switch(w->operation)
    {
      case EQUAL: PRINTF(" = "); break;
      case NOT_EQUAL: PRINTF(" != "); break;  
      case GREATER: PRINTF(" > "); break; 
      case LESS: PRINTF(" < "); break; 
      case GREATER_OR_EQUAL: PRINTF(" >= "); break; 
      case LESS_OR_EQUAL: PRINTF(" <= "); break;      
    }

    switch(w->rhs_location)
    {
      case NULL_LOC: break;
      case CONST: break;
      case PACKET: PRINTF("P."); break;
      case STATUS: PRINTF("R."); break;
    }

    PRINTF("%d",w->rhs); 
  }
/*----------------------------------------------------------------------------*/
  static void 
  print_action(action_t* a)
  {
    byte_t *b;

    switch(a->type){
      case NULL_TYPE: break;
      case FORWARD_U: PRINTF("FORWARD_U "); break;
      case FORWARD_B: PRINTF("FORWARD_B "); break;
      case DROP: PRINTF("DROP"); break;
      case ASK: PRINTF("ASK"); break;
      case FUNCTION: PRINTF("FUNCTION "); break;
      case SET_: PRINTF("SET "); break;
      case MATCH: PRINTF("MATCH"); break;
    }

    for(b = list_head(a->bytes); b != NULL; b = b->next) {
      PRINTF(" %d", b->value);
    }
  }
/*----------------------------------------------------------------------------*/
  void 
  print_entry(entry_t* e)
  {
    window_t *w;
    action_t *a;
    PRINTF("IF (");
    for(w = list_head(e->windows); w != NULL; w = w->next) {
      print_window(w);
      PRINTF(";");
    }
    PRINTF("){");
    for(a = list_head(e->actions); a != NULL; a = a->next) {
      print_action(a);
      PRINTF(";");
    } 
    PRINTF("}[%d %d]", e->stats.ttl, e->stats.count);   
  }
/*----------------------------------------------------------------------------*/
  void
  print_flowtable(void)
  {
    entry_t *e;
    for(e = list_head(flowtable); e != NULL; e = e->next) {
      PRINTF("[FLT]: ");
      print_entry(e);
      PRINTF("\n");  
    }
  }
/*----------------------------------------------------------------------------*/
  static window_t *
  window_allocate(void)
  {
    window_t *w;
    w = memb_alloc(&windows_memb);
    if(w == NULL) {
      PRINTF("[FLT]: Failed to allocate a window\n");
    }
    return w;
  }
/*----------------------------------------------------------------------------*/
  static void
  window_free(window_t *w)
  {
    if(memb_free(&windows_memb, w)==-1){
      PRINTF("[FLT]: Failed to free a window\n");
    }
  }
/*----------------------------------------------------------------------------*/
  static byte_t *
  byte_allocate(void)
  {
    byte_t *b;
    b = memb_alloc(&bytes_memb);
    if(b == NULL) {
      PRINTF("[FLT]: Failed to allocate an action\n");
      return NULL;
    }
    b->value = 0;
    return b;
  }
/*----------------------------------------------------------------------------*/
  static void
  byte_free(byte_t *b)
  {
    int res = memb_free(&bytes_memb, b); 
    if (res !=0){
      PRINTF("[FLT]: Failed to free a byte. Reference count: %d\n",res);
    }
  }
/*----------------------------------------------------------------------------*/
  static action_t *
  action_allocate(void)
  {
    action_t *a;
    a = memb_alloc(&actions_memb);
    if(a == NULL) {
      PRINTF("[FLT]: Failed to allocate an action\n");
      return NULL;
    }
    memset(a, 0, sizeof(*a));
    LIST_STRUCT_INIT(a, bytes);
    return a;
  }
/*----------------------------------------------------------------------------*/
  static void
  purge_bytes(list_t bytes)
  {
    byte_t *e;
    byte_t *next;

    for(e = list_head(bytes); e != NULL;) {
      next = e->next;
      byte_free(e);
      e = next;
    }
  }
/*----------------------------------------------------------------------------*/
  static void
  action_free(action_t *a)
  {
    purge_bytes(a->bytes);
    int res = memb_free(&actions_memb, a); 
    if (res !=0){
      PRINTF("[FLT]: Failed to free an action. Reference count: %d\n",res);
    }
  }
/*----------------------------------------------------------------------------*/
  static void
  purge_windows(list_t windows)
  {
    window_t *e;
    window_t *next;

    for(e = list_head(windows); e != NULL;) {
      next = e->next;
      window_free(e);
      e = next;
    }
  }
/*----------------------------------------------------------------------------*/
  static void
  purge_actions(list_t actions)
  {
    action_t *e;
    action_t *next;

    for(e = list_head(actions); e != NULL;) {
      next = e->next;
      action_free(e);
      e = next;
    }
  }
/*----------------------------------------------------------------------------*/
  static void
  purge_flowtable(void)
  {
    entry_t *e;
    entry_t *next;

    for(e = list_head(flowtable); e != NULL;) {
      next = e->next;
      entry_free(e);
      e = next;
    }
  }
/*----------------------------------------------------------------------------*/
  static void
  entry_init(entry_t *e)
  {
    memset(e, 0, sizeof(*e));
    e->stats.ttl = 0;
    e->stats.count = 0; 
    LIST_STRUCT_INIT(e, windows);
    LIST_STRUCT_INIT(e, actions);
  }
/*----------------------------------------------------------------------------*/
  static entry_t *
  entry_allocate(void)
  {
    entry_t *e;

    e = memb_alloc(&entries_memb);
    if(e == NULL) {
      purge_flowtable();
      e = memb_alloc(&entries_memb);
      if(e == NULL) {
        PRINTF("[FLT]: Failed to allocate an entry\n");
        return NULL;
      }
    }
    entry_init(e);
    return e;
  }
/*----------------------------------------------------------------------------*/
  static void
  entry_free(entry_t *e)
  {
    purge_windows(e->windows);
    purge_actions(e->actions);
    list_remove(flowtable, e);
    int res = memb_free(&entries_memb, e); 
    if (res !=0){
      PRINTF("[FLT]: Failed to free an entry. Reference count: %d\n",res);
    }
  }
/*----------------------------------------------------------------------------*/
  void
  flowtable_init(void)
  {
    list_init(flowtable);
    memb_init(&entries_memb);
    memb_init(&windows_memb);
    memb_init(&actions_memb);
    memb_init(&bytes_memb);
  }
/*----------------------------------------------------------------------------*/
  window_t*
  get_window_from_array(uint8_t* array){
    window_t* w = window_allocate();  
    w->operation = GET_BITS(array[W_OP_INDEX], W_OP_BIT, W_OP_LEN);

    w->size = GET_BITS(array[W_OP_INDEX], W_SIZE_BIT, W_SIZE_LEN);

    w->lhs = MERGE_BYTES(array[W_LEFT_INDEX], array[W_LEFT_INDEX+1]);
    w->lhs_location = GET_BITS(array[W_OP_INDEX], W_LEFT_BIT, W_LEFT_LEN);

    w->rhs = MERGE_BYTES(array[W_RIGHT_INDEX], array[W_RIGHT_INDEX+1]);
    w->rhs_location = GET_BITS(array[W_OP_INDEX], W_RIGHT_BIT, W_RIGHT_LEN);
    return w;
  }
/*----------------------------------------------------------------------------*/
  uint8_t
  get_array_from_window(uint8_t* array, window_t* w){
    array[W_OP_INDEX] = SET_BITS(w->operation, W_OP_BIT) + 
			SET_BITS(w->size, W_SIZE_BIT) + 
			SET_BITS(w->lhs_location, W_LEFT_BIT) + 
			SET_BITS(w->rhs_location, W_RIGHT_BIT);
    array[W_LEFT_INDEX+1] = w->lhs & 0xFF;
    array[W_LEFT_INDEX] = w->lhs >> 8;
    array[W_RIGHT_INDEX+1] = w->rhs & 0xFF;
    array[W_RIGHT_INDEX] = w->rhs >> 8;
    return WINDOW_SIZE;
  }
/*----------------------------------------------------------------------------*/
  action_t*
  get_action_from_array(uint8_t* array, uint8_t action_size){
    return create_action(array[0], &(array[1]), action_size-1);
  }
/*----------------------------------------------------------------------------*/
  uint8_t
  get_array_from_action(uint8_t* array, action_t* a){
    int i = 2;
    array[1] = a->type;
    byte_t *b;
    for(b = list_head(a->bytes); b != NULL; b = b->next) {
      array[i] = b->value;
      i++;
    }
    array[0] = i-1;
    return i; 
  }
/*----------------------------------------------------------------------------*/
  entry_t* 
  get_entry_from_array(uint8_t* array, uint16_t length)
  {
    entry_t* entry = entry_allocate();
    if (entry != NULL){
      uint16_t i = 0;
      uint8_t n_windows = array[i];

      for(i = 1; i <= n_windows; i += WINDOW_SIZE){
        window_t* w = get_window_from_array(&array[i]);
        list_add(entry->windows,w);
      }

      while (i < (length - STATS_SIZE)) {
        uint8_t action_size = array[i];
        i++;
        action_t* a = get_action_from_array(&array[i], action_size);
        list_add(entry->actions,a);     
        i += action_size;
      }

      entry->stats.ttl = array[i];
      i++;
      entry->stats.count = array[i];
    }
    return entry;
  }
/*----------------------------------------------------------------------------*/
  uint8_t 
  get_array_from_entry(uint8_t* array, entry_t* e)
  {  
    uint8_t index = 1;
    array[0] = 0;
    window_t *w;
    action_t *a;
    for(w = list_head(e->windows); w != NULL; w = w->next) {
      index += get_array_from_window(&array[index], w);
      array[0] += WINDOW_SIZE;
    }
    for(a = list_head(e->actions); a != NULL; a = a->next) {
      index += get_array_from_action(&array[index], a);
    } 
    array[index] = e->stats.ttl;
    index++;
    array[index] = e->stats.count; 
    index++;
    return index;
  }
/*----------------------------------------------------------------------------*/
  uint8_t 
  get_array_from_entry_id(uint8_t* array, int entry_id)
  {  
    int i = 0;
    uint8_t size = 0;
    entry_t *e;
    for(e = list_head(flowtable); e != NULL; e = e->next) {
      if (i == entry_id){
      	size += get_array_from_entry(array, e);	
        break;
      }
      i++;
    }
    return size;
  }
/*----------------------------------------------------------------------------*/
  uint8_t 
  window_cmp(window_t* a, window_t* b)
  {  
    print_window(a);
    PRINTF("\n");
    print_window(b);
    PRINTF("\n");
    return a->operation == b->operation &&
           a->size == b->size &&
           a->lhs_location == b->lhs_location &&
           a->rhs_location == b->rhs_location &&
           a->lhs == b->lhs &&
           a->rhs == b->rhs;
  }
/*----------------------------------------------------------------------------*/
  uint8_t 
  entry_cmp(entry_t* a, entry_t* b)
  {  
    if (list_length(a->windows) == list_length(b->windows))
    {
      window_t* aw;
      window_t* bw;
      bw = list_head(b->windows);
      for (aw = list_head(a->windows); aw != NULL; aw = aw->next){
        if (window_cmp(aw,bw)){
          bw = bw->next;
        } else {
          return 0;
        }
      }
      return 1;  
    } return 0;
  }
/*----------------------------------------------------------------------------*/
  void 
  add_entry(entry_t* e){
    entry_t* tmp;
    entry_t* found = NULL;
    for(tmp = list_head(flowtable); tmp != NULL; tmp = tmp->next) {
      if (entry_cmp(e,tmp)){
        found = tmp;
      }
    }

    if (found != NULL){
      entry_free(found);
    }
    list_add(flowtable,e);
  }
/*----------------------------------------------------------------------------*/
  void 
  match_packet(packet_t* p){
    entry_t *e;
    int found = 0;
    PRINTF("[FLT]: Matching Packet...\n");

    for(e = list_head(flowtable); e != NULL; e = e->next) {
      found = match_entry(p,e);
      if (found){
        break;
      }
    }
    if (!found){
      PRINTF("[FLT]: Match Not Found!\n");
      create_and_send_request(p);
    }
  }
/*----------------------------------------------------------------------------*/
  int 
  match_entry(packet_t* p, entry_t* e){
    window_t *w;
    action_t *a;
    int target = list_length(e->windows); 
    int actual = 0;

    if (target == 0) {return 0;}

    for(w = list_head(e->windows); w != NULL; w = w->next) {
      actual = actual + match_window(p, status_register, w);
    }

    if (actual == target){
      PRINTF("[FLT]: Match Found!\n");
      for(a = list_head(e->actions); a != NULL; a = a->next) {
        run_action(p, status_register, a);
      } 
      e->stats.count++;
      return 1;
    } else {
      return 0;
    }  
  }
/*----------------------------------------------------------------------------*/
  static int
  compare(operator_t op, uint16_t lhs, uint16_t rhs)
  {
    switch (op) {
      case EQUAL: return lhs == rhs;
      case NOT_EQUAL: return lhs != rhs;
      case GREATER: return lhs > rhs;
      case LESS: return lhs < rhs;
      case GREATER_OR_EQUAL: return lhs >= rhs;
      case LESS_OR_EQUAL: return lhs <= rhs;
      default: return 0;
    }
  }
/*----------------------------------------------------------------------------*/
  static uint16_t 
  get_operand(packet_t* p, uint8_t* r, operator_size_t s, operator_location_t l, 
    uint16_t v)
  {
    uint8_t* ptr;
    int limit;
    uint8_t* a = (uint8_t*)p;

    switch(l)
    {
      case CONST: return v;
      case PACKET: ptr = a; limit = p->header.len-1; break;
      case STATUS: ptr = r; limit = STATUS_REGISTER_SIZE-1; break;
      default: return 0;
    }

    switch(s)
    {
      case SIZE_1: if (v > limit) { return -1;} else {return ptr[v];}
      case SIZE_2: if (v + 1 > limit) { return -1;} else { return MERGE_BYTES(ptr[v],ptr[v + 1]);}
      default: return 0;
    }
  }
/*----------------------------------------------------------------------------*/
  int 
  match_window(packet_t* p, uint8_t* s, window_t* w)
  {
    operator_t op = w->operation;
    uint16_t lhs = get_operand(p, s, w->size, w->lhs_location, w->lhs);
    uint16_t rhs = get_operand(p, s, w->size, w->rhs_location, w->rhs);
    return compare(op, lhs, rhs);
  }
/*----------------------------------------------------------------------------*/
  static uint16_t 
  do_operation(set_operator_t op, uint16_t lhs, uint16_t rhs)
  {
    switch (op) 
    {
      case ADD: return lhs + rhs;
      case SUB: return lhs - rhs;
      case DIV: return lhs / rhs;
      case MUL: return lhs * rhs;
      case MOD: return lhs % rhs;
      case AND: return lhs & rhs;
      case OR: return lhs | rhs;
      case XOR: return lhs ^ rhs;
      default: return 0;
    }
  }
/*----------------------------------------------------------------------------*/
  static void 
  set_action(packet_t* p, uint8_t* s, uint8_t* action_array)
  {
    set_operator_t op = GET_BITS(action_array[S_OP_INDEX],S_OP_BIT,S_OP_LEN);
      
    operator_location_t lhs_location = GET_BITS(action_array[S_OP_INDEX],S_LEFT_BIT,S_LEFT_LEN);
    uint16_t lhs = MERGE_BYTES(action_array[S_LEFT_INDEX],action_array[S_LEFT_INDEX+1]);
      
    operator_location_t rhs_location = GET_BITS(action_array[S_OP_INDEX],S_RIGHT_BIT,S_RIGHT_LEN);
    uint16_t rhs = MERGE_BYTES(action_array[S_RIGHT_INDEX],action_array[S_RIGHT_INDEX+1]);
      
    operator_location_t res_location = GET_BITS(action_array[S_OP_INDEX],S_RES_BIT,S_RES_LEN)+2;
    uint16_t res = MERGE_BYTES(action_array[S_RES_INDEX],action_array[S_RES_INDEX+1]);
      
    uint16_t l = get_operand(p, s, SIZE_1, lhs_location, lhs);
    uint16_t r = get_operand(p, s, SIZE_1, rhs_location, rhs);

    if (res_location == PACKET){
      ((uint8_t*)p)[res] = do_operation(op, l, r);
    } else {
      s[res] = do_operation(op, l, r);
    }   
  }
/*----------------------------------------------------------------------------*/
  int 
  run_action(packet_t* p, uint8_t* s, action_t* a){
    int len = list_length(a->bytes);
    int i = 0;
    uint8_t action_array[len];
    byte_t* b;

    if (len>0){
      for(b = list_head(a->bytes); b != NULL; b = b->next) {
        action_array[i] = b->value;
        ++i;
      }      
    }  

    switch(a->type){
      case FORWARD_U:
      p->header.nxh = get_address_from_array(action_array);
      rf_unicast_send(p);
      break;

      case FORWARD_B:
      p->header.nxh = get_address_from_array(action_array);
      rf_broadcast_send(p);
      break;

      case ASK:
      create_and_send_request(p);
      break;

      case MATCH:
      // TODO there may be problems if match is in the middle of a list of actions
      match_packet(p);
      break;

      case FUNCTION:
      // TODO function
      break;
      
      case SET_:
      set_action(p, s, action_array);
      break;    

      default: 
      packet_deallocate(p);
      break;
    }
    return -1;
  }
/*----------------------------------------------------------------------------*/
  void 
  test_flowtable(void)
  {
    uint8_t array[25] = {
      20, 18, 0, 6, 0, 2, 82, 0, 15, 0, 1, 114, 
      0, 16, 0, 2, 50, 0, 17, 0, 5, 1, 4, 254, 
      0};

      if (!list_length(flowtable)){
        entry_t* e = get_entry_from_array(array,25);
        if (e != NULL){
          add_entry(e);
        }
      }else{
        entry_free(list_pop(flowtable));
      }

      uint8_t array1[116] = {
        1, 116, 0, 0, 0, 2, 2, 100, 0, 0, 
        11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
        21, 90, 10, 10, 10, 11, 22, 10, 12, 10, 
        35, 11, 14, 18, 16, 12, 10, 10, 10, 40, 
        40, 10, 18, 15, 11, 10, 10, 10, 10, 10, 
        50, 11, 13, 13, 12, 25, 25, 31, 11, 01, 
        61, 11, 17, 18, 16, 13, 10, 11, 10, 12, 
        71, 11, 17, 18, 16, 13, 10, 11, 10, 12, 
        81, 11, 17, 18, 16, 13, 10, 11, 10, 12, 
        91, 11, 17, 18, 16, 13, 10, 11, 10, 12, 
        11, 11, 17, 18, 16, 13, 10, 11, 10, 12, 
        10, 13, 254,11, 12, 13
      };


      packet_t* p = get_packet_from_array(array1);
      match_packet(p);
      packet_deallocate(p);
  }
/*----------------------------------------------------------------------------*/
/** @} */
