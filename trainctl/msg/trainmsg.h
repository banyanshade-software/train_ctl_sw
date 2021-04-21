/*
 * trainmsg.h
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */

#ifndef MSG_TRAINMSG_H_
#define MSG_TRAINMSG_H_

#include "../utils/lf_mqueue.h"
#include "notif.h"

extern uint8_t localBoardNum;

typedef uint8_t  msg_addr_t;
//
// first 2 bits :
// 0 x : (6bits) bbb xxx
// 1 0 x : (5bits)
// 1 1 0 x x : (3 bits)
// 1 1 1 x x x x x

#define MA_ADDR_MASK_2		0xC0
#define MA_ADDR_MASK_BOARD	0x38
#define MA_ADDR_MASK_3 		0xE0
#define MA_ADDR_MASK_5		0xF8
#define MA_ADDR_MASK_8		0xFF

// canton : up to 8 board with 8 canton
// 0 0  b b b c c c
#define MA_ADDR_2_CANTON	0x00
#define MA_CANTON(_board, _c) (MA_ADDR_2_CANTON| (((_board) & 0x7)<<3 | ((_c) & 0x07)))
#define IS_CANTON(_addr) (MA_ADDR_2_CANTON==((_addr) & MA_ADDR_MASK_2))

// turnouts : up to 8 board with 8 turnouts (may be changed to 4 boards with 16 turnouts)
// 0 1  b b b t t t
#define MA_ADDR_2_TURNOUT	0x40
#define MA_TURNOUT(_board, _c) (MA_ADDR_2_CANTON | (((_board) & 0x7)<<3 | ((_c) & 0x07)))
#define IS_TURNOUT(_addr) (MA_ADDR_2_CANTON==((_addr) & MA_ADDR_MASK_2))

#define MA_2_BOARD(_addr) (((_addr) & 0x38) >> 3)


// UI up to 32 components
// 1 0 0  c c c c c c
#define MA_UI(_c) (0x80 | ((_c) & 0x1F))
#define MA_ADDR_3_UI 	0x80
#define IS_UI(_addr) (MA_ADDR_3_UI == ((_addr) & MA_ADDR_MASK_3))

// train spd control : up to 8 trains
//  1 1 0 0  1 t t t
#define MA_ADDR_5_TRSC	0xC8
#define MA_TRAIN_SC(_t) (MA_ADDR_5_TRSC | (((_t)& 0x07)))
#define IS_TRAIN_SC(_addr) (MA_ADDR_5_TRSC == ((_addr) & MA_ADDR_MASK_5))


// train hilevel control
#define MA_ADDR_5_CTRL	0xD0
#define MA_CONTROL_T(_t) (MA_ADDR_5_CTRL  | (((_t)& 0x07)))
#define MA_CONTROL() MA_CONTROL_T(7);
#define IS_CONTROL_T(_addr)  (MA_ADDR_5_CTRL == ((_addr) & MA_ADDR_MASK_5))

#define IS_BROADCAST(_addr) (0xFF == (_addr))


typedef union {
	struct {
		msg_addr_t to;
		msg_addr_t from;
		union {
			uint8_t rbytes[6];
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
                    uint8_t vbytes[4];
				};
			}  __attribute__((packed));
		};
	} __attribute__((packed));
	uint64_t raw;
} msg_64_t;


/* general command */
#define CMD_RESET 			0xFF
#define CMD_EMERGENCY_STOP 	0xFE


LFMQUEUE_DEF_H(to_turnout, msg_64_t)
LFMQUEUE_DEF_H(from_turnout, msg_64_t)

/* turnout command */
#define CMD_TURNOUT_A		0x01
#define CMD_TURNOUT_B		0x02


LFMQUEUE_DEF_H(to_canton, msg_64_t)
LFMQUEUE_DEF_H(from_canton, msg_64_t)
/*to_canton : */
#define CMD_BEMF_ON			0x40
#define CMD_BEMF_OFF		0x41

#define CMD_SETVPWM			0x01
#define CMD_STOP			0x02

LFMQUEUE_DEF_H(to_spdctl, msg_64_t)
LFMQUEUE_DEF_H(from_spdctl, msg_64_t)

/* from canton: */
#define CMD_BEMF_NOTIF		0x03

/* from upper */
#define CMD_SET_TARGET_SPEED 0x10
#define CMD_SET_C1_C2		 0x11


LFMQUEUE_DEF_H(to_ctrl, msg_64_t)
LFMQUEUE_DEF_H(from_ctrl, msg_64_t)

/* to ctrl */
#define CMD_PRESENCE_CHANGE	0x12

LFMQUEUE_DEF_H(to_forward, msg_64_t)
LFMQUEUE_DEF_H(from_forward, msg_64_t)

LFMQUEUE_DEF_H(to_forward_usb, msg_64_t)
LFMQUEUE_DEF_H(from_forward_usb, msg_64_t)

/* to UI */
#define CMD_NOTIF_SPEED     0xA0

void msgsrv_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);





#endif /* MSG_TRAINMSG_H_ */