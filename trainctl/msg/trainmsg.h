/*
 * trainmsg.h
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */

#ifndef MSG_TRAINMSG_H_
#define MSG_TRAINMSG_H_

#include "../utils/lf_mqueue.h"


extern uint8_t localBoardNum;

typedef uint8_t  msg_addr_t;
//
// first 2 bits :
// 0 x : (6bits) bbb xxx
// 1 0 x : (5bits)
// 1 1 0 x x : (3 bits)
// 1 1 1 x x x x x

// canton : up to 8 board with 8 canton
// 0 0  b b b c c c
#define MA_CANTON(_board, _c) (0x80 | (((_board) & 0x7)<<3 | ((_c) & 0x07)))

// turnouts : up to 8 board with 8 turnouts (may be changed to 4 boards with 16 turnouts)
// 0 1  b b b t t t
#define MA_TURNOUT(_board, _c) (0xC0 | (((_board) & 0x7)<<3 | ((_c) & 0x07)))



// UI up to 32 components
// 1 0 0  c c c c c c
#define MA_UI(_c) (0x80 | ((_c) & 0x1F)))

// train spd control : up to 8 trains
//  1 1 0 0  1 t t t
#define MA_TRAIN_SC(_t) (0xC0 | (((_t)& 0x07)))



typedef union {
	struct {
		msg_addr_t to;
		msg_addr_t from;
		union {
			uint8_t vbytes[6];
			struct {
				uint8_t cmd;
				uint8_t sub;
				union {
					uint32_t v32u;
					int32_t v32;
					struct {
						uint16_t v1u;
						uint16_t v2u;
					};
					struct {
						int16_t v1;
						int16_t v2;
					};
				};
		};
	};
	uint64_t raw;
} msg_64_t;


/* general command */
#define CMD_RESET 			0xFF
#define CMD_EMERGENCY_STOP 	0xFE


LFMQUEUE_DEF_H(to_turnout, msg_64_t)
LFMQUEUE_DEF_H(from_turnout, msg_64_t)

/* turnout command */
#define CMD_TURNOUT_A		0x01
#define CMD_TURBOUT_B		0x02


LFMQUEUE_DEF_H(to_canton, msg_64_t)
LFMQUEUE_DEF_H(from_canton, msg_64_t)

LFMQUEUE_DEF_H(to_spdctl, msg_64_t)
LFMQUEUE_DEF_H(from_spdctl, msg_64_t)

LFMQUEUE_DEF_H(to_forward, msg_64_t)
LFMQUEUE_DEF_H(from_forward, msg_64_t)


void msgsrv_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);

#endif /* MSG_TRAINMSG_H_ */
