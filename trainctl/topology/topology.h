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
void next_lsblk_nums(lsblk_num_t blknum, uint8_t left, lsblk_num_t *pb1, lsblk_num_t *pb2, xtrnaddr_t *ptn);


/// next_lsblk
///
/// returns next lsblk, for a given direction
/// @param blknum curent lsblk
/// @param left 0=right, 1=left
/// @param palternate will be set to 1 if different turnout postion may lead to a route

lsblk_num_t next_lsblk(lsblk_num_t blknum, uint8_t left, int8_t *palternate);

// ---------------------------------------------------------------------

xblkaddr_t canton_for_lsblk(lsblk_num_t n);

lsblk_num_t first_lsblk_with_canton(xblkaddr_t ca, lsblk_num_t fromblk);

lsblk_num_t any_lsblk_with_canton(xblkaddr_t ca);


// ---------------------------------------------------------------------

//int get_blk_len(int blknum);
int get_lsblk_len_cm(lsblk_num_t num, int8_t *psteep);

// ---------------------------------------------------------------------

// turnouts NUM_TURNOUTS
// this only set internal topology variables, and does not actually change
// the turnout. ctrl.c does this
// 0=straight (TURNOUT_A), 1=turn (TURNOUT_B)

enum topo_turnout_state {
    topo_tn_straight = 0,
    topo_tn_turn = 1,
    topo_tn_undetermined = -1,
    topo_tn_moving = -2
};

int topology_set_turnout(xtrnaddr_t tn,  enum topo_turnout_state v, int numtrain);
enum topo_turnout_state  topology_get_turnout(xtrnaddr_t tn);


void topology_get_cantons_for_turnout(xtrnaddr_t turnout, xblkaddr_t *head, xblkaddr_t *straight, xblkaddr_t *turn);

// ---------------------------------------------------------------------

// get ina3221 associated with a sblk
uint8_t  get_lsblk_ina3221(lsblk_num_t num);

// for presence detection, retrive all ina3221 belonging to a canton
uint16_t get_ina_bitfield_for_canton(int cnum); // XXX TODO not ok for multiboard

xblkaddr_t get_canton_for_ina(int ina);

// get lsblk associated with a given ina3221
// several lsblk may share the same ina3221 (using POSE only between these lsblk)

// TODO
#endif /* TOPOLOGY_TOPOLOGY_H_ */
