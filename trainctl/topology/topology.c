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


#ifdef TOPOLOGY_SVG
#define TRACKPLAN_TESTPGM
#endif


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
#include "topologyP.h"


#ifdef TOPOLOGY_SVG
#define _PTS(pi, ...)  ,pi,{__VA_ARGS__}
#define _VP {-1, -1}
#define L0 (5)
#else
#define _PTS(...)
#endif

#define FUTURE_CANTON 0xFF

static const topo_lsblk_t _Topology[] = {
#ifdef UNIT_TEST
    // layout 5 segs
    //          canton         ina    steep  len      l1    l2    tn       r1   r2   tn      graph pt
    /* 0 */ { MA_CANTON(0, 0),  0xFF,   0, 98,    -1,   -1, 0xFF,       1,  -1,    0       _PTS(2, {1,3}, {4,3},{5,4},{5,8})},
    /* 1 */ { MA_CANTON(0, 1),  0xFF,   0, 45,     0,    2,    0,      -1,   3,    1       _PTS(0, {5,9}, {5,12}, _VP, _VP)},
    /* 2 */ { MA_CANTON(0, 2),  0xFF,   0, 90,    -1 ,  -1, 0xFF,      -1,   1,    0       _PTS(2, {1,4}, {3,4}, {4,5}, {4,8})},
    /* 3 */ { MA_CANTON(0, 3),  0xFF,   0, 54, 	4,    1,    1,      -1,  -1, 0xFF          _PTS(0, {6,13}, {6,15}, {5,16}, {1,16})},
    /* 4 */ { MA_CANTON(0, 3),  0xFF,  -1, 80,   -1,   5,     2,       3,  -1,    1        _PTS(0, {7,4}, {7,8}, {6,10}, {6,12})},
    /* 5 */ { MA_CANTON(0, 3),  0xFF,   0, 58,   -1,  -1,  0xFF,       6,   4,    2        _PTS(0, {1,2}, {5,2}, {6,3}, _VP)},
    /* 6 */ { MA_CANTON(0, 3),  0xFF,   0, 36,    5,   -1,    2,       7, -1, 0xFF         _PTS(0, {6,4}, {6,6}, _VP, _VP)},
    /* 7*/  { FUTURE_CANTON,    0xFF,   0, 60,    6,  -1,  0xFF,       8,   -1, 0xFF       _PTS(3, {6,6}, {6,8}, {7,10} ,{7, 14}) },
    /* 8*/  { FUTURE_CANTON,    0xFF,   0, 60,    7,  -1,  0xFF,       -1,  -1, 0xFF       _PTS(1, {7,14}, {7,16}, {6,17}, {1, 17})}
#else
    //          canton         ina    steep  len      l1    l2    tn       r1   r2   tn      graph pt
       /* 0 */ { MA_CANTON(0, 0),  0xFF,   0, 98,    -1,   -1, 0xFF,       1,  -1,    0    _PTS(2, {L0+1,3}, {L0+4,3},{L0+5,4},{L0+5,9})},
    /* 1 */ { MA_CANTON(0, 1),  0xFF,   0, 23,     0,    3,    0,       4,  12 ,   5       _PTS(0, {L0+5,10}, {L0+5,11}, _VP, _VP)},
       /* 2 */ { MA_CANTON(0, 2),  0xFF,   0, 70,    -1 ,  -1, 0xFF,       3,  11,    4    _PTS(2, {L0+1,4}, {L0+3,4}, {L0+4,5}, {L0+4,6})},
       /* 3 */ { MA_CANTON(0, 2),  0xFF,   0, 20,     2,   -1,    4,      -1,   1,    0    _PTS(0, {L0+4,7}, {L0+4, 9}, _VP, _VP)},
       /* 4 */ { MA_CANTON(0, 1),  0xFF,   0, 22,     1,   -1,    5,      -1,   5,    1    _PTS(0, {L0+5, 12}, {L0+5, 14}, _VP, _VP)},
       
       /* 5 */ { MA_CANTON(0, 3),  0xFF,   0, 54,     6,    4,    1,      -1,   -1, 0xFF   _PTS(0, {L0+6,15}, {L0+6,17}, {L0+5,18}, {L0+1,18})},
       /* 6 */ { MA_CANTON(0, 3),  0xFF,  -1, 80,   -1,    7,     2,       5,   -1,    1   _PTS(0, {L0+7,4}, {L0+7,8}, {L0+6,10}, {L0+6,14})},
       /* 7 */ { MA_CANTON(0, 3),  0xFF,   0, 58,   -1,  -1,   0xFF,       8,    6,    2   _PTS(0, {L0+1,2}, {L0+5,2}, {L0+6,3}, _VP)},
       /* 8 */ { MA_CANTON(0, 3),  0xFF,   0, 36,    7,   -1,     2,       9,   -1, 0xFF   _PTS(0, {L0+6,4}, {L0+6,6}, _VP, _VP)},
       /* 9*/  { FUTURE_CANTON,    0xFF,   0, 60,    8,  -1,   0xFF,       10,  -1, 0xFF   _PTS(3, {L0+6,6}, {L0+6,8}, {L0+7,10} ,{L0+7, 14}) },
       /* 10*/ { FUTURE_CANTON,    0xFF,   0, 60,    9,  -1,   0xFF,       -1,  -1, 0xFF   _PTS(0, {L0+7,14}, {L0+7,18}, {L0+6,19}, {L0+1, 19}) },
                                                      
       /* 11*/ { FUTURE_CANTON,    0xFF,   0, 20,    -1,  2,      4,       -1,  -1, 0xFF   _PTS(0, {L0+3,7}, {L0+2,8}, _VP, _VP) },
       /* 12*/ { FUTURE_CANTON,    0xFF,   0, 20,    -1,  1,      5,       13,  -1,    6   _PTS(0, {L0+4,12}, {L0+3, 13}, _VP, _VP) },
       /* 13*/ { FUTURE_CANTON,    0xFF,   0, 20,    12, 15,      6,       14,  -1,    7   _PTS(0, {L0+2,14}, {L0+1, 15}, _VP, _VP) },
       /* 14*/ { FUTURE_CANTON,    0xFF,   0, 20,    13, 16,      7,       -1,  -1, 0xFF   _PTS(0, {L0+0,16}, {L0-1, 17}, _VP, _VP) },
    
       /* 15*/ { FUTURE_CANTON,    0xFF,   0, 20,    -1, -1,   0xFF,       -1,  -1,    6   _PTS(0, {L0+2,10}, {L0+2, 13}, _VP, _VP) },
       /* 16*/ { FUTURE_CANTON,    0xFF,   0, 20,    17, 19,     8,       -1,  -1,    7    _PTS(0, {L0+0,14}, {L0+0, 15}, _VP, _VP) },
    
       /* 17*/ { FUTURE_CANTON,    0xFF,   0, 20,    18, 20,      9,       16,  -1,    8   _PTS(0, {L0+0, 11}, {L0+0, 13}, _VP, _VP) },
       /* 18*/ { FUTURE_CANTON,    0xFF,   0, 20,    -1, -1,   0xFF,       17,  -1,    9   _PTS(0, {L0+0, 4}, {L0+0, 10}, _VP, _VP)},
    
       /* 19*/ { FUTURE_CANTON,    0xFF,   0, 20,    -1, -1,   0xFF,       -1,  16,    8   _PTS(0, {L0-1,4}, {L0-1, 13}, _VP, _VP)},
       /* 20*/ { FUTURE_CANTON,    0xFF,   0, 20,    -1, -1,   0xFF,       -1,  17,    9   _PTS(0, {L0+1,8}, {L0+1, 10}, _VP, _VP)},

#endif
    
};


