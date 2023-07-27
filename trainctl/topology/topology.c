/*
 * topology.c
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
#define BOARD_HAS_TOPOLOGY
#include "boards_def.h"
#endif


#ifndef TRACKPLAN_TESTPGM
#include "../railconfig.h"
#else
#include <stdio.h>
#include <stdlib.h>

//#define NUM_TURNOUTS 16
#define DBG_TURNOUTS 1
//#define itm_debug3(_fl, _msg, _a, _b, _c) do {printf(_msg  "%d %d %d", _a, _b, _c);} while(0)
#endif



#ifndef BOARD_HAS_TOPOLOGY
#error BOARD_HAS_TOPOLOGY not defined, remove this file from build
#endif


#include "topology.h"
#include "topologyP.h"
#include "occupency.h"
#include "../utils/bitarray.h"
#include "../utils/dynamic_perfect_hash.h"

#ifdef TOPOLOGY_SVG
#define _PTS(pi, ...)  ,pi,{__VA_ARGS__}
#define _VP {-1, -1}
#define L0 (2)
#else
#define _PTS(...)
#include "../oam/oam.h"

#endif

#define FUTURE_CANTON 0xFF

#ifdef TOPOLOGY_SVG
#define FatalError(_m,_l,_e) do { abort(); } while(0)
#endif

/*
unsigned int conf_topology_num_entries(void);
const conf_topology_t *conf_topology_get(int num);
 */

static  const topo_lsblk_t *_Topology = NULL;
static  int _numTopology = 0;

#ifdef UNIT_TEST_____ // now in topology.cnf
static const topo_lsblk_t _Topology1[] = {
        // layout 5 segs
        //          canton         ina    steep  len      l1    l2    tn       r1   r2   tn      graph pt
        /* 0 */ { MA_CANTON(0, 0),  0xFF,   0, 98,    -1,   -1, 0xFF,       1,  -1,    0       _PTS(2, {1,3}, {4,3},{5,4},{5,8})},
        /* 1 */ { MA_CANTON(0, 1),  0xFF,   0, 45,     0,    2,    0,      -1,   3,    1       _PTS(0, {5,9}, {5,12}, _VP, _VP)},
        /* 2 */ { MA_CANTON(0, 2),  0xFF,   0, 90,    -1 ,  -1, 0xFF,      -1,   1,    0       _PTS(2, {1,4}, {3,4}, {4,5}, {4,8})},
        /* 3 */ { MA_CANTON(0, 3),  0xFF,   0, 54,     4,    1,    1,      -1,  -1, 0xFF          _PTS(0, {6,13}, {6,15}, {5,16}, {1,16})},
        /* 4 */ { MA_CANTON(0, 3),  0xFF,  -1, 80,   -1,   5,     2,       3,  -1,    1        _PTS(0, {7,4}, {7,8}, {6,10}, {6,12})},
        /* 5 */ { MA_CANTON(0, 3),  0xFF,   0, 58,   -1,  -1,  0xFF,       6,   4,    2        _PTS(0, {1,2}, {5,2}, {6,3}, _VP)},
        /* 6 */ { MA_CANTON(0, 3),  0xFF,   0, 36,    5,   -1,    2,       7, -1, 0xFF         _PTS(0, {6,4}, {6,6}, _VP, _VP)},
        /* 7*/  { FUTURE_CANTON,    0xFF,   0, 60,    6,  -1,  0xFF,       8,   -1, 0xFF       _PTS(3, {6,6}, {6,8}, {7,10} ,{7, 14}) },
        /* 8*/  { FUTURE_CANTON,    0xFF,   0, 60,    7,  -1,  0xFF,       -1,  -1, 0xFF       _PTS(1, {7,14}, {7,16}, {6,17}, {1, 17})}
};
#endif

static void _readTopology(void)
{
#ifndef TOPOLOGY_SVG
#ifndef TRAIN_SIMU
	if (!Oam_Ready) {
		return;
	}
#endif
#endif
	_numTopology = conf_topology_num_entries();
	_Topology = conf_topology_get(0);
}

static inline  int numTopology(void)
{
	if (!_Topology) _readTopology();
    return _numTopology;
}


static inline const topo_lsblk_t *Topology(lsblk_num_t blknum)
{
	if (!_Topology) _readTopology();
    if ((blknum.n<0) || (blknum.n >=_numTopology)) {
        FatalError("Tphi", "bad lsblk for topo", Error_Lsblk_Invalid);
    }
    return &_Topology[blknum.n];

}

