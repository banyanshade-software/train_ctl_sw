/*
 * trainmsg.c
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */


#include "trainmsg.h"
#include "../misc.h"
#include "../utils/itm_debug.h"

#define _UNUSED_ __attribute__((unused))

uint8_t localBoardNum = BOARD_NUMBER; // TODO move to config


// LFMQUEUE_DEF_C(_name, _type,_num, _sil)


LFMQUEUE_DEF_C(to_turnout, msg_64_t, 	8, 0)
LFMQUEUE_DEF_C(from_turnout, msg_64_t, 	1, 0)


LFMQUEUE_DEF_C(to_canton, msg_64_t, 	16, 0)
LFMQUEUE_DEF_C(from_canton, msg_64_t, 	8, 0)


LFMQUEUE_DEF_C(to_spdctl, msg_64_t, 	16, 0)
LFMQUEUE_DEF_C(from_spdctl, msg_64_t, 	16, 0)

//LFMQUEUE_DEF_C(to_forward, msg_64_t, 	8, 1) // XXX should not have silent drop
//LFMQUEUE_DEF_C(from_forward, msg_64_t, 	8, 0)

LFMQUEUE_DEF_C(to_forward_usb, msg_64_t,	8, 1)
LFMQUEUE_DEF_C(from_forward_usb, msg_64_t,  16, 0)


LFMQUEUE_DEF_C(to_ctrl, msg_64_t, 		16, 0)
LFMQUEUE_DEF_C(from_ctrl, msg_64_t, 	32, 0)


LFMQUEUE_DEF_C(to_ui, msg_64_t, 		64, 1)
LFMQUEUE_DEF_C(from_ui, msg_64_t, 		4,  0)

LFMQUEUE_DEF_C(to_ui_track, msg_64_t,   8, 1)
LFMQUEUE_DEF_C(from_ui_track, msg_64_t, 2, 0)

LFMQUEUE_DEF_C(from_nowhere, msg_64_t, 	4, 0)


LFMQUEUE_DEF_C(to_ina3221, msg_64_t, 	4, 0)
LFMQUEUE_DEF_C(from_ina3221, msg_64_t, 	64,  1)


LFMQUEUE_DEF_C(to_led,   msg_64_t, 		8, 0)
LFMQUEUE_DEF_C(from_led, msg_64_t, 		1, 0)


LFMQUEUE_DEF_C(to_canbus, msg_64_t, 8, 0)
LFMQUEUE_DEF_C(from_canbus, msg_64_t, 8, 0)


typedef struct {
	mqf_t *to;
	mqf_t *from;
} qdef_t;

typedef struct {
    uint8_t mask; uint8_t value; uint8_t destq;
} qroute_t;

#ifdef TRN_BOARD_MAIN
#define NQDEF 11
static const qdef_t qdef[NQDEF] = {
		/* 0*/ { &to_turnout, &from_turnout },
		/* 1*/ { &to_canton,  &from_canton},
		/* 2*/ { &to_spdctl,  &from_spdctl},
		/* 3*/ { &to_canbus, &from_canbus},
        /* 4*/ { &to_forward_usb, &from_forward_usb},
        /* 5*/ { &to_ctrl, &from_ctrl},
        /* 6*/ { &to_ui, &from_ui},
		/* 7*/ { &to_ina3221, &from_ina3221},
        /* 8*/ { &to_ui_track, &from_ui_track},
        /* 9*/ { &to_led, &from_led},
		/*10*/ { NULL, &from_nowhere}
};

#define NROUTES 11
static const qroute_t routes[NROUTES] = {
		{MA_ADDR_MASK_2|MA_ADDR_MASK_BOARD,		MA_ADDR_2_TURNOUT|0,	0},
		{MA_ADDR_MASK_2|MA_ADDR_MASK_BOARD,		MA_ADDR_2_CANTON|0,		1},

		{MA_ADDR_MASK_2,						MA_ADDR_2_TURNOUT,		3},
		{MA_ADDR_MASK_2,						MA_ADDR_2_CANTON,		3},
        {MA_ADDR_MASK_3|0x1F,                   MA_ADDR_3_UI|1,         6},
#ifdef TRAIN_SIMU
        {MA_ADDR_MASK_3|0x1F,                   MA_ADDR_3_UI|2,         8},
#else
        {MA_ADDR_MASK_3|0x1F,                   MA_ADDR_3_UI|2,         4},
#endif
        {MA_ADDR_MASK_3,                        MA_ADDR_3_UI,           4},
		{MA_ADDR_MASK_5,						MA_ADDR_5_TRSC,			2},
		{MA_ADDR_MASK_5,						MA_ADDR_5_CTRL,			5},
		{MA_ADDR_MASK_8,						MA_ADDR_5_LED|0,		9},
		{MA_ADDR_MASK_5,						MA_ADDR_5_LED,			3}

};

#endif // TRN_BOARD_MAIN

#ifdef TRN_BOARD_DISPATCHER
#define NQDEF 6
static const qdef_t qdef[NQDEF] = {
        /* 0*/ { &to_turnout, &from_turnout },
        /* 1*/ { &to_canton,  &from_canton},
        /* 2*/ { &to_canbus, &from_canbus},
        /* 3*/ { &to_ina3221, &from_ina3221},
        /* 4*/ { &to_led, &from_led},
        /* 5*/ { NULL, &from_nowhere}
};
#define NROUTES 6
static const qroute_t routes[NROUTES] = {
        {MA_ADDR_MASK_2|MA_ADDR_MASK_BOARD,        MA_ADDR_2_TURNOUT|1,    0},
        {MA_ADDR_MASK_2|MA_ADDR_MASK_BOARD,        MA_ADDR_2_CANTON|1,     1},

        {MA_ADDR_MASK_2,                        MA_ADDR_2_TURNOUT,         2},
        {MA_ADDR_MASK_2,                        MA_ADDR_2_CANTON,          2},
        {MA_ADDR_MASK_8,                        MA_ADDR_5_LED|0,           4},
        {MA_ADDR_MASK_5,                        MA_ADDR_5_LED,             2}

};
#endif


#ifdef TRN_BOARD_UI
#define NQDEF 4
static const qdef_t qdef[NQDEF] = {
		/* 0*/ { &to_canbus, &from_canbus},
        /* 1*/ { &to_ui, &from_ui},
        /* 2*/ { &to_forward_usb, &from_forward_usb},
		/* 3*/ { NULL, &from_nowhere}
};

#define NROUTES 3
static const qroute_t routes[NROUTES] = {
        {MA_ADDR_MASK_3|0x1F,                   MA_ADDR_3_UI|1,         1},
#ifdef TRAIN_SIMU
        {MA_ADDR_MASK_3|0x1F,                   MA_ADDR_3_UI|2,         1},
#else
        {MA_ADDR_MASK_3|0x1F,                   MA_ADDR_3_UI|2,         2},
#endif
        {0,                        				0,           0}, // default to canbu

};
#endif // TRN_BOARD_UI



static void msg_error(_UNUSED_ const char *msg)
{

}


static void dispatch_m64(msg_64_t *m, int f)
{
	if ((1)) {
		if (m->cmd == 0x52) {
			itm_debug1(DBG_USB, "disp A2", m->cmd);
		}
	}
    if (m->to == MA_BROADCAST) {
        for (int i=0; i<NQDEF; i++) {
            if (i == f) {
                continue;
            }
            mqf_t *q = qdef[i].to;
            if (!q) continue;
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

void msgsrv_tick(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
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