static inline  int numTopology(void)
{
    static int s=0;
    if (!s) {
        s = sizeof(_Topology)/sizeof(topo_lsblk_t);
    }
    return s;
}

static inline const topo_lsblk_t *Topology(lsblk_num_t blknum)
{
    return &_Topology[blknum.n];
}

const topo_lsblk_t *topology_get_sblkd(int lsblk)
{
    lsblk_num_t n;
    n.n = lsblk;
    return Topology(n);
}
uint8_t get_lsblk_ina3221(lsblk_num_t num)
{
    return topology_get_sblkd(num.n)->ina_segnum;
}


int topology_num_sblkd(void)
{
    return numTopology();
}

void next_lsblk_nums(lsblk_num_t blknum, uint8_t left, lsblk_num_t *pb1, lsblk_num_t *pb2, int *tn)
{
    pb1->n = -1;
    pb2->n = -1;
    *tn = -1;
    if (blknum.n<0) {
        abort();
        return;
    }
    if (left) {
        pb1->n = Topology(blknum)->left1;
        pb2->n = Topology(blknum)->left2;
        *tn =  Topology(blknum)->ltn;
    } else {
        pb1->n = Topology(blknum)->right1;
        pb2->n = Topology(blknum)->right2;
        *tn =  Topology(blknum)->rtn;
    }
    if ((pb1->n>=0) && (Topology(*pb1)->canton_addr == 0xFF)) {
        pb1->n = -1; // inactive/future lsblk
    }
    if ((pb2->n>=0) && (Topology(*pb2)->canton_addr == 0xFF)) {
        pb2->n = -1; // inactive/future lsblk
    }
    if (*tn  == 0xFF) *tn  = -1;
}

