/*
 *  flow_table.h
 *
 *  Created on: 09 nov 2015
 *      Author: Mario Brischetto
 */

#ifndef _SDN_WISE_FLOW_TABLE_H_
#define _SDN_WISE_FLOW_TABLE_H_
/*************************************************************************/
#include <stdio.h>
#include "util.h"
/*************************************************************************/
#define WINDOW_LEN      4
#define ACTION_LEN      4
#define STATS_LEN       2
#define WINDOWS_MAX     3
#define WINDOWS_LEN     WINDOW_LEN*WINDOWS_MAX
#define RULE_LEN        WINDOWS_LEN + ACTION_LEN + STATS_LEN ///////
#define WINDOW_OP_POS       0
#define WINDOW_POS_POS      1
#define WINDOW_VALUE_POS    2
// memory
#define SDN_WISE_PACKET                         1		//	0000 0001
#define SDN_WISE_STATUS                         0		//	0000 0000
// size
#define SDN_WISE_SIZE_0                         0		//	0000 0000
#define SDN_WISE_SIZE_1                         2		//	0000 0010
#define SDN_WISE_SIZE_2                         4		//	0000 0100
// operators
#define SDN_WISE_EQUAL                          8		//	0000 1000
#define SDN_WISE_NOT_EQUAL                      16		//	0001 0000
#define SDN_WISE_BIGGER                         24		//	0001 1000
#define SDN_WISE_LESS                           32		//	0010 0000
#define SDN_WISE_EQUAL_OR_BIGGER                40		//	0010 1000
#define SDN_WISE_EQUAL_OR_LESS                  48		//	0011 0000
// multimatch
#define SDN_WISE_MULTI                          2		//	0000 0010
#define SDN_WISE_NOT_MULTI                      0		//	0000 0000
// actions
#define SDN_WISE_FORWARD_UNICAST                4		//	0000 0100
#define SDN_WISE_FORWARD_BROADCAST              8		//	0000 1000
#define SDN_WISE_DROP                           12		//	0000 1100
#define SDN_WISE_MODIFY                         16		//	0001 0000
#define SDN_WISE_AGGREGATE                      20		//	0001 0100
#define SDN_WISE_FORWARD_UP                     24		//	0001 1000
#define ACTION_ACT_POS      0
#define ACTION_POS_POS      1
#define ACTION_VALUE_POS    2
#define STATS_TTL_POS   0
#define STATS_COUNT_POS 1
/*************************************************************************/
struct structRuleWindow{
    uint8_t op;         // l operazione di confronto
    uint8_t pos;        // la posizione del byte a partire dal quale effettuare in confronto
    uint8_t value_h;    // il valore con cui effettuare il confronto
    uint8_t value_l;
};
typedef struct structRuleWindow ruleWindow;
struct structRuleAction{
    uint8_t act;        // l azione che dovra essere eseguita
    uint8_t pos;        // questo valore dipende dall'azione
    uint8_t value_h;    // questo valore dipende dall'azione
    uint8_t value_l;    // questo valore dipende dall'azione
};
typedef struct structRuleAction ruleAction;
struct structRuleStats{
    uint8_t ttl;
    uint8_t count;
};
typedef struct structRuleStats ruleStats;
struct structRule{
    ruleWindow windows[WINDOWS_MAX];
    ruleAction action;
    ruleStats stats;
};
typedef struct structRule rule;
/*************************************************************************/
void ruleWindow2array(ruleWindow rulew, uint8_t array[]){
    array[WINDOW_OP_POS] = rulew.op;
    array[WINDOW_POS_POS] = rulew.pos;
    array[WINDOW_VALUE_POS] = rulew.value_h;
    array[WINDOW_VALUE_POS + 1] = rulew.value_l;
}
/*************************************************************************/
void array2ruleWindow(uint8_t array[], ruleWindow *rulew){
    rulew->op = array[WINDOW_OP_POS];
    rulew->pos = array[WINDOW_POS_POS];
    rulew->value_h = array[WINDOW_VALUE_POS];
    rulew->value_l = array[WINDOW_VALUE_POS + 1];
}
/*************************************************************************/
void printRuleWindow(ruleWindow rulew){
    uint8_t rulew_array[WINDOW_LEN];
    ruleWindow2array(rulew, rulew_array);
#if !SINK
    printArray(rulew_array, WINDOW_LEN);
#endif
}
/*************************************************************************/
void ruleAction2array(ruleAction rulea, uint8_t array[]){
    array[ACTION_ACT_POS] = rulea.act;
    array[ACTION_POS_POS] = rulea.pos;
    array[ACTION_VALUE_POS] = rulea.value_h;
    array[ACTION_VALUE_POS +1] = rulea.value_l;
}
/*************************************************************************/
void array2ruleAction(uint8_t array[], ruleAction *rulea){
    rulea->act = array[ACTION_ACT_POS];
    rulea->pos = array[ACTION_POS_POS];
    rulea->value_h = array[ACTION_VALUE_POS];
    rulea->value_l = array[ACTION_VALUE_POS +1];
}
/*************************************************************************/
void printRuleAction(ruleAction rulea){
    uint8_t rulea_array[ACTION_LEN];
    ruleAction2array(rulea, rulea_array);
#if !SINK
    printArray(rulea_array, ACTION_LEN);
#endif
}
/*************************************************************************/
void ruleStats2array(ruleStats rules, uint8_t array[]){
    array[STATS_TTL_POS] = rules.ttl;
    array[STATS_COUNT_POS] = rules.count;
}
/*************************************************************************/
void array2ruleStats(uint8_t array[], ruleStats *rules){
    rules->ttl = array[STATS_TTL_POS];
    rules->count = array[STATS_COUNT_POS];
}
/*************************************************************************/
void printRuleStats(ruleStats rules){
    uint8_t rules_array[STATS_LEN];
    ruleStats2array(rules, rules_array);
#if !SINK
    printArray(rules_array, STATS_LEN);
#endif
}
/*************************************************************************/
void rule2array(rule rule, uint8_t array[], boolean include_stats){
    int k=0;
    uint8_t rulew_array[WINDOW_LEN];
    int i;
    for (i=0; i<WINDOWS_MAX; i++) {
        ruleWindow2array(rule.windows[i], rulew_array);
        int j;
        for (j=0; j<WINDOW_LEN; j++) {
            array[k] = rulew_array[j];
            k++;
        }
    }
    uint8_t rulea_array[ACTION_LEN];
    ruleAction2array(rule.action, rulea_array);
    int j;
    for (j=0; j<ACTION_LEN; j++,k++) {
        array[k] = rulea_array[j];
    }
    if (include_stats == 1) {
        uint8_t rules_array[STATS_LEN];
        ruleStats2array(rule.stats, rules_array);
        int j;
        for (j=0; j<STATS_LEN; j++,k++) {
            array[k] = rules_array[j];
        }
    }
}
/*************************************************************************/
void array2rule(uint8_t array[], rule *rule, boolean include_stats){
	uint8_t tmpWindow[WINDOW_LEN];
	int i;
    for (i=0; i<WINDOWS_MAX; i++) {
    	split(array, tmpWindow, i*WINDOW_LEN, WINDOW_LEN);
        array2ruleWindow(tmpWindow, &rule->windows[i]);
    }
    uint8_t tmpAction[ACTION_LEN];
    split(array, tmpAction, WINDOWS_LEN, ACTION_LEN);
    array2ruleAction(tmpAction, &rule->action);
    if (include_stats == 1){
    	uint8_t tmpStats[STATS_LEN];
    	split(array, tmpStats, WINDOWS_LEN + ACTION_LEN, STATS_LEN);
        array2ruleStats(tmpStats, &rule->stats);
    }
}
/*************************************************************************/
void printRule(rule rule, boolean include_stats){
#if !SINK
    printf("Print Rule\n");

    int i;
    for (i=0; i<WINDOWS_MAX; i++) {
        printRuleWindow(rule.windows[i]);
    }
    printRuleAction(rule.action);
    if (include_stats == 1){
        printRuleStats(rule.stats);
    }
#endif
}
/*************************************************************************/
uint8_t ruleCmp (rule r1, rule r2, boolean include_stats){
	if (include_stats == 1){
		uint8_t r1_a[RULE_LEN];
		uint8_t r2_a[RULE_LEN];
		rule2array(r1, r1_a, 1);
		rule2array(r2, r2_a, 1);
		return arrayCmp(r1_a, r2_a, RULE_LEN);
	} else {
		uint8_t r1_a[RULE_LEN - STATS_LEN];
		uint8_t r2_a[RULE_LEN - STATS_LEN];
		rule2array(r1, r1_a, 0);
		rule2array(r2, r2_a, 0);
		return arrayCmp(r1_a, r2_a, RULE_LEN - STATS_LEN);
	}
}
/*************************************************************************/
#endif // _SDN_WISE_FLOW_TABLE_H_
