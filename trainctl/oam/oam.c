/*
 * cantest.c
 *
 *  Created on: 2 avr. 2022
 *      Author: danielbraun
 */






#include <stdint.h>
#include <memory.h>

#include "../misc.h"
#include "../msg/trainmsg.h"


#include "oam.h"
#ifdef BOARD_HAS_FLASH
#include "oam_flash.h"
#endif

#include "boards.h"
#include "../config/propag.h"

#include "../config/conf_globparam.h"
#include "../config/conf_globparam.propag.h"
#include "../config/conf_utest.propag.h"

int OAM_NeedsReschedule = 0;

static runmode_t run_mode = 0;

static int initdone = 0;

void OAM_Init(void)
{
	itm_debug1(DBG_OAM, "OAM init", 0);
	oam_flash_init();
    initdone=1;
	itm_debug1(DBG_OAM, "OAM loc rd", 0);
    oam_flashlocal_read(-1);
    // TODO propag normal store if master
	itm_debug1(DBG_OAM, "OAM ready", 0);
	if ((1)) {
		// void oam_flashstore_set_value(int confnum, int fieldnum, int confbrd, int instnum, int32_t v)
		// uint32_t oam_flashstore_get_value(int confnum, int fieldnum, int confbrd, int instnum)
		oam_flashstore_set_value(conf_pnum_utest, conf_numfield_utest_beta, 0, 0, 4242);
		itm_debug1(DBG_OAM, "T/store", 0);
		int32_t v = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_beta, 0, 0);
		itm_debug1(DBG_OAM, "T/read", v);

	}
}


static void exit_can_test(void)
{
	msg_64_t m;
	m.from = MA3_BROADCAST;
	m.to = MA3_BROADCAST;
	m.cmd = CMD_SETRUN_MODE;
	m.v1u =  runmode_normal;
	mqf_write_from_nowhere(&m);
}


static void customOam(msg_64_t *m);
static void handle_slave_msg(msg_64_t *m);
static void handle_slave_tick(uint32_t tick);

static void handle_testcan_tick(uint32_t tick);
static void handle_testcan_msg(msg_64_t *m);

static int respok = 0;


