/*
 * topology.h
 *
 *  Created on: Apr 21, 2021
 *      Author: danielbraun
 */

#ifndef TOPOLOGY_TOPOLOGY_H_
#define TOPOLOGY_TOPOLOGY_H_

/*
                       /--5.1---- --5.2--
 -1.1-- -1.2--\--2----/-4---/--6.1- -6.2--
               \----3------/
 
 ('1.1 '1.2 (switchr ('2 '3))
 ('2 (switchr '4 ('5.1 5.2)))
 ('6.1 (switchl '4 '3))
 
 l2r : (blk 1.1 ...) (blk 1.2 ...) (sw s 1) (blk 2) (sw_s 2) (blk 4) (sw is 3) (blk 6.1) (blk 6.2)
 l2r : (sw t 1) (3) (sw it 3)
 l2r : (sw t 2) (blk 5.1) (blk 5.2)
 */
#include "../msg/trainmsg.h"

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
    *sub = subnum % 12;
    int brd = subnum/12;
    return MA_CANTON(brd, 0);
}

static uint8_t _blk_num_to_blk_addr(int blknum)
{
    int nc = blknum % 6;
    int brd = blknum/6;
    return MA_CANTON(brd, nc);
}

// ---------------------------------------------------------------------

int _blk_num_for_sub_num(int subnum);
int _next_block_num(int blknum, uint8_t left);
int _next_sub_num_for_sub_num(int subnum, uint8_t left);

// ---------------------------------------------------------------------

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
#endif /* TOPOLOGY_TOPOLOGY_H_ */
