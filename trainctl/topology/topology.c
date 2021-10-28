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




typedef struct {
    uint8_t canton_addr;
    uint8_t ina_segnum;
    
	uint8_t length_cm;
    
    uint8_t left1;
    uint8_t left2;
    uint8_t ltn; // leeee turnout
    uint8_t right1;
    uint8_t right2;
    uint8_t rtn; // leeee turnout
} topo_lsblk_t;



static const topo_lsblk_t _Topology[] = {
    // layout 5 segs
    /* 0 */ { MA_CANTON(0, 0),  0xFF,   84,     0xFF, 0xFF, 0xFF,      1,    0xFF, 0},
    /* 1 */ { MA_CANTON(0, 1),  0xFF,   42,     0,    2,       0,      0xFF, 3,    1},
    /* 2 */ { MA_CANTON(0, 2),  0xFF,   73, 	0xFF, 0xFF, 0xFF,      0xFF, 1,    0},
    /* 3 */ { MA_CANTON(0, 3),  0xFF,   32, 	4,    1,       1,      0xFF, 0xFF, 0xFF},  // unused
    /* 4 */ { MA_CANTON(0, 3),  0xFF,   105, 	0xFF, 0xFF, 0xFF,      3,    0xFF, 1}
};


static inline int numTopology(void)
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
    //if (pb1->n == 0xFF) pb1->n = -1;
    //if (pb2->n == 0xFF) pb2->n = -1;
    if (*tn  == 0xFF) *tn  = -1;
}

int get_lsblk_len(lsblk_num_t blknum)
{
	return Topology(blknum)->length_cm;
	
}

lsblk_num_t next_lsblk(lsblk_num_t blknum, uint8_t left)
{
    if (blknum.n == -1) return blknum;
    
    lsblk_num_t a, b;
    int tn;
    next_lsblk_nums(blknum, left, &a, &b, &tn);
    if (tn>=0) {
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
    lsblk_num_t n = next_lsblk(f, left);
    if (n.n<0) return 0xFF;
    return canton_for_lsblk(n);
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


