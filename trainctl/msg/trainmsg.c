/*
 * trainmsg.c
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */

#include "trainctl_config.h"
#include "trainmsg.h"
#include "../misc.h"
#include "../utils/itm_debug.h"
//#include "../oam/oam.h"
#include "../oam/boards.h"

#define _UNUSED_ __attribute__((unused))



#ifdef BOARD_HAS_TURNOUTS
LFMQUEUE_DEF_C(to_turnout, msg_64_t, 	12, 0)
LFMQUEUE_DEF_C(from_turnout, msg_64_t, 	1, 0)
#endif

#ifdef BOARD_HAS_CANTON
LFMQUEUE_DEF_C(to_canton, msg_64_t, 	16, 0)
LFMQUEUE_DEF_C(from_canton, msg_64_t, 	8, 0)
#endif

#ifdef BOARD_HAS_CTRL
LFMQUEUE_DEF_C(to_spdctl, msg_64_t, 	16, 0)
LFMQUEUE_DEF_C(from_spdctl, msg_64_t, 	16, 0)

LFMQUEUE_DEF_C(to_ctrl, msg_64_t,         16, 0)
LFMQUEUE_DEF_C(from_ctrl, msg_64_t,     32, 0)
#endif

#ifdef BOARD_HAS_USB
LFMQUEUE_DEF_C(to_usb, msg_64_t, 			8, 1)
LFMQUEUE_DEF_C(from_usb, msg_64_t,			16, 1)
#endif

#ifdef BOARD_HAS_IHM
LFMQUEUE_DEF_C(to_ui, msg_64_t, 		64, 1)
LFMQUEUE_DEF_C(from_ui, msg_64_t, 		4,  0)
#endif

#ifdef BOARD_HAS_UI_CTC
LFMQUEUE_DEF_C(to_ui_track, msg_64_t,   8, 1)
LFMQUEUE_DEF_C(from_ui_track, msg_64_t, 2, 0)
#endif


#ifdef BOARD_HAS_INA3221
LFMQUEUE_DEF_C(to_ina3221, msg_64_t, 	5, 0)
LFMQUEUE_DEF_C(from_ina3221, msg_64_t, 	64,  1)
#endif

#ifdef BOARD_HAS_LED
LFMQUEUE_DEF_C(to_led,   msg_64_t, 		8, 0)
LFMQUEUE_DEF_C(from_led, msg_64_t, 		1, 0)
#endif

#ifdef BOARD_HAS_CAN
LFMQUEUE_DEF_C(to_canbus_loc, msg_64_t, 3, 0)
LFMQUEUE_DEF_C(to_canbus, msg_64_t, 8, 0)
LFMQUEUE_DEF_C(from_canbus, msg_64_t, 16, 0)
#endif

#ifdef BOARD_HAS_OSCILLO
LFMQUEUE_DEF_C(from_oscillo, msg_64_t, 2, 1)
#endif

#ifdef BOARD_HAS_TRKPLN
LFMQUEUE_DEF_C(to_planner, msg_64_t, 	8, 0)
LFMQUEUE_DEF_C(from_planner, msg_64_t, 	8,  1)
#endif

#ifdef BOARD_HAS_SERVOS
LFMQUEUE_DEF_C(to_servo, msg_64_t,		3, 0)
LFMQUEUE_DEF_C(from_servo, msg_64_t,	3, 0)
#endif

LFMQUEUE_DEF_C(to_oam, msg_64_t, 8, 0)
LFMQUEUE_DEF_C(from_oam, msg_64_t, 12, 0)

LFMQUEUE_DEF_C(from_nowhere, msg_64_t,     4, 0)


typedef struct {
	mqf_t *from;
	mqf_t *to;
    uint8_t isforwd:1;		// mostly used for local broadcast
    uint8_t allow_loop:1;
} qdef_t;

typedef struct {
    uint8_t mask; uint8_t value; uint8_t destq;
} qroute_t;

