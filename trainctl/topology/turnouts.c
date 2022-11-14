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

static volatile uint32_t turnoutvals = 0; // bit field

uint8_t topology_or_occupency_changed = 0;

int topology_set_turnout(xtrnaddr_t tno, enum topo_turnout_state v, int numtrain)
{
    xtrnaddr_t turnout = tno;
	if (turnout.v == 0xFF) return -1;
	if (turnout.v > 31) {
		return -1; //XXX turnoutvals is uint32_t, fix this
	}
    int d = turnout.isdoor;
    turnout.isdoor = 0;
    if (turnout.v >= MAX_TOTAL_TURNOUTS) return -1;

	if (numtrain>=0) {
		int rc = occupency_turnout_reserve(tno, numtrain);
		if (rc) {
			return -1;
		}
	}
    if (!d) {
        if (v) {
            __sync_fetch_and_or(&turnoutvals, (1ULL<<turnout.v));
        } else {
            __sync_fetch_and_and(&turnoutvals, ~(1ULL<<turnout.v));
        }
        topology_or_occupency_changed = 1;
    } else {
        abort();
    }
	itm_debug2(DBG_TURNOUT, "tt", turnout.v, topology_get_turnout(tno));
	return 0;
}



enum topo_turnout_state topology_get_turnout(xtrnaddr_t turnout)
{
    if (turnout.v == 0xFF) return 0;
	if (turnout.v > 31) {
		return topo_tn_undetermined; //XXX turnoutvals is uint32_t, fix this
	}

    int d = turnout.isdoor;
    turnout.isdoor = 0;

	if (turnout.v >= MAX_TOTAL_TURNOUTS) return 0;
	if (turnout.v == 0xFF) return 0;

    if (!d) {
        uint64_t b = turnoutvals;
        return (b & (1ULL<<turnout.v)) ? topo_tn_turn : topo_tn_straight;
    } else {
        // TODO
        void alloc_turnouts(void);
        alloc_turnouts(); //XX
        abort();
    }
}

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
static volatile uint8_t  lockedby[MAX_TOTAL_TURNOUTS]; // XXX TODO this should be total number of turnouts

void occupency_clear_turnouts(void)
{
    memset((void *)lockedby, 0xFF, sizeof(lockedby));

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
    int d = turnout.isdoor;
    turnout.isdoor = 0;
	if (turnout.v >= MAX_TOTAL_TURNOUTS) return -1;


	itm_debug3(DBG_CTRL, "res.to", turnout.v, numtrain, lockedby[turnout.v]);
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
	return 0;
}


void occupency_turnout_release(xtrnaddr_t turnout, _UNUSED_ int8_t train)
{
    int l = lockedby[turnout.v];
	lockedby[turnout.v] = 0xFF;
    if (l != 0xFF) {
        _notify_chg_owner(turnout, -1);
    }
}

void occupency_turnout_release_for_train_canton(int train, xblkaddr_t canton)
{
	if (train<0) FatalError("OccTrn", "bad train num", Error_OccTrn);
	for (int tn = 0; tn<MAX_TOTAL_TURNOUTS; tn++) {
		if (lockedby[tn] != train) continue;
		xblkaddr_t ca1, ca2, ca3;
		xtrnaddr_t xtn = {.v = tn};
		topology_get_cantons_for_turnout(xtn, &ca1, &ca2, &ca3);
		if ((ca1.v == canton.v) || (ca2.v == canton.v) || (ca3.v == canton.v)) {
			occupency_turnout_release(xtn, train);
		}
	}
}

