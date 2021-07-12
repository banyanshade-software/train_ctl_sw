/*
 * trainmsg.c
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */


#include "trainmsg.h"
#include "../utils/itm_debug.h"

uint8_t localBoardNum = 0; // TODO move to config



LFMQUEUE_DEF_C(to_turnout, msg_64_t, 	8, 0)
LFMQUEUE_DEF_C(from_turnout, msg_64_t, 	1, 0)


LFMQUEUE_DEF_C(to_canton, msg_64_t, 	8, 0)
LFMQUEUE_DEF_C(from_canton, msg_64_t, 	8, 0)


LFMQUEUE_DEF_C(to_spdctl, msg_64_t, 	8, 0)
LFMQUEUE_DEF_C(from_spdctl, msg_64_t, 	16, 0)

LFMQUEUE_DEF_C(to_forward, msg_64_t, 	8, 1) // XXX should not have silent drop
LFMQUEUE_DEF_C(from_forward, msg_64_t, 	8, 0)

LFMQUEUE_DEF_C(to_forward_usb, msg_64_t,	8, 1)
LFMQUEUE_DEF_C(from_forward_usb, msg_64_t,  8, 0)


LFMQUEUE_DEF_C(to_ctrl, msg_64_t, 		12, 0)
LFMQUEUE_DEF_C(from_ctrl, msg_64_t, 	12, 0)


LFMQUEUE_DEF_C(to_ui, msg_64_t, 		64, 0)
LFMQUEUE_DEF_C(from_ui, msg_64_t, 		4, 1)

typedef struct {
	mqf_t *to;
	mqf_t *from;
} qdef_t;

#define NQDEF 7
static const qdef_t qdef[NQDEF] = {
		/* 0*/ { &to_turnout, &from_turnout },
		/* 1*/ { &to_canton,  &from_canton},
		/* 2*/ { &to_spdctl,  &from_spdctl},
        /* 3*/ { &to_forward, &from_forward},
        /* 4*/ { &to_forward_usb, &from_forward_usb},
        /* 5*/ { &to_ctrl, &from_ctrl},
		/* 6*/ { &to_ui, &from_ui}
};

typedef struct {
	uint8_t mask; uint8_t value; uint8_t destq;
} qroute_t;

#define NROUTES 8
static const qroute_t routes[NROUTES] = {
		{MA_ADDR_MASK_2|MA_ADDR_MASK_BOARD,		MA_ADDR_2_TURNOUT|0,	0},
		{MA_ADDR_MASK_2|MA_ADDR_MASK_BOARD,		MA_ADDR_2_CANTON|0,		1},
		{MA_ADDR_MASK_2,						MA_ADDR_2_TURNOUT,		3},
		{MA_ADDR_MASK_2,						MA_ADDR_2_CANTON,		3},
        {MA_ADDR_MASK_3|0x1F,                   MA_ADDR_3_UI|1,         6},
        {MA_ADDR_MASK_3,                        MA_ADDR_3_UI,           4},
		{MA_ADDR_MASK_5,						MA_ADDR_5_TRSC,			2},
		{MA_ADDR_MASK_5,						MA_ADDR_5_CTRL,			5}

};

static void msg_error(const char *msg)
{

}


static void dispatch_m64(msg_64_t *m, int f)
{
    if (m->to == MA_BROADCAST) {
        for (int i=0; i<NQDEF; i++) {
            if (i == f) {
                continue;
            }
            mqf_t *q = qdef[i].to;
            mqf_write(q, m);
        }
        return;
    }
	for (int i=0; i<NROUTES; i++) {
		if ((m->to & routes[i].mask) == routes[i].value) {
			if (f==routes[i].destq) {
				// loop
				itm_debug1(DBG_ERR|DBG_MSG, "loop", f);
				return;
			}
			mqf_t *q = qdef[routes[i].destq].to;
			mqf_write(q, m);
			return;
		}
	}
	itm_debug1(DBG_ERR|DBG_MSG, "no route", m->to);
	msg_error("no route");
}


static void dump_qusage(int i, int d, mqf_t *q)
{
	itm_debug3(DBG_ERR, "qu", d*100+i, q->maxuse, mqf_len(q));
	q->maxuse = 0;
}

void msgsrv_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
    static int first = 1;
    if (first) {
        if (sizeof(msg_64_t) != 8) {
#ifdef TRAIN_SIMU
            void abort(void);
        	abort();
#else
        	itm_debug1(DBG_ERR|DBG_MSG, "bad size", sizeof(msg_64_t));
        	for (;;);
#endif
        }
    }
	for (int i=0; i<NQDEF; i++) {
		mqf_t *q = qdef[i].from;

		itm_debug2(DBG_MSG, "mlen1",i, mqf_len(q));
		itm_debug3(DBG_MSG, "mth1 ", i, q->head, q->tail);
		for (;;) {
				msg_64_t m;
				int rc = mqf_read(q, &m);
				if (rc) break;
				if (i==5) {
					itm_debug1(DBG_MSG, "from ctrl", m.cmd);
				}
				dispatch_m64(&m, i);
			}
		itm_debug2(DBG_MSG, "mlen2",i, mqf_len(q));
		itm_debug3(DBG_MSG, "mth2 ", i, q->head, q->tail);
	}
	if ((0)) {
		static uint32_t last=0;
		if (tick>=last+10000) {
			for (int i=0; i<NQDEF; i++) {
				dump_qusage(i, 0, qdef[i].from);
				dump_qusage(i, 1, qdef[i].to);
			}
			last = tick;
		}
	}
}


void dump_msg(mqf_t *mq, int n)
{
	int i = ( n + mq->tail ) % mq->num;
	msg_64_t *msg = (msg_64_t *) &(mq->msgbuf[i*mq->msgsiz]);
	itm_debug3(DBG_ERR, "q", i, msg->cmd, msg->from);
}

