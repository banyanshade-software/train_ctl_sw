/*
 * topology.h
 *
 *  Created on: Apr 21, 2021
 *      Author: danielbraun
 */

#ifndef TOPOLOGY_TOPOLOGY_H_
#define TOPOLOGY_TOPOLOGY_H_


#include "../msg/trainmsg.h"


/*
Cantons (Blocks), share common electrical power, and therefore there can be only one train on a given block
Blocks are divided in Logical SubBlocks (one or more Logical Blocks per Canton)

Logical SubBlocks may use current dectection (ina3221), or only rely on BEMF POSition Evaluation to
detect on wich Logical SubBlocks a train is positioned

Topology is based on Logical SubBlocks
*/


/*
static inline int _sub_addr_to_sub_num(uint8_t addr, uint8_t sub)
{
    int brd = MA_2_BOARD(addr);
    int n = brd * 12 + sub;
    return n;
}

static inline int _blk_addr_to_blk_num(uint8_t addr)
{
    int brd = MA_2_BOARD(addr);
    int nc = addr & 0x07;
    return brd*6+nc;
}

static inline uint8_t _sub_num_to_sub_addr(int subnum, uint8_t *sub)
{
	if (subnum == -1) {
		*sub = 0;
		return 0xFF;
	}
    *sub = subnum % 12;
    int brd = subnum/12;
    return MA_CANTON(brd, 0);
}

static uint8_t _blk_num_to_blk_addr(int blknum)
{
	if (blknum == -1) return 0xFF;
    int nc = blknum % 6;
    int brd = blknum/6;
    return MA_CANTON(brd, nc);
}
*/

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


void next_lsblk_nums(lsblk_num_t blknum, uint8_t left, lsblk_num_t *pb1, lsblk_num_t *pb2, int *t);
lsblk_num_t next_lsblk(lsblk_num_t blknum, uint8_t left);

// ---------------------------------------------------------------------

uint8_t canton_for_lsblk(lsblk_num_t n);

lsblk_num_t first_lsblk_with_canton(uint8_t ca, lsblk_num_t fromblk);

lsblk_num_t any_lsblk_with_canton(uint8_t ca);

// ---------------------------------------------------------------------

//uint8_t next_block_addr(uint8_t blkaddr, uint8_t left);

/*
static inline uint8_t blk_addr_for_sub_addr(uint8_t addr, uint8_t sub)
{
    int n = _sub_addr_to_sub_num(addr, sub);
    n = _blk_num_for_sub_num(n);
    return _blk_num_to_blk_addr(n);
}

static inline uint8_t next_block_addr(uint8_t blkaddr, uint8_t left)
{
    int blknum = _blk_addr_to_blk_num(blkaddr);
    int n = _next_block_num(blknum, left);
    return _blk_num_to_blk_addr(n);
}

static inline int next_sub_block_addr(uint8_t subaddr, uint8_t sub, int left, uint8_t *rsub)
{
    int snum = _sub_addr_to_sub_num(subaddr, sub);
    snum = _next_sub_num_for_sub_num(snum, left);
    return _sub_num_to_sub_addr(snum, rsub);
}
*/
// ---------------------------------------------------------------------

//int get_blk_len(int blknum);
int get_lsblk_len(lsblk_num_t num);

// ---------------------------------------------------------------------

// turnouts NUM_TURNOUTS
// this only set internal topology variables, and does not actually change
// the turnout. ctrl.c does this
// 0=straight (TURNOUT_A), 1=turn (TURNOUT_B)

void topolgy_set_turnout(int tn, int v);
int topology_get_turnout(int tn);

#endif /* TOPOLOGY_TOPOLOGY_H_ */
