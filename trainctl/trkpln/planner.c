/*
 * planner.c
 *
 *  Created on: 27 sept. 2022
 *      Author: danielbraun
 */


#include <stdint.h>
#include <memory.h>


#include "../misc.h"
#include "../msg/trainmsg.h"

#include "planner.h"
#include "../topology/topology.h"
#include "../topology/topologyP.h"
#include "../ctrl/ctrl.h"
#include "../ctrl/ctrlP.h"
#include "../ctrl/cautoP.h"
#include "../leds/led.h"

#ifndef BOARD_HAS_TRKPLN
#error BOARD_HAS_TRKPLN not defined, remove this file from build
#endif

// ----------------------------------------------------------------------------------

static void planner_reset_all(void);
static void handle_planner_msg(msg_64_t *m);
static void planner_tick(_UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt);
static void planner_plan_pending(int delay);

static void planner_enter_runmode(_UNUSED_ runmode_t m)
{
	planner_reset_all();
}


static const tasklet_def_t planner_tdef = {
		.init 				= planner_reset_all,
		.poll_divisor		= NULL,
		.emergency_stop 	= planner_reset_all,
		.enter_runmode		= planner_enter_runmode,
		.pre_tick_handler	= NULL,
		.default_msg_handler = handle_planner_msg,
		.default_tick_handler = planner_tick,
		.msg_handler_for	= NULL,
		.tick_handler_for 	= NULL,

		.recordmsg			= RECORD_MSG,

};
tasklet_t planner_tasklet = { .def = &planner_tdef, .init_done = 0, .queue=&to_planner};

typedef struct {
    uint8_t train;
    uint8_t delay;
    uint8_t target;
    uint8_t spd;
    uint32_t startTick;
} pending_t;

#define NUM_PENDING 4
static pending_t PlanPending[NUM_PENDING] = {0};
static int PendingIdx = 0;

static void reset_pending(void)
{

    PendingIdx = 0;
}
static void planner_reset_all(void)
{
	memset(PlanPending, 0, sizeof((PlanPending)));
	for (int i=0; i<NUM_PENDING; i++) {
		PlanPending[i].train = 0xFF;
	}
    reset_pending();
}
static void handle_planner_msg(msg_64_t *m)
{
	switch (m->cmd) {
	case CMD_PLANNER_CANCEL:
        reset_pending();
		break;
	case CMD_PLANNER_ADD:
		if (PendingIdx >= NUM_PENDING) break;
		PlanPending[PendingIdx].train = m->subc;
		PlanPending[PendingIdx].delay = m->va16;
		PlanPending[PendingIdx].target = m->vb8;
		PlanPending[PendingIdx].spd = m->vcu8;
		PendingIdx++;
		break;
	case CMD_PLANNER_COMMIT:
		planner_plan_pending(m->v1u);
        reset_pending();
		break;
	case CMD_PLANNER_RESET:
		planner_reset_all();
		break;
	default:
		break;
	}
}

static void planner_start_pending(int n, uint8_t Auto1ByteCode[]);

static void planner_tick(_UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	  for (int n = 0; n<NUM_PENDING; n++) {
	    	if (0xFF == PlanPending[n].train) continue;
	    	if (PlanPending[n].startTick && (tick >= PlanPending[n].startTick)) {
	    		planner_start_pending(n, ctrl_get_autocode(PlanPending[n].train));
	    		PlanPending[n].startTick = 0;
	    }
	}
}

static uint32_t adjmatrix[32] = {0};
static int adjdone = 0;

static inline void set_matrix(int i, int j)
{
    adjmatrix[i] |= (1UL<<j);
}
static inline int get_matrix(int i, int j)
{
    int r = (adjmatrix[i] & (1UL<<j)) ? 1  : 0;
    return r;
}
static void build_matrix(void)
{
    memset(adjmatrix, 0, sizeof(adjmatrix));
    int n = topology_num_sblkd();
    if (n>32) {
        n=32;
        //FatalError("TOBI", "too big network", Error_PlannerTooManyBlocks);
    }
    for (int i=0; i<n; i++) {
        const topo_lsblk_t *p = topology_get_sblkd(i);
        if (!p) continue;
        int n1 = p->right1;
        int n2 = p->right2;
        int n3 = p->left1;
        int n4 = p->left2;
        // test <32 also check != 0xFF
        if (n1 < 32) set_matrix(i, n1);
        if (n2 < 32) set_matrix(i, n2);
        if (n3 < 32) set_matrix(i, n3);
        if (n4 < 32) set_matrix(i, n4);
    }
}
static uint16_t distance[32];
static uint32_t visited; // bitfield


static void planner_plan_pending(int delay)
{
    if (!adjdone) build_matrix();
    for (int n = 0; n<NUM_PENDING; n++) {
    	if (0xFF == PlanPending[n].train) continue;
    	if (PlanPending[n].startTick) continue; // already commited
    	PlanPending[n].startTick = (PlanPending[n].delay + delay)*1000 + planner_tasklet.last_tick;
    }
}