void OAM_Tasklet(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
    if (!initdone) {
        OAM_Init();
    }
	static int first = 1;
	if ((first)) {
		//uint32_t myid = oam_getDeviceUniqueId();
		//int myboard = boardIdToBoardNum(myid);
    
		first = 0;
		msg_64_t m;
		m.from = MA3_BROADCAST;
		m.to = MA2_LOCAL_BCAST;
		m.cmd = CMD_SETRUN_MODE;
        if (oam_isMaster()) {
            m.v1u =  runmode_master; //runmode_normal; //runmode_testcan; // runmode_normal; runmode_detect2; runmode_off
            m.v1u = runmode_normal; // XXX temp, delete me
        } else {
            m.v1u = runmode_slave;
        }
        mqf_write_from_nowhere(&m); // from_nowher, otherwise it wont be sent to self
	}


	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_oam(&m);
		if (rc) break;

        unsigned int instnum;
        unsigned int confnum;
        unsigned int fieldnum;
        unsigned int confbrd;
        int32_t v;
        uint64_t enc;

        switch (m.cmd) {
        case CMD_SETRUN_MODE:
        	if (m.v1u != run_mode) {
        		run_mode = (runmode_t) m.v1u;
        		if (run_mode == runmode_testcan) {
        			respok = 0;
        		}
        	}
        	break;

        case CMD_OAM_CUSTOM:
                customOam(&m);
                break;
        case CMD_PARAM_USER_SET:
        	if (!oam_isMaster()) {
        		itm_debug1(DBG_OAM|DBG_ERR, "only master recv set", 0);
        		Error_Handler();
        		break;
        	}

        	oam_decode_val40(m.val40, &confnum, &confbrd, &instnum, &fieldnum, &v);
        	// store in flash
        	oam_flashstore_set_value(confnum, fieldnum, confbrd, instnum, v);
        	if (confbrd == 0)  {
        		// master == local board
        		conf_propagate(confnum, fieldnum, instnum, v);
        	} else {
        		m.cmd = CMD_PARAM_PROPAG;
        		m.to = MA0_OAM(confbrd);
        		mqf_write_from_oam(&m);
        	}
        	break;
        case CMD_PARAM_USER_GET:
        	if (!oam_isMaster()) {
        		itm_debug1(DBG_OAM|DBG_ERR, "only master recv get", 0);
        		FatalError("NoMast", "only master should receive this", Error_NotMaster);
        		break;
        	}

        	oam_decode_val40(m.val40, &confnum, &confbrd, &instnum, &fieldnum, &v);
        	int normal = 1;
        	if (confnum == conf_lnum_globparam) {
        		// special handling for some param
        		if (fieldnum == conf_numfield_globparam_oscillo) {
#ifdef BOARD_HAS_OSCILLO
        			extern int oscillo_enable;
        			v = oscillo_enable;
#else
        			v = 0;
#endif
        			normal = 0;
        		}
        	}
        	if (normal) {
        		v = oam_flashstore_get_value(confnum, fieldnum, confbrd, instnum);
        	}
    		oam_encode_val40(&enc, confnum, confbrd, instnum, fieldnum, v);
        	m.to = m.from;
        	m.from = MA0_OAM(0);
        	m.cmd = CMD_PARAM_USER_VAL;
            m.val40 = enc;
        	mqf_write_from_oam(&m);
        	break;


        case CMD_PARAM_LUSER_SET:
        	if (!oam_isMaster()) {
        		itm_debug1(DBG_OAM|DBG_ERR, "only master recv set", 0);
        		FatalError("NoMast", "only master should receive this", Error_NotMaster);
        		break;
        	}
        	oam_decode_val40(m.val40, &confnum, &confbrd, &instnum, &fieldnum, &v);

        	if (confnum == conf_lnum_globparam) {
        		// special handling for some param
        		if (fieldnum == conf_numfield_globparam_oscillo) {
#ifdef BOARD_HAS_OSCILLO
        			extern int oscillo_enable;
        			oscillo_enable = v;
#endif
        			break;
        		}
        	}
        	// store in flash
        	oam_flashlocal_set_value(confnum, fieldnum,  instnum, v);
        	break;


        case CMD_PARAM_LUSER_COMMIT:
        	if (!oam_isMaster()) {
        		itm_debug1(DBG_OAM|DBG_ERR, "only master recv commit", 0);
        		FatalError("NoMast", "only master should receive this", Error_NotMaster);
        		break;
        	}
        	oam_flashlocal_commit(m.v1);
        	break;



        case CMD_PARAM_LUSER_GET:
        	if (!oam_isMaster()) {
        		itm_debug1(DBG_OAM|DBG_ERR, "only master recv get", 0);
        		FatalError("NoMast", "only master should receive this", Error_NotMaster);
        		break;
        	}
        	oam_decode_val40(m.val40, &confnum, &confbrd, &instnum, &fieldnum, &v);
        	v = oam_flashlocal_get_value(confnum, fieldnum, instnum);
        	oam_encode_val40(&enc, confnum, 0, instnum, fieldnum, v);
        	m.to = m.from;
        	m.from = MA0_OAM(0);
        	m.cmd = CMD_PARAM_LUSER_VAL;
            m.val40 = enc;
        	mqf_write_from_oam(&m);
        	break;

        case CMD_PARAM_PROPAG: {
        	if (oam_isMaster()) {
        		itm_debug1(DBG_OAM|DBG_ERR, "only slave recv propag", 0);
        		FatalError("NoSlv", "only slave should receive this", Error_NotSlave);
        		break;
        	}
        	unsigned int instnum;
        	unsigned int confnum;
        	unsigned int fieldnum;
        	unsigned int confbrd;
        	int32_t v;
        	oam_decode_val40(m.val40, &confnum, &confbrd, &instnum, &fieldnum, &v);
        	conf_propagate(confnum, fieldnum, instnum, v);
        }
        break;


        default:
        	break;
        }
        if (runmode_testcan == run_mode) {
            handle_testcan_msg(&m);
        } else if (oam_isMaster()) {
            // TODO
        } else {
            handle_slave_msg(&m);
        }
	}
	switch (run_mode) {
	case runmode_testcan:
            handle_testcan_tick(tick);
            break;
        case runmode_master:
            break;
        case runmode_slave:
            handle_slave_tick(tick);
            break;
	default:
		return;
	}
}

