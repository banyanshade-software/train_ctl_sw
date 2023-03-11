/*
 * turnouts.c
 *
 *  Created on: 14 nov. 2022
 *      Author: danielbraun
 */



#include <stddef.h>
#include <memory.h>


#ifndef TRACKPLAN_TESTPGM
#include "../railconfig.h"
#else
#include <stdio.h>
#include <stdlib.h>

//#define NUM_TURNOUTS 16
#define DBG_TURNOUTS 1
//#define itm_debug3(_fl, _msg, _a, _b, _c) do {printf(_msg  "%d %d %d", _a, _b, _c);} while(0)
#endif


#include "topology.h"
#include "topologyP.h"
#include "occupency.h"
#include "../utils/bitarray.h"
#include "../utils/dynamic_perfect_hash.h"


// data structures to hold turnouts / doors position and reservation

typedef struct {
	uint8_t lockby:4; // trainnum locking the turnout
	enum topo_turnout_state st:4;
} turnout_st_t;

#ifndef __clang__
static_assert(sizeof(turnout_st_t)==1);
#endif

static turnout_st_t turnout_st[MAX_TOTAL_TURNOUTS] = {0};

// index in turnout_st for a given turnout
// currently assume low number of board
static int tn_index(xtrnaddr_t tn)
{
	// TODO
	int v = tn.board * 7 + tn.turnout;
	if (tn.isdoor) {
		v = MAX_TOTAL_TURNOUTS-v;
	}
	return v;
}

static xtrnaddr_t reverse_tn_index(int tridx)
{
    xtrnaddr_t tn;
    for (int i=0; i<0xFF; i++) {
        tn.v = i;
        int t = tn_index(tn);
        if (t==tridx) return tn;
    }
    tn.v = 0xFF;
    return tn;
}

//static volatile uint8_t  lockedby[MAX_TOTAL_TURNOUTS]; // XXX TODO this should be total number of turnouts
//static volatile uint32_t turnoutvals = 0; // bit field

uint8_t _topology_or_occupency_changed = 0;


int topology_set_turnout(xtrnaddr_t turnout, enum topo_turnout_state v, int numtrain)
{
	if (turnout.v == 0xFF) return -1;

    int tnidx = tn_index(turnout);

    if (tnidx >= MAX_TOTAL_TURNOUTS) return -1;

	if (numtrain>=0) {
		int rc = occupency_turnout_reserve(turnout, numtrain);
		if (rc) {
			return -1;
		}
	}
	turnout_st[tnidx].st = v;
    topology_updated(numtrain);
	
    
	itm_debug2(DBG_TURNOUT, "tt", turnout.v, topology_get_turnout(turnout));
	return 0;
}



enum topo_turnout_state topology_get_turnout(xtrnaddr_t turnout)
{
    if (turnout.v == 0xFF) return 0;

    int tnidx = tn_index(turnout);
    //int d = turnout.isdoor;

	if (tnidx >= MAX_TOTAL_TURNOUTS) return 0;

	return turnout_st[tnidx].st;
	/*
    if (!d) {
        uint64_t b = turnoutvals;
        return (b & (1ULL<<turnout.v)) ? topo_tn_turn : topo_tn_straight;
    } else {
        // TODO
        void alloc_turnouts(void);
        alloc_turnouts(); //XX
        abort();
    }
    */
}


int allturnouts(int(*f)(uint8_t tn, void *arg), void *val)
{
    DECL_BIT_ARRAY(tna, 256);
    BIT_ARRAY_CLEAR(tna);
    int n = topology_num_sblkd();
    for (int i=0; i<n; i++) {
        int rc;
        //lsblk_num_t b = {i};
        const topo_lsblk_t *t = topology_get_sblkd(i);
        if (t->ltn != 0xFF) {
            if (!BIT_ARRAY_VALUE(tna, t->ltn)) {
                BIT_ARRAY_SET(tna, t->ltn);
                rc = f(t->ltn,val);
                if (rc) return rc;
            }
        }
        if (t->rtn != 0xFF) {
            if (!BIT_ARRAY_VALUE(tna, t->rtn)) {
                BIT_ARRAY_SET(tna, t->rtn);
                rc = f(t->rtn,val);
                if (rc) return rc;
            }
        }
    }
    return 0;
}

static dph64_def_t dph;

static int tst(uint8_t tn, void *p)
{
    uint32_t *bitarray = (uint32_t *)p;
    uint8_t h = dph64_hash(&dph, tn, 64);
    //printf("turnout: %d, h=%d, used=%d\n", tn, h,  BIT_ARRAY_VALUE(bitarray, h));
    if (BIT_ARRAY_VALUE(bitarray, h)) return -1;
    BIT_ARRAY_SET(bitarray, h);
    return 0;
}
void alloc_turnouts(void)
{
    dph64_start(&dph);
    for (;;) {
        DECL_BIT_ARRAY(used, 64);
        BIT_ARRAY_CLEAR(used);
        int rc = allturnouts(tst, used);
        if (!rc) break;
        if (dph64_next(&dph)) {
            //printf("hash not found\n");
            FatalError("HASH", "hash not found", Error_Hash);
        }
    }
}


// --------------------------------------------------------------------------

