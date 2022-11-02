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


extern int oam_localBoardNum(void);

typedef uint8_t  msg_addr_t;

/* new: (#15)
 
 
 
  or :
  MA0: explicit board address

  	    0 x x x  b b b b
        0 0 0 0  b b b b    canton
        0 0 0 1  b b b b    turnout
        0 0 1 0  b b b b    relay
        0 0 1 1  b b b b    ina
        0 1 0 0  b b b b    led
        0 1 0 1  b b b b    OAM
        0 1 1 0  b b b b    servos
        0 1 1 1  b b b b    reserved

   MA1: train number (on master)
        1 0 0 x  t t t t
        1 0 0 0  t t t t	ctrl
        1 0 0 1  t t t t	spdctl

   reserved
   	    1 0 1 1  0 x x x

   local only addr
   	    1 0 1 1  1 x x x
   	    1 0 1 1  1 0 0 0  	UI local
   	    1 0 1 1  1 0 0 1    OAM local
   	    1 0 1 1  1 0 1 0    USB local
   	    1 0 1 1  1 - - -    reserved
   	    1 0 1 1  1 1 1 1	local broadcast

   reserved
        1 1 0 x  x x x x
        1 1 1 0  x x x x

   global address (any node may implement the function)
   	    1 1 1 1  x x x x
   	    1 1 1 1  0 0 x x 	UI (ch)
   	    		     0 0    UI general
   	    		   	 0 1    UI ctc
   	    1 1 1 1  0 1 0 0    all slave OAM (sent only by master)
        1 1 1 1  0 1 0 1    planner
   	    1 1 1 1  1 1 1 1   	global broadcast
        
 
 */

#define MA0_ADDR_IS_BOARD_ADDR(_addr) (0==((_addr) & 0x80))
#define MA0_BOARD(_addr)  		((_addr) & 0x0F)

#define MA0_CANTON(_board)		(0x00 | ((_board) & 0x0F))
#define MA0_IS_CANTON(_addr)	(((_addr) & 0xF0) == MA0_CANTON(0))


#define MA0_TURNOUT(_board)		(0x10 | ((_board) & 0x0F))
#define MA0_IS_TURNOUT(_addr)	(((_addr) & 0xF0) == MA0_TURNOUT(0))

#define MA0_RELAY(_board)		(0x20 | ((_board) & 0x0F))
#define MA0_IS_RELAY(_addr)	    (((_addr) & 0xF0) == MA0_RELAY(0))

#define MA0_INA(_board)			(0x30 | ((_board) & 0x0F))
#define MA0_IS_INA(_addr)  		(((_addr) & 0xF0) == MA0_INA(0))


#define MA0_LED(_board)			(0x40 | ((_board) & 0x0F))
#define MA0_IS_LED(_addr)  	   	(((_addr) & 0xF0) == MA0_LED(0))

#define MA0_OAM(_board)			(0x50 | ((_board) & 0x0F))
#define MA0_IS_OAM(_addr)  		(((_addr) & 0xF0) == MA0_OAM(0))


#define MA0_SERVO(_board)		(0x60 | ((_board) & 0x0F))
#define MA0_IS_SERVO(_addr)	    (((_addr) & 0xF0) == MA0_RELAY(0))



#define MA1_ADDR_IS_TRAIN_ADDR(_addr) (0x80 == ((_addr) & 0xE0))
#define MA1_TRAIN(_addr)		 ((_addr) & 0x0F)


#define MA1_CTRL(_trn)			(0x80 | ((_trn) & 0x0F))
#define MA1_CONTROL()			(0x80 | 0x0F)
#define MA1_IS_CTRL(_addr)  	(((_addr) & 0xF0) == MA1_CTRL(0))

#define MA1_SPDCTL(_trn)		(0x90 | ((_trn) & 0x0F))
#define MA1_IS_SPDCTL(_addr)  	(((_addr) & 0xF0) == MA1_SPDCTL(0))


#define MA2_IS_LOCAL_ADDR(_addr)	(0xB8 == ((_addr) & 0xF8))
#define MA2_UI_LOCAL			(0xB8 | 0x00)
#define MA2_OAM_LOCAL			(0xB8 | 0x01)
#define MA2_USB_LOCAL			(0xB8 | 0x02)
#define MA2_LOCAL_BCAST			(0xB8 | 0x07)

#define MA3_IS_GLOB_ADDR(_addr) (0xF0 == ((_addr) & 0xF0))
#define MA3_UI_GEN				(0xF0 | 0x00)
#define MA3_UI_CTC				(0xF0 | 0x01)
#define MA3_SLV_OAM				(0xF0 | 0x04)	// sent only by master
#define MA3_PLANNER             (0xF0 | 0x05)
#define MA3_BROADCAST			(0xFF)


#define IS_BROADCAST(_addr) (((_addr) == MA3_BROADCAST) || ((_addr) == MA2_LOCAL_BCAST))

#define CNUM(_board, _canton)  ((((_board) & 0x0F)<<4) | ((_canton) & 0x0F))

//#define FROM_CANTON(m) ((MA0_BOARD((m).from) << 4) | ((m).subc & 0x0F))



// check
// CMD_STOP  CMD_BEMF_ON CMD_SETVPWM  CMD_BEMF_NOTIF
// CMD_SET_C1_C2





