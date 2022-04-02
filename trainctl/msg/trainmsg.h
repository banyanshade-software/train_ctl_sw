/*
 * trainmsg.h
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */

#ifndef MSG_TRAINMSG_H_
#define MSG_TRAINMSG_H_

#include <memory.h>
#include <string.h>
#include "../utils/lf_mqueue.h"
#include "notif.h"

extern uint8_t localBoardNum;

typedef uint8_t  msg_addr_t;
// old
// first 2 bits :
// M2:  0 x : (6bits) bbb xxx		CANTON (00) and TURNOUT (01)
// M3   1 0 x : (5bits)				UI
// M5   1 1 0 x x : (3 bits)		SPD_CTL (110 01) + trn,  CTRL (110 10) + trn , LED (110 11)+brd, INA (110 00)+brd
// M8   1 1 1 1 1 x x x				OAM

/* new:
 *
 * M2 :		0 0 b b b c c c 		CANTON (00)
 *
 * M5t:		0 1 0 x x t t t			per train addr, all on board 0
 * 									1 0 0  0 0  t t t  = CTRL
 * 									1 0 0  0 1  t t t  = SPDCTL
 *
 * M5b :    1 0 x x x b b b			per board addr
 * 			   						1 0  0 0 0  b b b 	= turnout
 * 			   						1 0  0 0 1  b b b 	= led
 * 			   						1 0  0 1 0  b b b   = relay
 * 			   						1 0  0 1 1  b b b	= INA
 *
 * 			   						1 0  1 1 1  b b b   = OAM
 *
 * GLO :    1 1 0 0 x x x x			global addresses (1100, extensible to 1101, 1110)
 * 		 							1 1 0  0 0 0 0 0   	= UI (gen)
 * 									1 1 0  0 0 0 0 1 	= UI (ctc)
 *
 * LOC :    1 1 1 1 x x x x			local addresses, never routed (xxxx != 1111)
 * 									1 1 1 1  0 0 0 0	= UI (local)
 * 									1 1 1 1  0 1 1 1	= OAM (local)
 *
 *  BCAST:  1 1 1 1 1 1 1 1
 *
 *
 *
 */

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
#define MA_GET_CANTON_NUM(_addr) ((_addr) & 0x07)

// turnouts : up to 8 board with 8 turnouts (may be changed to 4 boards with 16 turnouts)
// 0 1  b b b t t t
#define MA_ADDR_2_TURNOUT	0x40
#define MA_TURNOUT(_board, _c) (MA_ADDR_2_TURNOUT | (((_board) & 0x7)<<3 | ((_c) & 0x07)))
#define IS_TURNOUT(_addr) (MA_ADDR_2_TURNOUT==((_addr) & MA_ADDR_MASK_2))

#define MA_2_BOARD(_addr) (((_addr) & 0x38) >> 3)


// UI up to 32 components
// 1 0 0  c c c c c c
#define MA_UI(_c) (0x80 | ((_c) & 0x1F))
#define MA_ADDR_3_UI 	0x80
#define IS_UI(_addr) (MA_ADDR_3_UI == ((_addr) & MA_ADDR_MASK_3))

// MA_UI sub adresses
#define UISUB_USB   0   // usb connected GUI (train_throttle)
#define UISUB_TFT   1   // normal UI
#define UISUB_TRACK 2   // track diagram

// train spd control : up to 8 trains
//  1 1 0 0  1 t t t
#define MA_ADDR_5_TRSC	0xC8
#define MA_TRAIN_SC(_t) (MA_ADDR_5_TRSC | (((_t)& 0x07)))
#define IS_TRAIN_SC(_addr) (MA_ADDR_5_TRSC == ((_addr) & MA_ADDR_MASK_5))
#define MA_GET_TRAINNUM(_addr) ((_addr) & 0x07)


