/*
 * ihm_messages.h
 *
 *  Created on: Feb 12, 2024
 *      Author: danielbraun
 */

#ifndef UTILS_IHM_MESSAGES_H_
#define UTILS_IHM_MESSAGES_H_

#include <stdint.h>

#include "../misc.h"
#include "../msg/trainmsg.h"

#define _RESERVED(_id) _id
#define _M(_id, _short, _long) _id
typedef enum ihmmsg {
#include "ihm_msg.inc"
} ihm_msg_t;

#undef _RESERVED
#undef _M


extern const char *ihmmsglist[];

// eg ihm_message(mqf_write_from_nowhere, MSG_AUTO_DONE, 0, 0)

static inline void ihm_message(int (*snd)(msg_64_t *), ihm_msg_t mnum, uint8_t subc, int16_t v2)
{
	msg_64_t m = {0};
	m.from = MA3_BROADCAST; // any
	m.to = MA3_UI_GEN;
	m.cmd = CMD_UI_MSG;
	m.v1u = (uint16_t)mnum;
	m.v2 = v2;
	m.subc = subc;
	snd(&m);
}

#endif /* UTILS_IHM_MESSAGES_H_ */
