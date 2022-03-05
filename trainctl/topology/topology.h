/*
 * topology.h
 *
 *  Created on: Apr 21, 2021
 *      Author: danielbraun
 */

#ifndef TOPOLOGY_TOPOLOGY_H_
#define TOPOLOGY_TOPOLOGY_H_


#include "../msg/trainmsg.h"

extern uint8_t topology_or_occupency_changed;


/*
Cantons (Blocks), share common electrical power, and therefore there can be only one train on a given block
Blocks are divided in Logical SubBlocks (one or more Logical Blocks per Canton)

Logical SubBlocks may use current dectection (ina3221), or only rely on BEMF POSition Evaluation to
detect on wich Logical SubBlocks a train is positioned

Topology is based on Logical SubBlocks
*/


int topology_num_sblkd(void);


// ---------------------------------------------------------------------
/*
int _blk_num_for_sub_num(int subnum);
int _next_block_num(int blknum, uint8_t left);
int _next_sub_num_for_sub_num(int subnum, uint8_t left);
*/

/* in order to identify places where there might be confusion betweeen addr and lsblk we define
 * a struct to avoid implicit casts
 */

typedef struct {
    int8_t n;
} lsblk_num_t;


/// next_lsblk_nums
/// @param blknum current lsblk
/// @param left 0=right, 1=left
/// @param pb1 returns next lsblk if turnout=0
/// @param pb2 returns next lsblk if turnout=1
/// @param ptn returns turnout number, 0xFF if none
void next_lsblk_nums(lsblk_num_t blknum, uint8_t left, lsblk_num_t *pb1, lsblk_num_t *pb2, int *ptn);


/// next_lsblk
///
/// returns next lsblk, for a given direction
/// @param blknum curent lsblk
/// @param left 0=right, 1=left
/// @param palternate will be set to 1 if different turnout postion may lead to a route

lsblk_num_t next_lsblk(lsblk_num_t blknum, uint8_t left, uint8_t *palternate);

// ---------------------------------------------------------------------

uint8_t canton_for_lsblk(lsblk_num_t n);

lsblk_num_t first_lsblk_with_canton(uint8_t ca, lsblk_num_t fromblk);

lsblk_num_t any_lsblk_with_canton(uint8_t ca);


// ---------------------------------------------------------------------

//int get_blk_len(int blknum);
int get_lsblk_len(lsblk_num_t num, int8_t *psteep);

// ---------------------------------------------------------------------

// turnouts NUM_TURNOUTS
// this only set internal topology variables, and does not actually change
// the turnout. ctrl.c does this
// 0=straight (TURNOUT_A), 1=turn (TURNOUT_B)

int topology_set_turnout(int tn, int v, int numtrain);
int topology_get_turnout(int tn);


void topology_get_cantons_for_turnout(uint8_t turnout, uint8_t *head, uint8_t *straight, uint8_t *turn);

// ---------------------------------------------------------------------

uint8_t get_lsblk_ina3221(lsblk_num_t num);
uint16_t get_ina_bitfield_for_canton(int cnum);


#endif /* TOPOLOGY_TOPOLOGY_H_ */