static void planner_start_pending(int n, uint8_t Auto1ByteCode[])
{
    //XXX currently handle only 1st request
    int train = PlanPending[n].train;
	if (0xFF == train) return;
	PlanPending[n].train = 0xFF;

	int spd = PlanPending[n].spd;
    int target = PlanPending[n].target;
    int curpos = ctrl_get_train_curlsblk(train);
    if (curpos == -1) return;
    //XXX delay ignored
    // we keep
    for (int i=0;i<32;i++) distance[i] = 0xFFFE;
    distance[target] = 0;
    visited = 0;
    for (;;) {
        // select shortest distance node
        uint16_t shortest_distance = 0xFFFF;
        int  shortest_index = -1;
        for (int i=0; i<32; i++) {
            if ((distance[i] < shortest_distance) && (0 == (visited & (1UL<<i)))) {
                shortest_distance = distance[i];
                shortest_index = i;
            }
        }
        if (-1 == shortest_index) break; // done, all nodes visited
        
        lsblk_num_t s = {shortest_index};
        int l = distance[shortest_index] + get_lsblk_len_cm(s, NULL);
        for (int i=0; i<32; i++) {
            if (!get_matrix(shortest_index, i)) continue;
            if (distance[i] > l) {
                distance[i] = l;
            }
        }
        visited |= (1UL<<shortest_index);
    }
     // debug print
    int b = curpos;
#ifdef TRAIN_SIMU
    for (;;) {
        printf(" b%d ", b);
        if (b==target) break;
        int s=0xFFFFF;
        int si = -1;
        for (int i=0; i<32; i++) {
            if (!get_matrix(b, i)) continue;
            if (distance[i]<s) {
                s = distance[i];
                si = i;
            }
        }
        if (si<0) FatalError("NF", "path not found", Error_Other); // to be removed
        b = si;
    }
    printf("hop");
#endif
    // convert to cauto byte code
    int bytecodeIndex = 0;
    b = curpos;
    int prevb = -1;
    int dir = 0;
    _UNUSED_ int led = 0;
    for (;;) {
        //printf(" b%d ", b);
        if (b==curpos) {
            //Auto1ByteCode[bytecodeIndex++] = b;
        } else if (prevb >= 0) {
            int ndir = 0;
            const topo_lsblk_t *p = topology_get_sblkd(prevb);
            if (!p) FatalError("NF", "path not found", Error_Other);
            if ((p->left2 == b) || (p->left1 == b)) ndir = -1;
            if ((p->right2 == b) || (p->right1 == b)) ndir = 1;
            if (ndir != dir) {
                if (dir) {
                    Auto1ByteCode[bytecodeIndex++] = _AR_TRG_LIM; //_AR_TRG_END;
                    Auto1ByteCode[bytecodeIndex++] = _AR_WTRG_U1;
                    Auto1ByteCode[bytecodeIndex++] = _AR_SPD(0);
                    Auto1ByteCode[bytecodeIndex++] = _AR_WSTOP;
                    Auto1ByteCode[bytecodeIndex++] = _AR_TIMER(1);
                    Auto1ByteCode[bytecodeIndex++] = _AR_WTIMER;
                }
                Auto1ByteCode[bytecodeIndex++] = _AR_SPD(ndir*spd);
            }
            dir = ndir;
            Auto1ByteCode[bytecodeIndex++] = b;
        }
        /*
        if ((b==1) && (!led)) {
        	led = 1;
        	Auto1ByteCode[bytecodeIndex++] = _AR_LED;
        	Auto1ByteCode[bytecodeIndex++] = 0;
        	Auto1ByteCode[bytecodeIndex++] = LED_PRG_25p;

        	Auto1ByteCode[bytecodeIndex++] = _AR_LED;
        	Auto1ByteCode[bytecodeIndex++] = 1;
        	Auto1ByteCode[bytecodeIndex++] = LED_PRG_NEONON;
        }
        */
        if (b==target) {
            if (dir) {
                //Auto1ByteCode[bytecodeIndex++] = b;
                Auto1ByteCode[bytecodeIndex++] = _AR_TRG_LIM;
                Auto1ByteCode[bytecodeIndex++] = _AR_WTRG_U1;
                Auto1ByteCode[bytecodeIndex++] = _AR_SPD(0);
                //Auto1ByteCode[bytecodeIndex++] = _AR_WSTOP;
            }
            Auto1ByteCode[bytecodeIndex++] = _AR_END;
            break;
        }
        int s=0xFFFFF;
        int si = -1;
        for (int i=0; i<32; i++) {
            if (!get_matrix(b, i)) continue;
            if (distance[i]<s) {
                s = distance[i];
                si = i;
            }
        }
        if (si<0) FatalError("NF", "path not found", Error_Other); // to be removed
        prevb = b;
        b = si;
    }
    //printf("hop");
    msg_64_t m = {0};
    m.cmd = CMD_START_AUTO;
    m.v1u = 1;
    m.to = MA1_CTRL(train);
    m.from = MA3_PLANNER;
    mqf_write_from_planner(&m);
}