static const qdef_t qdefs[] = {
    // forward q must be first
#ifdef BOARD_HAS_CAN
    {&from_canbus, &to_canbus,  	1, 0},
    {NULL,   &to_canbus_loc,    	0, 0},
#endif
#ifdef BOARD_HAS_USB
    {&from_usb, &to_usb,        	1, 1},
#endif
    
#ifdef BOARD_HAS_TURNOUTS
    {&from_turnout, &to_turnout,    0, 0},
#endif
#ifdef BOARD_HAS_CANTON
    {&from_canton, &to_canton,      0, 0},
#endif
#ifdef BOARD_HAS_INA3221
    {&from_ina3221, &to_ina3221,    0, 0},
#endif
#ifdef BOARD_HAS_LED
    {&from_led, &to_led,            0, 0},
#endif
#ifdef BOARD_HAS_CTRL
    {&from_spdctl, &to_spdctl,      0, 0},
    {&from_ctrl, &to_ctrl,          0, 0},
#endif
#ifdef BOARD_HAS_IHM
    {&from_ui,  &to_ui,             0, 0},
#endif
#ifdef BOARD_HAS_UI_CTC
    {&from_ui_track, &to_ui_track,  0, 0},
#endif
#ifdef BOARD_HAS_OSCILLO
    {&from_oscillo, NULL,           0, 0},
#endif
#ifdef BOARD_HAS_TRKPLN
	{&from_planner, &to_planner,	0, 0},
#endif
    
    {&from_oam, &to_oam,            0, 0},
    {&from_nowhere, NULL,           0, 0},
    {NULL, NULL,0,0}
};







static int  _local_disptach(msg_64_t *m, mqf_t *dont_send_to, uint8_t allow_loop)
{
    if (m->to == 0x66) {
        itm_debug1(DBG_MSG, "route66", m->cmd);
    }
    mqf_t *dest = NULL;
    if (allow_loop) {
    	dont_send_to = NULL;
    }
    int cont = 0;
    if (MA0_ADDR_IS_BOARD_ADDR(m->to)) {
        int dbrd = MA0_BOARD(m->to);
        if (dbrd != oam_localBoardNum()) return 0;
        if ((0)) {
#ifdef BOARD_HAS_CANTON
        } else if (MA0_IS_CANTON(m->to)) {
            dest = &to_canton;
#endif
#ifdef BOARD_HAS_TURNOUTS
        } else if (MA0_IS_TURNOUT(m->to)) {
            dest = &to_turnout;
#endif
#ifdef BOARD_HAS_RELAY
#error relay is not currently implemented
        } else if (MA0_IS_RELAY(m->to)) {
            dest = &to_relay;
#endif
#ifdef BOARD_HAS_SERVOS
        } else if (MA0_IS_SERVO(m->to)) {
            dest = &to_servo;
#endif
#ifdef BOARD_HAS_INA3221
        } else if (MA0_IS_INA(m->to)) {
            dest = &to_ina3221;
#endif
#ifdef BOARD_HAS_LED
        } else if (MA0_IS_LED(m->to)) {
            dest = &to_led;
#endif
        } else if (MA0_IS_OAM(m->to)) {
            dest = &to_oam;
        }
        
        
    } else if (MA1_ADDR_IS_TRAIN_ADDR(m->to)) {
#ifdef BOARD_HAS_CTRL
        if (MA1_IS_CTRL(m->to)) dest =     &to_ctrl;
        else if (MA1_SPDCTL(m->to)) dest = &to_spdctl;
#else
        return 0;
#endif
        
        
    } else if (MA2_IS_LOCAL_ADDR(m->to)) {
#ifdef BOARD_HAS_IHM
        if (MA2_UI_LOCAL == m->to) dest = &to_ui;
#endif
        if (MA2_OAM_LOCAL == m->to) dest = &to_oam;
#ifdef BOARD_HAS_USB
        if (MA2_USB_LOCAL== m->to) dest = &to_usb;
#endif


    } else if (MA3_IS_GLOB_ADDR(m->to)) {
    	if (MA3_UI_GEN == m->to) {
#ifdef BOARD_HAS_UI_GEN
    		cont = 1; dest = &to_ui;
#else
    		return 0;
#endif
    	}
    	if (MA3_UI_CTC == m->to) {
#ifdef BOARD_HAS_UI_CTC
    		cont = 1; dest = &to_ui_track;
#else
        	return 0;
#endif
    	}
    	if (MA3_PLANNER == m->to) {
#ifdef BOARD_HAS_TRKPLN
    		dest = &to_planner;
#else
    		return 0;
#endif
    	}
    	if (MA3_SLV_OAM == m->to) {
    		if (oam_isMaster()) {
    			return 0; // must be sent to CAN
    		} else {
    			// slave, route to OAM
    			dest = &to_oam;
    		}
    	}
    }
    if (!dest) {
        // message should be locally routable but is not
        itm_debug1(DBG_ERR|DBG_MSG, "no local", m->to);
        return -1;
    }
    if (dest == dont_send_to) {
        itm_debug1(DBG_ERR|DBG_MSG, "loop local", m->to);
        return -1;
    }
    mqf_write(dest, m);
    if (cont) {
        // UI shall be sent on canbus as there can by several UI boards
        return 0;
    }
    return 1;
}