// train hilevel control
#define MA_ADDR_5_CTRL	0xD0
#define MA_CONTROL_T(_t) (MA_ADDR_5_CTRL  | (((_t)& 0x07)))
#define MA_CONTROL() MA_CONTROL_T(7)
#define IS_CONTROL_T(_addr)  (MA_ADDR_5_CTRL == ((_addr) & MA_ADDR_MASK_5))

// led (per board)
#define MA_ADDR_5_LED 	0xD8
#define MA_LED_B(_b) (MA_ADDR_5_LED  | (((_b)& 0x07)))
#define IS_LED_B(_addr) (MA_ADDR_5_LED == ((_addr) & MA_ADDR_MASK_5))

// ina3221 (per board)
#define MA_ADDR_5_INA 0xC0
#define MA_INA3221_B(_b) (MA_ADDR_5_INA  | (((_b)& 0x07)))
#define IS_INA3221_B(_addr) (MA_ADDR_5_INA == ((_addr) & MA_ADDR_MASK_5))

// OAM (per board)
#define MA_ADDR_OAM 0xF8
#define MA_OAM(_b)		(MA_ADDR_OAM | ((_b)& 0x07)))
#define IS_OAM(_addr)	(MA_ADDR_OAM == ((_addr) & MA_ADDR_MASK_5))

#define IS_BROADCAST(_addr) (0xFF == (_addr))
#define MA_BROADCAST 0xFF

typedef union {
	struct {
		msg_addr_t to;
		msg_addr_t from;
		union {
			uint8_t rbytes[6];
			struct {
				uint8_t cmd;
				uint8_t subc;
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
                    struct {
                        uint8_t vb0;
                        uint8_t vb1;
                        uint8_t vb2;
                        uint8_t vb3;
                    };
				};
			}  __attribute__((packed));
		};
	} __attribute__((packed));
	uint64_t raw;
} msg_64_t;

typedef enum {
	runmode_off,				// when switching mode
	runmode_normal,
	runmode_testcanton,
	runmode_detect_experiment,	// obsolete, used for experimentation
	runmode_detect2,			// auto detect trains

	runmode_testcan,			// test CAN bus
} runmode_t;

LFMQUEUE_DEF_H(to_canbus, msg_64_t)
LFMQUEUE_DEF_H(from_canbus, msg_64_t)



LFMQUEUE_DEF_H(to_turnout, msg_64_t)
LFMQUEUE_DEF_H(from_turnout, msg_64_t)


LFMQUEUE_DEF_H(to_canton, msg_64_t)
LFMQUEUE_DEF_H(from_canton, msg_64_t)


LFMQUEUE_DEF_H(to_spdctl, msg_64_t)
LFMQUEUE_DEF_H(from_spdctl, msg_64_t)


LFMQUEUE_DEF_H(to_ctrl, msg_64_t)
LFMQUEUE_DEF_H(from_ctrl, msg_64_t)



//LFMQUEUE_DEF_H(to_forward, msg_64_t)
//LFMQUEUE_DEF_H(from_forward, msg_64_t)

LFMQUEUE_DEF_H(to_forward_usb, msg_64_t)
LFMQUEUE_DEF_H(from_forward_usb, msg_64_t)


LFMQUEUE_DEF_H(to_ui, msg_64_t)
LFMQUEUE_DEF_H(from_ui, msg_64_t)


LFMQUEUE_DEF_H(to_ui_track, msg_64_t)
LFMQUEUE_DEF_H(from_ui_track, msg_64_t)


LFMQUEUE_DEF_H(to_ina3221, msg_64_t)
LFMQUEUE_DEF_H(from_ina3221, msg_64_t)


LFMQUEUE_DEF_H(to_led, msg_64_t)
LFMQUEUE_DEF_H(from_led, msg_64_t)



LFMQUEUE_DEF_H(to_oam, msg_64_t)
LFMQUEUE_DEF_H(from_oam, msg_64_t)


void msgsrv_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);

// TODO
void purge_all_queue(void);

LFMQUEUE_DEF_H(from_nowhere, msg_64_t)

#include "trainmsgdef.h"

#endif /* MSG_TRAINMSG_H_ */