const topo_lsblk_t *topology_get_sblkd(int lsblk)
{
    lsblk_num_t n;
    n.n = lsblk;
    return Topology(n);
}

ina_num_t get_lsblk_ina3221(lsblk_num_t num)
{
    ina_num_t r;
    r.v = topology_get_sblkd(num.n)->ina_segnum;
    return r;
}


int topology_num_sblkd(void)
{
    return numTopology();
}

static void bh(void)
{
}

void next_lsblk_nums(lsblk_num_t blknum, uint8_t left, lsblk_num_t *pb1, lsblk_num_t *pb2, xtrnaddr_t *tn)
{
    pb1->n = -1;
    pb2->n = -1;
    tn->v = -1;
    if (blknum.n<0) {
    	itm_debug1(DBG_ERR|DBG_CTRL, "next_lsblk_nums", 0);
    	FatalError("ABRT", "next_lsblk_nums", Error_Topology_BlkNum);
    	return;
    }

    if (left) {
        pb1->n = Topology(blknum)->left1;
        pb2->n = Topology(blknum)->left2;
        tn->v =  Topology(blknum)->ltn;
    } else {
        pb1->n = Topology(blknum)->right1;
        pb2->n = Topology(blknum)->right2;
        tn->v =  Topology(blknum)->rtn;
    }
    if ((pb1->n>=0) && (Topology(*pb1)->canton_addr == 0xFF)) {
        pb1->n = -1; // inactive/future lsblk
    }
    if ((pb2->n>=0) && (Topology(*pb2)->canton_addr == 0xFF)) {
        pb2->n = -1; // inactive/future lsblk
    }
    //if (tn->v>=6) tn->v = -1; // XXX
    if (tn->v == 4) {
        bh();
        //tn->v = -1; //XXX
    }
    // if (*tn  == 0xFF) tn  = -1;
}

int get_lsblk_len_cm(lsblk_num_t blknum, int8_t *psteep)
{
    const topo_lsblk_t *t = Topology(blknum);
    if (psteep) *psteep = t->steep;
	return t->length_cm;
	
}

#ifdef TOPOLOGY_SVG
enum topo_turnout_state  topology_get_turnout(xtrnaddr_t tn)
{
    return topo_tn_straight;
}
#endif