static void dispatch_m64(msg_64_t *m, int f)
{
	/*
    if (m->cmd == CMD_PARAM_LUSER_GET) {
        itm_debug1(DBG_MSG, "brk here",0);
    }
    if (m->cmd == CMD_PARAM_LUSER_VAL) {
        itm_debug1(DBG_MSG, "brk here",0);
    }
    */
    if (m->to == MA3_BROADCAST) {
        for (int i=0;; i++) {
            const qdef_t *qd = &qdefs[i];
            if (!qd->from && !qd->to) break;
            if (i == f) {
                continue;
            }
            mqf_t *q = qd->to;
            if (!q) continue;
            mqf_write(q, m);
        }
        return;
    }
    if (m->to == MA2_LOCAL_BCAST) {
        for (int i=0;; i++) {
            const qdef_t *qd = &qdefs[i];
            if (!qd->from && !qd->to) break;
            if (i == f) {
                continue;
            }
            if (qd->isforwd) continue;
            mqf_t *q = qd->to;
            if (!q) continue;
            mqf_write(q, m);
        }
        return;
    }
    int ok = _local_disptach(m, qdefs[f].to,  qdefs[f].allow_loop);
    if (ok<0) {
        itm_debug1(DBG_ERR|DBG_MSG, "cant route local", m->to);
        return;
    }
    if (ok) return;
    
    // no forward for MA2 local
    if (MA2_IS_LOCAL_ADDR(m->to)) {
        itm_debug1(DBG_ERR|DBG_MSG, "cant ma2 route", m->to);
        return;
    }
    if (oam_localBoardNum()<0) {
    	itm_debug2(DBG_OAM, "skip no brd", m->to, m->cmd);
    	return;
    }
    ok = 0;
#ifdef BOARD_HAS_USB
    if ((MA3_UI_GEN == m->to) || (MA3_UI_CTC == m->to)) {
        if (qdefs[f].from != &from_usb) {
            mqf_write(&to_usb, m);
            ok = 1;
        }
        if ((1)) return; // XXX disable return so also send on CAN
    }
#endif
#ifdef BOARD_HAS_CAN
    if ((1) && (qdefs[f].from != &from_canbus)) { // XXX put 0 to disable CAN forwarding until ok
        mqf_write(&to_canbus, m);
        ok =  1;
    }
#endif
    if (!ok) {
        itm_debug2(DBG_ERR|DBG_MSG, "cant route", m->to, m->cmd);
    }
}


static void dump_qusage(int i, int d, mqf_t *q)
{
	itm_debug3(DBG_ERR, "qu", d*100+i, q->maxuse, mqf_len(q));
	q->maxuse = 0;
}

#ifndef TRAIN_SIMU
static_assert(sizeof(msg_64_t) == 8);
#else
typedef char compile_assert[(sizeof(msg_64_t) == 8) ? 1 : -1];
#endif

/*
 * msgsrv_tick() : the heart of the system
 * simply polls a all queue to msgsrv, and dispatch them
 * (all routing and smart things (if any) are in dispatch_m64()
 */
void msgsrv_tick(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	for (int i=0; ; i++) {
        const qdef_t *qd = &qdefs[i];
        if (!qd->from && !qd->to) break;
        mqf_t *q = qd->from;
        if (!q) continue;

        for (;;) {
            msg_64_t m;
            int rc = mqf_read(q, &m);
            if (rc) break;
            
            dispatch_m64(&m, i);
        }

    }

	// optional dump for debug
	if ((0)) {
		static uint32_t last=0;
		if (tick>=last+10000) {
			for (int i=0; ; i++) {
                const qdef_t *qd = &qdefs[i];
                if (!qd->from && !qd->to) break;
				dump_qusage(i, 0, qd->from);
				dump_qusage(i, 1, qd->to);
			}
			last = tick;
		}
	}
}


void dump_msg(mqf_t *mq, int n)
{
	int i = ( n + mq->tail ) % mq->num;
	msg_64_t *msg = (msg_64_t *) &(mq->msgbuf[i*mq->msgsiz]);
	itm_debug3(DBG_ERR, "q:icf", i, msg->cmd, msg->from);
}