static void handle_testcan_msg(msg_64_t *m)
{
	switch (m->cmd)  {
	default: break;
	case CMD_CANTEST:
		if ((0)) { // XXX test relat
			static int cnt=0;
			cnt++;
			if (0==(cnt%4)) {
				static int a = 0;
				msg_64_t m = {0};
				m.to = MA0_TURNOUT(1);
				m.subc = 0;
				m.from = MA0_OAM(1);
				m.cmd = a ? CMD_TURNOUT_A : CMD_TURNOUT_B;
				a = a ? 0 : 1;
				mqf_write_from_oam(&m);
			}
		}
		if (run_mode != runmode_testcan) break;
		if (oam_isMaster()) break;
		m->cmd = CMD_CANTEST_RESP;
		m->v2u = m->v1u*2;
		mqf_write_from_oam(m);
		break;

	case CMD_CANTEST_RESP:
		// also handled by IHM
		if (run_mode == runmode_testcan) {
			respok++;
			if (respok > 20) {
				respok = 0;
				exit_can_test();
			}
		}
		break;
	}
}
    
    
static void handle_testcan_tick(uint32_t tick)
{
	if (!oam_isMaster()) return;

	// send ping
	static uint32_t lasttick = 0;

	if (tick>lasttick+1000) {
		lasttick = tick;
		static uint16_t cnt = 0;
		cnt++;
		if (cnt > 999) {
			respok = 0;
			exit_can_test();
			return;
		}

		msg_64_t m = {0};
		m.from = MA3_BROADCAST;
		m.to = MA3_BROADCAST;
		m.cmd = CMD_CANTEST;
		m.v1u = cnt;

		mqf_write_from_oam(&m);
	}
}

/*
 * file:	4bit
 * board:   4 bits
 * inst:    6 bits
 * field:   5 bits
 * value : 21 bits
 */

void oam_decode_val40(uint64_t  val40, unsigned int *fnum, unsigned int *brd, unsigned int *inst, unsigned int *fld, int32_t *v)
{
	*fnum = (val40 >> (21+5+6+4)) & 0x0F;
	*brd  = (val40 >> (21+5+6))   & 0x0F;
	*inst = (val40 >> (21+5))     & 0x3F;
	*fld =  (val40 >> (21))       & 0x1F;
	*v   =  (val40 >> (0))        & 0x1FFFFF;
}
void oam_encode_val40(uint64_t *val40, unsigned int  fnum, unsigned int brd, unsigned int  inst, unsigned int  fld, int32_t  v)
{
	*val40 = 0;
	*val40 |= (uint64_t) (v & 0x1FFFFF)  << 0;
	*val40 |= (uint64_t) (fld  & 0x1F)   << (21);
	*val40 |= (uint64_t) (inst & 0x3F)   << (21+5);
	*val40 |= (uint64_t) (brd & 0x0F)    << (21+5+6);
	*val40 |= (uint64_t) (fnum & 0x0F)   << (21+5+6+4);
}



uint32_t oam_getDeviceUniqueId(void)
{
#ifndef TRAIN_SIMU
	/* Read MCU Id, 32-bit access */
	uint32_t id0 = HAL_GetUIDw0();
	uint32_t id1 = HAL_GetUIDw1();
	uint32_t id2 = HAL_GetUIDw2();

	// stm32 gets a uniq 96 bits id, but this is too large for a msg64_t
	// so we need to reduce it to 32 bits (or at least 64)

	uint32_t xid = id0 ^ id1 ^ id2;
	// XXX TODO use crc32 instead
	return xid;
#else
    return 42;
#endif
}


// --------------------------------------------------------------


static void customOam(msg_64_t *m)
{
	switch (m->subc) {
	case 42:
		FatalError("CUSTOM", "OAM custom error", 0x00);
		break;
	default:
		itm_debug2(DBG_OAM, "custom", m->subc, m->from);
		break;
	}
}


// --------------------------------------------------------------

/* O&M slave handling */

static uint32_t lastBcast = 0;
enum oam_slv_state {
    oam_slv_bcast,
};
static enum oam_slv_state slvState = oam_slv_bcast;

static void handle_slave_tick(uint32_t tick)
{
    if (slvState == oam_slv_bcast) {
        if (tick > lastBcast+200) {
            lastBcast = tick;
            msg_64_t m = {0};
            m.cmd = CMD_OAM_SLAVE;
            m.v32u = oam_getDeviceUniqueId();
            m.from = MA3_BROADCAST;
            m.to = MA0_OAM(0);
            mqf_write_from_oam(&m);
        }
    }
}

static void handle_slave_msg(msg_64_t *m)
{
	switch (m->cmd) {
	default: break;
	case CMD_OAM_BNUM:
		if (oam_getDeviceUniqueId() != m->v32u) {
			itm_debug2(DBG_ERR|DBG_OAM, "bad uniq", m->v32u, oam_getDeviceUniqueId());
			return;
		}
		oam_store_slave_local_boardnum(m->subc);
		break;

	}
}
// --------------------------------------------------------------
// --------------------------------------------------------------
// --------------------------------------------------------------