lsblk_num_t next_lsblk(lsblk_num_t blknum, uint8_t left, int8_t *palternate)
{
    if (palternate) *palternate = 0;
    if (blknum.n == -1) return blknum;
    
    lsblk_num_t a, b;
    xtrnaddr_t tn;
    next_lsblk_nums(blknum, left, &a, &b, &tn);
    //printf("blk %d, left=%d next tn=%d a=%d b=%d\n", blknum.n, left, tn.v, a.n, b.n);
    if (tn.v != 0xFF) {
        if (palternate) *palternate = 1;
        a = (topology_get_turnout(tn) == topo_tn_turn) ? b : a;
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

xblkaddr_t canton_for_lsblk(lsblk_num_t n)
{
    if (n.n<0) {
    	xblkaddr_t r = { .v = 0xFF };
    	return r;
    }
    xblkaddr_t r;
    r.v = Topology(n)->canton_addr;
    return r;
}

static lsblk_num_t _first_lsblk_with_canton(uint8_t ca,  lsblk_num_t fromblk)
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


uint16_t get_ina_bitfield_for_canton(int cnum)
{
	uint16_t r = 0;
    for (int i=0; i<numTopology(); i++) {
        lsblk_num_t n;
        n.n = i;
        const topo_lsblk_t *t = Topology(n);
        if (t->canton_addr != cnum) continue;
        if (t->ina_segnum == 0xFF) continue;
        r |= (1<<t->ina_segnum);
    }
    return r;
}

xblkaddr_t get_canton_for_ina(ina_num_t ina)
{
	xblkaddr_t r;
    get_lsblk_and_canton_for_ina(ina, NULL, &r);
    return r;
    /*
	r.v = 0xFF;
	for (int i=0; i<numTopology(); i++) {
		lsblk_num_t n;
		n.n = i;
		const topo_lsblk_t *t = Topology(n);
		if (t->ina_segnum != ina) continue;
		r.v = t->canton_addr;
		break;
	}
	return r;
     */
}

lsblk_num_t get_lsblk_for_ina(ina_num_t ina)
{
    lsblk_num_t r;
    get_lsblk_and_canton_for_ina(ina, &r, NULL);
    return r;
    /*
    r.n = -1;
    for (int i=0; i<numTopology(); i++) {
        lsblk_num_t n;
        n.n = i;
        const topo_lsblk_t *t = Topology(n);
        if (t->ina_segnum != ina) continue;
        r.n = i;
        break;
    }
    return r;
     */
}

void get_lsblk_and_canton_for_ina(ina_num_t ina, lsblk_num_t *plsblk, xblkaddr_t *pcan)
{
    if (plsblk) plsblk->n = -1;
    if (pcan) pcan->v = 0xFF;
    int nt = numTopology();
    for (int i=0; i<nt; i++) {
        lsblk_num_t n;
        n.n = i;
        const topo_lsblk_t *t = Topology(n);
        if (t->ina_segnum != ina.v) continue;
        if (plsblk) plsblk->n = i;
        if (pcan) pcan->v = t->canton_addr;
        break;
    }
}

lsblk_num_t get_nextlsblk_with_same_ina(lsblk_num_t lsb)
{
    lsblk_num_t r;
    r.n = -1;
    if (lsb.n<0) return r;
    int nt = numTopology();
    if (lsb.n >= nt) return r;
    const topo_lsblk_t *topo0 = Topology(lsb);
    if (topo0->ina_segnum == 0xFF) return r;
    for (int i=lsb.n+1; i<nt; i++) {
        lsblk_num_t n;
        n.n = i;
        const topo_lsblk_t *t = Topology(n);
        if (t->ina_segnum == topo0->ina_segnum) {
            return n;
        }
    }
    return r;
}

static inline lsblk_num_t _lsblk(int n)
{
    lsblk_num_t r;
    r.n = n;
    return r;
}

lsblk_num_t first_lsblk_with_canton(xblkaddr_t ca,  lsblk_num_t fromblk)
{
    lsblk_num_t rs = _first_lsblk_with_canton(ca.v, fromblk);
    if (rs.n >= 0) return rs;
    // try with canton num
    xblkaddr_t fc = canton_for_lsblk(fromblk);
    for (int i=0; i<numTopology(); i++) {
        lsblk_num_t n;
        n.n = i;
        const topo_lsblk_t *t = Topology(n);
        if (t->canton_addr != ca.v) continue;
        if ((canton_for_lsblk(_lsblk(t->left1)).v != fc.v)
            && (canton_for_lsblk(_lsblk(t->left2)).v  != fc.v)
            && (canton_for_lsblk(_lsblk(t->right1)).v != fc.v)
            && (canton_for_lsblk(_lsblk(t->right2)).v != fc.v)) continue;
        return n;
    }
    lsblk_num_t n = {-1};
    return n;
    
}

lsblk_num_t any_lsblk_with_canton(xblkaddr_t ca)
{
    if (0xFF == ca.v) {
        lsblk_num_t n = {-1};
        return n;
    }
    for (int i=0; i<numTopology(); i++) {
        lsblk_num_t n;
        n.n = i;
        const topo_lsblk_t *t = Topology(n);
        if (t->canton_addr != ca.v) continue;
        return n;
    }
    lsblk_num_t n = {-1};
    return n;
}

xblkaddr_t next_block_addr(xblkaddr_t blkaddr, uint8_t left)
{
    // slow should not be used ?
    lsblk_num_t f = any_lsblk_with_canton(blkaddr);
    lsblk_num_t n = next_lsblk(f, left, NULL);
    if (n.n<0) {
    	xblkaddr_t r = { .v = 0xFF };
    	return r;
    }
    return canton_for_lsblk(n);
}
// --------------------------------------------------------------------------------------


void topology_get_cantons_for_turnout(xtrnaddr_t turnout, xblkaddr_t *head, xblkaddr_t *straight, xblkaddr_t *turn)
{
	head->v = 0xFF;
	straight->v = 0xFF;
	turn->v = 0xFF;

	int n=0;
	for (int i=0; i<topology_num_sblkd(); i++) {
		const topo_lsblk_t *t = topology_get_sblkd(i);
		uint8_t fc = t->canton_addr;
		if (t->ltn == turnout.v) {
			n++;
			if (t->left2 == -1) straight->v = fc;
			else if (t->left1 == -1) turn->v = fc;
			else head->v = fc;
		}
		if (t->rtn == turnout.v) {
			n++;
			if (t->right2 == -1) straight->v = fc;
			else if (t->right1 == -1) turn->v = fc;
			else head->v = fc;
		}
		if (n==3) return;
	}
}

