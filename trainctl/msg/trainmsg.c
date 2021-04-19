/*
 * trainmsg.c
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */


#include "trainmsg.h"

uint8_t localBoardNum = 0; // TODO move to config



LFMQUEUE_DEF_C(to_turnout, msg_64_t, 8)
LFMQUEUE_DEF_C(from_turnout, msg_64_t, 1)


LFMQUEUE_DEF_C(to_canton, msg_64_t, 8)
LFMQUEUE_DEF_C(from_canton, msg_64_t, 8)


LFMQUEUE_DEF_C(to_spdctl, msg_64_t, 8)
LFMQUEUE_DEF_C(from_spdctl, msg_64_t, 8)

LFMQUEUE_DEF_C(to_forward, msg_64_t, 8)
LFMQUEUE_DEF_C(from_forward, msg_64_t, 8)

typedef struct {
	mqf_t *to;
	mqf_t *from;
} qdef_t;

#define NQDEF 4
static const qdef_t qdef[NQDEF] = {
		/* 0*/ { &to_turnout, &from_turnout },
		/* 1*/ { &to_canton,  &from_canton},
		/* 2*/ { &to_spdctl,  &from_spdctl},
		/* 3*/ { &to_forward, &from_forward}
};

typedef struct {
	uint8_t mask; uint8_t value; uint8_t destq;
} qroute_t;

#define NROUTES 5
static const qroute_t routes[NROUTES] = {
		{MA_ADDR_MASK_2|MA_ADDR_MASK_BOARD,		MA_ADDR_2_TURNOUT|0,	0},
		{MA_ADDR_MASK_2|MA_ADDR_MASK_BOARD,		MA_ADDR_2_CANTON|0,		1},
		{MA_ADDR_MASK_2,						MA_ADDR_2_TURNOUT,		3},
		{MA_ADDR_MASK_2,						MA_ADDR_2_CANTON,		3},
		{MA_ADDR_5_TRSC,						MA_ADDR_5_TRSC,			2},

};

static void msg_error(const char *msg)
{

}


static void dispatch_m64(msg_64_t *m, int f)
{
	for (int i=0; i<NROUTES; i++) {
		if ((m->to & routes[i].mask) == routes[i].value) {
			if (f==routes[i].destq) {
				// loop
				return;
			}
			mqf_t *q = qdef[routes[i].destq].to;
			mqf_write(q, m);
			return;
		}
	}
	msg_error("no route");
}




void msgsrv_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
	for (int i=0; i<NQDEF; i++) {
		mqf_t *q = qdef[i].from;
		for (;;) {
				msg_64_t m;
				int rc = mqf_read(q, &m);
				if (rc) break;
				dispatch_m64(&m, i);
			}
	}
}