int get_lsblk_len(lsblk_num_t blknum, int8_t *psteep)
{
    const topo_lsblk_t *t = Topology(blknum);
    if (psteep) *psteep = t->steep;
	return t->length_cm;
	
}

lsblk_num_t next_lsblk(lsblk_num_t blknum, uint8_t left, uint8_t *palternate)
{
    if (palternate) *palternate = 0;
    if (blknum.n == -1) return blknum;
    
    lsblk_num_t a, b;
    int tn;
    next_lsblk_nums(blknum, left, &a, &b, &tn);
    if (tn>=0) {
        if (palternate) *palternate = 1;
        a = topology_get_turnout(tn) ? b : a;
    }
    // sanity XXX KO with virtual canton */
    if ((a.n>120) || (b.n>120)) {
        // log
        if (a.n>120) a.n = -1;
        if (b.n>120) b.n = -1;
    }
    //if ((a.n<0) && (b.n<0)) return a; // end of track
    return a;
}


uint8_t canton_for_lsblk(lsblk_num_t n)
{
    if (n.n<0) return 0xFF;
    return Topology(n)->canton_addr;
}

lsblk_num_t first_lsblk_with_canton(uint8_t ca,  lsblk_num_t fromblk)
{
    if (0xFF == ca) {
        lsblk_num_t n = {-1};
        return n;
    }
    for (int i=0; i<numTopology(); i++) {
        lsblk_num_t n;
        n.n = i;
        const topo_lsblk_t *t = Topology(n);
        if (t->canton_addr != ca) continue;
        if (   (t->left1  != fromblk.n)
            && (t->left2  != fromblk.n)
            && (t->right1 != fromblk.n)
            && (t->right2 != fromblk.n)) continue;
        return n;
    }
    lsblk_num_t n = {-1};
    return n;
}
lsblk_num_t any_lsblk_with_canton(uint8_t ca)
{
    if (0xFF == ca) {
        lsblk_num_t n = {-1};
        return n;
    }
    for (int i=0; i<numTopology(); i++) {
        lsblk_num_t n;
        n.n = i;
        const topo_lsblk_t *t = Topology(n);
        if (t->canton_addr != ca) continue;
        return n;
    }
    lsblk_num_t n = {-1};
    return n;
}

uint8_t next_block_addr(uint8_t blkaddr, uint8_t left)
{
    // slow should not be used ?
    lsblk_num_t f = any_lsblk_with_canton(blkaddr);
    lsblk_num_t n = next_lsblk(f, left, NULL);
    if (n.n<0) return 0xFF;
    return canton_for_lsblk(n);
}
// --------------------------------------------------------------------------------------

static volatile uint32_t turnoutvals = 0; // bit field
uint8_t topology_or_occupency_changed = 0;

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
    topology_or_occupency_changed = 1;
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


