/*
 * trainmsg.c
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */


#include "trainmsg.h"




LFMQUEUE_DEF_C(to_turnout, msg_64_t, 8)
LFMQUEUE_DEF_C(from_turnout, msg_64_t, 1)


LFMQUEUE_DEF_C(to_canton, msg_64_t, 8)
LFMQUEUE_DEF_C(from_canton, msg_64_t, 8)

LFMQUEUE_DEF_C(to_spdctl, msg_64_t, 8)
LFMQUEUE_DEF_C(from_spdctl, msg_64_t, 8)

LFMQUEUE_DEF_C(to_forward, msg_64_t, 8)
LFMQUEUE_DEF_C(from_forward, msg_64_t, 8)



static void msg_error(const char *msg)
{

}


static void dispatch_m64(msg_64_t *m, int f)
{
	if (0) {
	} else if (0 == (m->to & 0x00)) { // canton or turnout
		int b = (m->to & 0x38)>>3;
		if (b != localBoardNum) {
			goto forward;
		}
		switch (m->to & 0x40) {
		case 0x80:
			mqf_write_to_canton(m);
			break;
		case 0xC0:
			mqf_write_to_turnout(m);
		}
	} else if ((m->to & 0xE0) == 0x80) {
		// UI
		// drop for now
	} else if ((m->to & 0xE0) == 0xC0) {
		switch (m->to & 0xF8) {
		case 0xC8:
			if (0==localBoardNum) {
				mqf_write_to_spdctl(m);
			} else {
				goto forward;
			}
		}
	} else if (m->to == 0xFF) {
		// broadcast
		mqf_write_to_spdctl(m);
		mqf_write_to_turnout(m);
		mqf_write_to_canton(m);
		if (!f) mqf_write_to_forward(m);
	}
forward:
	if (f) {
		msg_error("message loop");
	} else {
		mqf_write_to_forward(m);
	}
}




void msgsrv_tick(uint32_t tick, uint32_t dt)
{
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_from_canton(&m);
		if (rc) break;
		dispatch_m64(&m, 0);
	}
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_from_turnout(&m);
		if (rc) break;
		dispatch_m64(&m, 0);
	}
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_from_spdctl(&m);
		if (rc) break;
		dispatch_m64(&m, 0);
	}
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_from_forward(&m);
		if (rc) break;
		dispatch_m64(&m, 1);
	}
}



