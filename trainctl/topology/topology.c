/*
 * inertia.c
 *
 *  Created on: Jun 21, 2021
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2021
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#include <stddef.h>
#include <memory.h>



#ifndef TRACKPLAN_TESTPGM
#include "railconfig.h"
#else

#include <stdio.h>
#include <stdlib.h>

#define NUM_TURNOUTS 16
#define DBG_TURNOUTS 1
#define itm_debug3(_fl, _msg, _a, _b, _c) do {printf(_msg  "%d %d %d", _a, _b, _c);} while(0)
#endif

#include "topology.h"


int _blk_num_for_sub_num(int subnum)
{
	if (subnum == 2) return 0;
	if (subnum == 1) return 1;
	if (subnum == 0) return 2;
	return -1;
}


int _next_block_num(int blknum, uint8_t left)
{
    int a,b,tn;
    next_blocks_nums(blknum, left, &a, &b, &tn);
    if (tn>=0) {
        a = topology_get_turnout(tn) ? b : a;
    }
    // sanity */
    if ((a>15) || (b>15)) {
    	// log
    	if (a>15) a = -1;
    	if (b>15) b = -1;
    }
    if ((a<0) && (b<0)) return -2; // end of track
    return a;
}

typedef struct {
	uint8_t lencm;
    uint8_t left1;
    uint8_t left2;
    uint8_t ltn; // leeee turnout
    uint8_t right1;
    uint8_t right2;
    uint8_t rtn; // leeee turnout
} topo_seg_t;

#define TOPOLOGY 0
 
static const topo_seg_t Topology[] = {
#if TOPOLOGY == 1
    // trackplan test
    /* 0 */ { 84, 0xFF, 0xFF, 0xFF,   2,    0xFF, 0},
    /* 1 */ { 42, 0xFF, 0xFF, 0xFF,   2,    0xFF, 0},
    /* 2 */ { 73,    1,    0,      3,    0xFF, 0xFF},
    /* 3 */ { 32,    0xFF, 0xFF,   0xFF, 0xFF, 0xFF}
#elif TOPOLOGY == 2
    // partial layout, 3 segs
    /* 0 */ {84, 0xFF, 0xFF, 0xFF,   1,    0xFF, 0},
    /* 1 */ {42,  2,    0,       0,   0xFF, 0xFF, 0xFF},
    /* 2 */ {73,  0xFF, 0xFF, 0xFF,   1,    0xFF, 0},
#elif TOPOLOGY == 0
    // layout 5 segs
    /* 0 */ { 84,   0xFF, 0xFF, 0xFF,      1,    0xFF, 0},
    /* 1 */ { 42, 	0,    2,       0,      0xFF, 3,    1},
    /* 2 */ { 73, 	0xFF, 0xFF, 0xFF,      0xFF, 1,    0},
    /* 3 */ { 32, 	4,    1,       1,      0xFF, 0xFF, 0xFF},
    /* 4 */ { 105, 	0xFF, 0xFF, 0xFF,      3,    0xFF, 1}
#else
    /* 0 */ { 84, 0xFF, 0xFF, 0xFF,   1,    0xFF, 0},
    /* 1 */ { 42, 0,    2,       0,   0xFF, 0xFF, 0xFF},
    /* 2 */ { 73, 0xFF, 0xFF, 0xFF,   1,    0xFF, 0},
#error bad TOPOLOGY value
#endif
};

void next_blocks_nums(int blknum, uint8_t left, int *pb1, int *pb2, int *tn)
{
    *pb1 = -1;
    *pb2 = -1;
    *tn = -1;
    if (blknum<0) {
        abort();
        return;
    }
    if (left) {
        *pb1 = Topology[blknum].left1;
        *pb2 = Topology[blknum].left2;
        *tn =  Topology[blknum].ltn;
    } else {
        *pb1 = Topology[blknum].right1;
        *pb2 = Topology[blknum].right2;
        *tn =  Topology[blknum].rtn;
    }
    if (*pb1 == 0xFF) *pb1 = -1;
    if (*pb2 == 0xFF) *pb2 = -1;
    if (*tn  == 0xFF) *tn  = -1;
}

int get_blk_len(int blknum)
{
	return Topology[blknum].lencm;
	/*
	switch (blknum) {
	case 0:
		return 70;
	case 1:
		return 40;
	case 2:
		return 50;
	default:
		return 30;
	}
	*/
}


// --------------------------------------------------------------------------------------

static volatile uint32_t turnoutvals = 0; // bit field

void topolgy_set_turnout(int tn, int v)
{
	if (tn >= NUM_TURNOUTS) return;
	if (tn<0) return;
	if (tn>31) return;

	if (v) {
		__sync_fetch_and_or(&turnoutvals, (1<<tn));
	} else {
		__sync_fetch_and_and(&turnoutvals, ~(1<<tn));
	}
	itm_debug3(DBG_TURNOUT, "tt",tn,v, topology_get_turnout(tn));
}
int topology_get_turnout(int tn)
{
	if (tn >= NUM_TURNOUTS) return 0;
	if (tn<0) return 0;
	if (tn>31) return 0;

	uint32_t b = turnoutvals;
	return (b & (1<<tn)) ? 1 : 0;
}