//static void occupency_turnout_release_for_train_canton(int train, xblkaddr_t canton);

void occupency_clear_turnouts(void)
{
	for (int i=0; i<MAX_TOTAL_TURNOUTS; i++) {
		turnout_st[i].lockby = 0xF;
	}
    //memset((void *)lockedby, 0xFF, sizeof(lockedby));
}

static  void _notify_chg_owner(xtrnaddr_t turnout, uint8_t numtrain)
{
    msg_64_t m = {0};
    m.cmd = CMD_TN_RESER_NOTIF;
    m.v1 = turnout.v;
    m.v2 = numtrain;
    m.to = MA3_UI_CTC;
    m.from = MA1_CONTROL();
    mqf_write_from_ctrl(&m);
}


int occupency_turnout_reserve(xtrnaddr_t turnout, int8_t numtrain)
{
    //int d = turnout.isdoor;
    int tnidx = tn_index(turnout);

	if (tnidx >= MAX_TOTAL_TURNOUTS) return -1;


	itm_debug3(DBG_CTRL, "res.to", turnout.v, numtrain, turnout_st[tnidx].lockby);

	if (numtrain>=0) {
		uint8_t old = turnout_st[tnidx].lockby;
		if (old == numtrain) return 0;
		if (old != 0xF) {
            itm_debug3(DBG_ERR, "res.to.f", turnout.v, numtrain, turnout_st[tnidx].lockby);
            return -1;
		}
		turnout_st[tnidx].lockby = numtrain;
        _notify_chg_owner(turnout, numtrain);
	}
#if 0
if (numtrain>=0) {
        uint8_t expected = numtrain;
        int ok = 0;
        if (d) {
            abort();
        } else {
            ok = __atomic_compare_exchange_n(&lockedby[turnout.v], &expected, numtrain, 0 /*weak*/, __ATOMIC_ACQUIRE/*success memorder*/, __ATOMIC_ACQUIRE/*fail memorder*/ );
        }
        if (!ok) {
            expected = 0xFF;
            ok = __atomic_compare_exchange_n(&lockedby[turnout.v], &expected, numtrain, 0 /*weak*/, __ATOMIC_ACQUIRE/*success memorder*/, __ATOMIC_ACQUIRE/*fail memorder*/ );
            _notify_chg_owner(turnout, numtrain);
        }
		if (!ok) {
            itm_debug3(DBG_ERR, "res.to.f", turnout.v, numtrain, lockedby[turnout.v]);
			return -1;
		}
	}
#endif
	return 0;
}


void occupency_turnout_release(xtrnaddr_t turnout, _UNUSED_ int8_t train)
{
	//int d = turnout.isdoor;
	int tnidx = tn_index(turnout);

	if (tnidx >= MAX_TOTAL_TURNOUTS) return;
    int l = turnout_st[tnidx].lockby;
    turnout_st[tnidx].lockby = 0xF;
    if (l != 0xF) {
        _notify_chg_owner(turnout, -1);
    }
}

static void _rel(int tn, int train)
{
	if (tn==0xFF) return;
	xtrnaddr_t turnout;
	turnout.v = tn;
	int tnidx = tn_index(turnout);
	if (turnout_st[tnidx].lockby == 0xF) return;
	if (turnout_st[tnidx].lockby != train) {
		itm_debug3(DBG_ERR|DBG_CTRL, "badlock", tn, train, turnout_st[tnidx].lockby);
		return;
	}
	occupency_turnout_release(turnout, train);
}

void occupency_turnout_release_for_train_canton(int train, xblkaddr_t canton)
{
	// release reservation for any turnout connected to block

	if (train<0) FatalError("OccTrn", "bad train num", Error_OccTrn);

	  int n = topology_num_sblkd();
	    for (int i=0; i<n; i++) {
	        //lsblk_num_t b = {i};
	        const topo_lsblk_t *t = topology_get_sblkd(i);
	        if (t->canton_addr != canton.v) continue;
	        _rel(t->rtn, train);
	        _rel(t->ltn, train);
	    }
}

void occupency_turnout_release_for_train(int train)
{
    // release reservation for any turnout reserved by a given train
    if (train<0) FatalError("OccTrn", "bad train num", Error_OccTrn);

    for (int tnidx=0; tnidx<MAX_TOTAL_TURNOUTS; tnidx++) {
        if (turnout_st[tnidx].lockby == train) {
            int l = turnout_st[tnidx].lockby;
            turnout_st[tnidx].lockby = 0xF;
            if (l != 0xF) {
                xtrnaddr_t turnout = reverse_tn_index(tnidx);
                _notify_chg_owner(turnout, -1);
            }
        }
    }
}
/*	for (int tn = 0; tn<MAX_TOTAL_TURNOUTS; tn++) {
		if (lockedby[tn] != train) continue;
		xblkaddr_t ca1, ca2, ca3;
		xtrnaddr_t xtn = {.v = tn};
		topology_get_cantons_for_turnout(xtn, &ca1, &ca2, &ca3);
		if ((ca1.v == canton.v) || (ca2.v == canton.v) || (ca3.v == canton.v)) {
			occupency_turnout_release(xtn, train);
		}
	}
}*/