typedef union {
	struct {
		msg_addr_t to;
		msg_addr_t from;
		union {
			uint8_t rbytes[6];
			struct {
				uint8_t cmd;
                union {
                    struct {
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
                            struct { // CMD_POSE_SET_TRIG
                            	int16_t va16;
                            	int8_t vb8;
                            	uint8_t vcu8;
                            };
                            uint8_t vbytes[4];
                            struct {
                                uint8_t vb0;
                                uint8_t vb1;
                                uint8_t vb2;
                                uint8_t vb3;
                            };
                            
                        };
                    } __attribute__((packed));
					uint64_t val40:40 ;
				} __attribute__((packed));
			}  __attribute__((packed));
		};
	} __attribute__((packed));
	uint64_t raw;
} msg_64_t;

#if defined(TRAIN_SIMU) || defined (TOPOLOGY_SVG)
// mac llvm clang does not support compile_assert(), doing it old way
typedef char compile_assert[(sizeof(msg_64_t) == 8) ? 1 : -1];
#else
static_assert(sizeof(msg_64_t) == 8);
#endif

typedef enum {
	runmode_off,				// when switching mode
    runmode_master,
    runmode_slave,
	runmode_normal,
	runmode_testcanton,
	runmode_detect_experiment,	// obsolete, used for experimentation
	runmode_detect2,			// auto detect trains

	runmode_testcan,			// test CAN bus
} runmode_t;

#ifdef BOARD_HAS_CAN
LFMQUEUE_DEF_H(to_canbus_loc, msg_64_t)
LFMQUEUE_DEF_H(to_canbus, msg_64_t)
LFMQUEUE_DEF_H(from_canbus, msg_64_t)
#endif


#ifdef BOARD_HAS_USB
LFMQUEUE_DEF_H(to_usb, msg_64_t)
LFMQUEUE_DEF_H(from_usb, msg_64_t)
#endif


#ifdef BOARD_HAS_TURNOUTS
LFMQUEUE_DEF_H(to_turnout, msg_64_t)
LFMQUEUE_DEF_H(from_turnout, msg_64_t)
#endif

#ifdef BOARD_HAS_CANTON
LFMQUEUE_DEF_H(to_canton, msg_64_t)
LFMQUEUE_DEF_H(from_canton, msg_64_t)
#endif

#ifdef BOARD_HAS_INA3221
LFMQUEUE_DEF_H(to_ina3221, msg_64_t)
LFMQUEUE_DEF_H(from_ina3221, msg_64_t)
#endif

#ifdef BOARD_HAS_LED
LFMQUEUE_DEF_H(to_led, msg_64_t)
LFMQUEUE_DEF_H(from_led, msg_64_t)
#endif

#ifdef BOARD_HAS_CTRL
LFMQUEUE_DEF_H(to_spdctl, msg_64_t)
LFMQUEUE_DEF_H(from_spdctl, msg_64_t)

LFMQUEUE_DEF_H(to_ctrl, msg_64_t)
LFMQUEUE_DEF_H(from_ctrl, msg_64_t)
#endif


#ifdef BOARD_HAS_IHM
LFMQUEUE_DEF_H(to_ui, msg_64_t)
LFMQUEUE_DEF_H(from_ui, msg_64_t)
#endif

#ifdef BOARD_HAS_UI_CTC
LFMQUEUE_DEF_H(to_ui_track, msg_64_t)
LFMQUEUE_DEF_H(from_ui_track, msg_64_t)
#endif


#ifdef BOARD_HAS_TRKPLN
LFMQUEUE_DEF_H(to_planner, msg_64_t)
LFMQUEUE_DEF_H(from_planner, msg_64_t)
#endif


#ifdef BOARD_HAS_OSCILLO
LFMQUEUE_DEF_H(from_oscillo, msg_64_t)
#endif

#ifdef BOARD_HAS_SERVOS
LFMQUEUE_DEF_H(to_servo, msg_64_t)
LFMQUEUE_DEF_H(from_servo, msg_64_t)
#endif

LFMQUEUE_DEF_H(to_oam, msg_64_t)
LFMQUEUE_DEF_H(from_oam, msg_64_t)

void msgsrv_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);

// TODO
void purge_all_queue(void);

LFMQUEUE_DEF_H(from_nowhere, msg_64_t)

#include "trainmsgdef.h"


// ---------------

// new type xblkaddr_t, castable to uint8_t, replaces old uint8_t to avoid confusion and error
typedef union {
	struct {
		uint8_t canton:4;
		uint8_t board:4;
	};
	uint8_t v;
} xblkaddr_t;


#ifndef __clang__
static_assert(sizeof(xblkaddr_t) == 1);
#else
typedef char xcompile_assert[(sizeof(xblkaddr_t) == 1) ? 0 : -1];
#endif

#define FROM_CANTON(m) (_from_canton(&m))

static inline xblkaddr_t _from_canton(msg_64_t *m)
{
	xblkaddr_t r;
	r.board = MA0_BOARD(m->from);
	r.canton = m->subc;
	return r;
}

#define TO_CANTON(_m, _caddr) do {	\
	(m).to = MA0_CANTON((_caddr).board);	\
	(m).subc = (_caddr).canton;		\
} while (0)							\


// ---------------

typedef union {
	struct {
		uint8_t turnout:4;
		uint8_t board:4;
	};
	uint8_t v;
} xtrnaddr_t;


#endif /* MSG_TRAINMSG_H_ */
