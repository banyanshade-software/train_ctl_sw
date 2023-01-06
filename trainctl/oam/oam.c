/*
 * oam.c
 *
 *  Created on: 2 avr. 2022
 *      Author: danielbraun
 */




//#define RECORD_MSG 1

#include <stdint.h>
#include <memory.h>
#include "../utils/itm_debug.h"

#include "../misc.h"
#include "../msg/trainmsg.h"
#include "../msg/tasklet.h"


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




// ------------------------------------------------------

static void OAM_Init(void);
static void OAM_change_mode(runmode_t m);

static msg_handler_t msg_handler_selector(runmode_t);
static tick_handler_t tick_handler_selector(runmode_t);
static void pretick_handler(uint32_t t, uint32_t dt);

static const tasklet_def_t oam_tdef = {
		.init 				= OAM_Init,
		.poll_divisor		= NULL,
		.emergency_stop 	= NULL,
		.enter_runmode		= OAM_change_mode,
		.pre_tick_handler	= pretick_handler,
		.default_msg_handler = NULL,
		.default_tick_handler = NULL,
		.msg_handler_for	= msg_handler_selector,
		.tick_handler_for 	= tick_handler_selector,

		.recordmsg			= RECORD_MSG,

};
tasklet_t OAM_tasklet = { .def = &oam_tdef, .init_done = 0, .queue=&to_oam};

// ------------------------------------------------------

#ifdef STM32F1

static uint8_t nflash = 0;
static uint8_t ntick = 0;

static void flash_led_tick(void)
{
	if (!nflash) return;
	if (!ntick) {
		HAL_GPIO_TogglePin(BOARD_LED_GPIO_Port, BOARD_LED_Pin);
		ntick = 5;
		nflash--;
	} else {
		ntick--;
	}
}

static void oam_flash_led(void)
{
	nflash = 5;
	ntick = 0;
}


#else
static void flash_led_tick(void)
{
}

static  void oam_flash_led(void)
{

}
#endif

static void pretick_handler(_UNUSED_ uint32_t t, _UNUSED_ uint32_t dt)
{
	flash_led_tick();
}
// ------------------------------------------------------

static void handle_testcan_msg(msg_64_t *m);
static void handle_msg_slave(msg_64_t *m);
static void handle_msg_master(msg_64_t *m);

static void handle_slave_tick(uint32_t tick, uint32_t dt);
static void handle_master_tick(uint32_t tick, uint32_t dt);
static void handle_testcan_tick(uint32_t tick, uint32_t dt);



static msg_handler_t msg_handler_selector(runmode_t m)
{
	switch (m) {
        case runmode_testcan:   return handle_testcan_msg;
        case runmode_master:    return handle_msg_master;
        case runmode_slave:     return handle_msg_slave;
        default: break;
	}
    if (oam_isMaster()) {
        return handle_msg_master;
    } else {
        return handle_msg_slave;
    }
}

static tick_handler_t tick_handler_selector(runmode_t m)
{
    switch (m) {
        case runmode_testcan:   return handle_testcan_tick;
        case runmode_master:    return handle_master_tick;
        case runmode_slave:     return handle_slave_tick;

        default:
        	if (oam_isMaster()) {
        		return handle_master_tick;
        	} else {
        		return handle_slave_tick;
        	}
        	break;
    }
    return NULL;
}

// ------------------------------------------------------

//static runmode_t run_mode = 0;

//static int initdone = 0;

static int respok = 0; // for testcan

volatile int Oam_Ready = 0;

static uint32_t bootcount=0;

static void OAM_Init(void)
{
    //initdone=1;
    
    // init flash
#ifdef BOARD_HAS_FLASH
	itm_debug1(DBG_OAM, "OAM init", 0);
	oam_flash_init();
	itm_debug1(DBG_OAM, "OAM loc rd", 0);
    oam_flashlocal_read(-1);

	if ((0)) {
		// void oam_flashstore_set_value(int confnum, int fieldnum, int confbrd, int instnum, int32_t v)
		// uint32_t oam_flashstore_get_value(int confnum, int fieldnum, int confbrd, int instnum)
		oam_flashstore_set_value(conf_pnum_utest, conf_numfield_utest_beta, 0, 0, 4242);
		itm_debug1(DBG_OAM, "T/store", 0);
		int32_t v = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_beta, 0, 0);
		itm_debug1(DBG_OAM, "T/read", v);
	}

	if (oam_isMaster()) {
	    //  propag normal store locally if master
		oam_flashstore_rd_rewind();
		for (;;) {
			unsigned int confnum; unsigned int fieldnum;
			unsigned int confbrd; unsigned int instnum;
			int32_t v;

			int rc = oam_flashstore_rd_next(&confnum, &fieldnum, &confbrd, &instnum, &v);
			if (rc<0) {
				// EOF
				break;
			}
			if (confbrd != 0) continue;
			conf_propagate(confnum, fieldnum, instnum, v);
		}
		oam_flashstore_rd_rewind();
	}
#endif
    
    // other init
	itm_debug1(DBG_OAM, "OAM ready", 0);
    
    // initial mode
    static int first = 1;
    if ((first)) {
        first = 0;
    	/*
    	 * update bootcount
    	 */
    	if ((1)) {
#ifdef TRAIN_SIMU
            bootcount=1;
#else
#ifdef BOARD_CAN_BE_MASTER
    		extern RTC_HandleTypeDef hrtc;
    		bootcount = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
    		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, bootcount+1);
#endif
#endif
    	}
        //uint32_t myid = oam_getDeviceUniqueId();
        //int myboard = boardIdToBoardNum(myid);
    
        msg_64_t m;
        m.from = MA3_BROADCAST;
        m.to = MA2_LOCAL_BCAST;
        m.cmd = CMD_SETRUN_MODE;
        if (oam_isMaster()) {
            m.v1u = runmode_master;  //runmode_normal; //runmode_testcan; // runmode_normal; runmode_detect2; runmode_off
            //m.v1u = runmode_normal;  // XXX temp, delete me
        } else {
            m.v1u = runmode_slave;
        }
        mqf_write_from_nowhere(&m); // from_nowher, otherwise it wont be sent to self
    }
    Oam_Ready = 1;
}


static void OAM_change_mode(runmode_t run_mode)
{
	if (run_mode == runmode_testcan) {
		respok = 0;
	}
}

static void _bcast_normal(void)
{
	msg_64_t m;
	m.from = MA3_BROADCAST;
	m.to = MA3_BROADCAST;
	m.cmd = CMD_SETRUN_MODE;
	m.v1u =  runmode_normal;
	mqf_write_from_nowhere(&m);
}
static void exit_can_test(void)
{
	_bcast_normal();
}

static void customOam(msg_64_t *m);



#if 0
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
            m.v1u = runmode_master;  //runmode_normal; //runmode_testcan; // runmode_normal; runmode_detect2; runmode_off
            //m.v1u = runmode_normal;  // XXX temp, delete me
        } else {
            m.v1u = runmode_slave;
        }
        mqf_write_from_nowhere(&m); // from_nowher, otherwise it wont be sent to self
	}


	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_oam(&m);
		if (rc) break;

        _UNUSED_ unsigned int instnum;
        _UNUSED_ unsigned int confnum;
        _UNUSED_ unsigned int fieldnum;
        _UNUSED_ unsigned int confbrd;
        _UNUSED_ int32_t v;
        _UNUSED_ uint64_t enc;

        int handled = 0;
        switch (m.cmd) {
        case CMD_SETRUN_MODE:
        	if (m.v1u != run_mode) {
        		run_mode = (runmode_t) m.v1u;
        		if (run_mode == runmode_testcan) {
        			respok = 0;
        		}
        	}
            handled = 1;
        	break;

        case CMD_OAM_CUSTOM:
                customOam(&m);
                handled = 1;
                break;
#ifdef BOARD_HAS_FLASH
        case CMD_PARAM_USER_SET:
        	if (!oam_isMaster()) {
        		itm_debug1(DBG_OAM|DBG_ERR, "only master recv set", 0);
                extern void Error_Handler(void);
        		Error_Handler();
        		break;
        	}

        	oam_decode_val40(m->val40, &confnum, &confbrd, &instnum, &fieldnum, &v);
        	// store in flash
        	oam_flashstore_set_value(confnum, fieldnum, confbrd, instnum, v);
        	if (confbrd == 0)  {
        		// master == local board
        		conf_propagate(confnum, fieldnum, instnum, v);
        	} else {
        		m->cmd = CMD_PARAM_PROPAG;
        		m->to = MA0_OAM(confbrd);
        		mqf_write_from_oam(&m);
        	}
            handled = 1;
        	break;
        case CMD_PARAM_USER_GET:
        	if (!oam_isMaster()) {
        		itm_debug1(DBG_OAM|DBG_ERR, "only master recv get", 0);
        		FatalError("NoMast", "only master should receive this", Error_NotMaster);
        		break;
        	}

        	oam_decode_val40(m->val40, &confnum, &confbrd, &instnum, &fieldnum, &v);
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
        	m->to = m->from;
        	m->from = MA0_OAM(0);
        	m->cmd = CMD_PARAM_USER_VAL;
            m->val40 = enc;
        	mqf_write_from_oam(&m);
            handled = 1;
        	break;


        case CMD_PARAM_LUSER_SET:
        	if (!oam_isMaster()) {
        		itm_debug1(DBG_OAM|DBG_ERR, "only master recv set", 0);
        		FatalError("NoMast", "only master should receive this", Error_NotMaster);
        		break;
        	}
        	oam_decode_val40(m->val40, &confnum, &confbrd, &instnum, &fieldnum, &v);

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
            handled = 1;
        	break;


        case CMD_PARAM_LUSER_COMMIT:
        	if (!oam_isMaster()) {
        		itm_debug1(DBG_OAM|DBG_ERR, "only master recv commit", 0);
        		FatalError("NoMast", "only master should receive this", Error_NotMaster);
        		break;
        	}
        	oam_flashlocal_commit(m->v1);
            handled = 1;
        	break;



        case CMD_PARAM_LUSER_GET:
        	if (!oam_isMaster()) {
        		itm_debug1(DBG_OAM|DBG_ERR, "only master recv get", 0);
        		FatalError("NoMast", "only master should receive this", Error_NotMaster);
        		break;
        	}
        	oam_decode_val40(m->val40, &confnum, &confbrd, &instnum, &fieldnum, &v);
        	v = oam_flashlocal_get_value(confnum, fieldnum, instnum);
        	oam_encode_val40(&enc, confnum, 0, instnum, fieldnum, v);
        	m->to = m->from;
        	m->from = MA0_OAM(0);
        	m->cmd = CMD_PARAM_LUSER_VAL;
            m->val40 = enc;
        	mqf_write_from_oam(&m);
            handled = 1;
#endif // BOARD_HAS_FLASH
        	break;

        case CMD_PARAM_PROPAG:
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
        	handled = 1;
        	break;


        default:
        	break;
        }

        if (!handled) {
        	if (runmode_testcan == run_mode) {
        		handle_testcan_msg(&m);
        	} else if (oam_isMaster()) {
        		handle_master_msg(&m);
        	} else {
        		handle_slave_msg(&m);
        	}
        }
	}
	switch (run_mode) {
	case runmode_testcan:
            handle_testcan_tick(tick);
            break;
        case runmode_master:
            handle_master_tick(tick);
            break;
        case runmode_slave:
            handle_slave_tick(tick);
            break;
	default:
		return;
	}
}
#endif


/*
 * ------------------------------------------------------------------
 * common message handling (in all modes) done by handle_msg_common
 * which calls :
 *   handle_msg_flash
 */

static int handle_msg_flash(msg_64_t *m)
{
#ifdef BOARD_HAS_FLASH
	unsigned int instnum;
	unsigned int confnum;
	unsigned int fieldnum;
	unsigned int confbrd;
	int32_t v;
	uint64_t enc;
#endif

    switch (m->cmd) {
#ifdef BOARD_HAS_FLASH
        case CMD_PARAM_USER_SET:
            if (!oam_isMaster()) {
                itm_debug1(DBG_OAM|DBG_ERR, "only master recv set", 0);
                extern void Error_Handler(void);
                Error_Handler();
                return 1;
                break;
            }

            oam_decode_val40(m->val40, &confnum, &confbrd, &instnum, &fieldnum, &v);
            // store in flash
            oam_flashstore_set_value(confnum, fieldnum, confbrd, instnum, v);
            if (confbrd == 0)  {
                // master == local board
                conf_propagate(confnum, fieldnum, instnum, v);
            } else {
                m->cmd = CMD_PARAM_PROPAG;
                m->to = MA0_OAM(confbrd);
                mqf_write_from_oam(m);
            }
            return 1;
            break;
        case CMD_PARAM_USER_GET:
            if (!oam_isMaster()) {
                itm_debug1(DBG_OAM|DBG_ERR, "only master recv get", 0);
                FatalError("NoMast", "only master should receive this", Error_NotMaster);
                return 1;
                break;
            }

            oam_decode_val40(m->val40, &confnum, &confbrd, &instnum, &fieldnum, &v);
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
            m->to = m->from;
            m->from = MA0_OAM(0);
            m->cmd = CMD_PARAM_USER_VAL;
            m->val40 = enc;
            mqf_write_from_oam(m);
            return 1;
            break;


        case CMD_PARAM_LUSER_SET:
            if (!oam_isMaster()) {
                itm_debug1(DBG_OAM|DBG_ERR, "only master recv set", 0);
                FatalError("NoMast", "only master should receive this", Error_NotMaster);
                return 1;
                break;
            }
            oam_decode_val40(m->val40, &confnum, &confbrd, &instnum, &fieldnum, &v);

            if (confnum == conf_lnum_globparam) {
                // special handling for some param
                if (fieldnum == conf_numfield_globparam_oscillo) {
#ifdef BOARD_HAS_OSCILLO
                    extern int oscillo_enable;
                    oscillo_enable = v;
#endif
                    return 1;
                    break;
                }
            }
            // store in flash
            oam_flashlocal_set_value(confnum, fieldnum,  instnum, v);
            return 1;
            break;


        case CMD_PARAM_LUSER_COMMIT:
            if (!oam_isMaster()) {
                itm_debug1(DBG_OAM|DBG_ERR, "only master recv commit", 0);
                FatalError("NoMast", "only master should receive this", Error_NotMaster);
                return 1;
                break;
            }
            oam_flashlocal_commit(m->v1);
            return 1;
            break;



        case CMD_PARAM_LUSER_GET:
            if (!oam_isMaster()) {
                itm_debug1(DBG_OAM|DBG_ERR, "only master recv get", 0);
                FatalError("NoMast", "only master should receive this", Error_NotMaster);
                return 1;
                break;
            }
            oam_decode_val40(m->val40, &confnum, &confbrd, &instnum, &fieldnum, &v);
            v = oam_flashlocal_get_value(confnum, fieldnum, instnum);
            oam_encode_val40(&enc, confnum, 0, instnum, fieldnum, v);
            m->to = m->from;
            m->from = MA0_OAM(0);
            m->cmd = CMD_PARAM_LUSER_VAL;
            m->val40 = enc;
            mqf_write_from_oam(m);
            return 1;
            break;
#else  // BOARD_HAS_FLASH
        case CMD_PARAM_USER_SET:     //FALLTHRU
        case CMD_PARAM_USER_GET:     //FALLTHRU
        case CMD_PARAM_LUSER_SET:    //FALLTHRU
        case CMD_PARAM_LUSER_GET:    //FALLTHRU
        case CMD_PARAM_LUSER_COMMIT:
            itm_debug1(DBG_ERR|DBG_OAM, "no flash", m->cmd);
            return 1;
#endif // BOARD_HAS_FLASH
        default:
            return  0;
    }
}

static int handle_msg_common(msg_64_t *m)
{
    if (handle_msg_flash(m)) return 1;
    switch (m->cmd) {
        case CMD_OAM_CUSTOM:
            customOam(m);
            return 1;
        default:
            return 0;
    }
}


static uint32_t lastBcast = 0;
enum oam_slv_state {
    oam_slv_bcast,
	oam_slv_unknown, // board is not known by master, continue bcast but with lower freq
	oam_slv_ok       // board has received number
};
static enum oam_slv_state slvState = oam_slv_bcast;



static void _send_slv_ok(void)
{
	msg_64_t m = {0};
	m.cmd = CMD_OAM_SLV_OK;
	m.from = MA0_OAM(oam_localBoardNum());
	m.to = MA0_OAM(0);
	m.v32u = oam_getDeviceUniqueId();
	mqf_write_from_oam(&m);
}

static uint16_t slave_master_id16 = 0;
static uint32_t slave_bootcount = 0;

static int _handle_msg_slave(msg_64_t *m)
{
	/*unsigned int instnum;
	unsigned int confnum;
	unsigned int fieldnum;
	unsigned int confbrd;
	int32_t v;
	uint64_t enc;*/
	itm_debug2(DBG_OAM, "om_slv", m->cmd, m->v1);

    switch (m->cmd) {
        case CMD_PARAM_PROPAG:
            if (oam_isMaster()) {
                itm_debug1(DBG_OAM|DBG_ERR, "only slave recv propag", 0);
                FatalError("NoSlv", "only slave should receive this", Error_NotSlave);
                return 1;
                break;
            }
            unsigned int instnum;
            unsigned int confnum;
            unsigned int fieldnum;
            unsigned int confbrd;
            int32_t v;
            oam_decode_val40(m->val40, &confnum, &confbrd, &instnum, &fieldnum, &v);
            conf_propagate(confnum, fieldnum, instnum, v);
            return 1;
            break;
            
        case CMD_OAM_BNUM:
        	oam_flash_led();
            if (oam_getDeviceUniqueId() != m->v32u) {
                itm_debug2(DBG_ERR|DBG_OAM, "bad uniq", m->v32u, oam_getDeviceUniqueId());
                return 1;
            }
            switch (slvState) {
            case oam_slv_ok:
            	if (m->subc != oam_localBoardNum()) {
            		itm_debug3(DBG_OAM|DBG_ERR, "BNUM/slvst2", slvState, m->subc, oam_localBoardNum());
            		// what to do here ?
            		return 1;
            	}
            	break;
            case oam_slv_bcast:   //FALLTHRU
            case oam_slv_unknown: //FALLTHRU
            default:
                itm_debug3(DBG_OAM, "BNUM/slvst", slvState, m->subc, oam_localBoardNum());
            }
            if (m->subc != 0xFF) {
                oam_localBoardNum_set(m->subc);
                slvState = oam_slv_ok;
                itm_debug2(DBG_OAM, "BNUM/ok",  m->subc, oam_localBoardNum());
                _send_slv_ok();
            } else {
                itm_debug2(DBG_OAM, "BNUM/unk",  m->subc, oam_localBoardNum());
                slvState = oam_slv_unknown;
            }
            return 1;
            break;

        case CMD_OAM_MASTER:
        	/*
        	 * m.v1u = oam_getDeviceUniqueId() & 0xFFFF;
			 * m.v2u = bootcount;
	    	 */
        	if (!slave_master_id16) {
        		// 1st msg
        		slave_master_id16 = m->v1u;
        		slave_bootcount = m->v2u;
        	} else {
        		int reboot = 0;
        		static int mcnt = 5;
        		if (slave_master_id16 == m->v1u) {
        			if (slave_bootcount != m->v2u) {
        				itm_debug3(DBG_ERR|DBG_OAM, "M/Reboot", slave_master_id16, slave_bootcount, m->v2u);
        				reboot = 1;
        			}
        		} else {
        			// multiple master, allow 5 bad master until resolution
    				itm_debug2(DBG_ERR|DBG_OAM, "M/MChg", slave_master_id16, m->v1u);
    				if (!mcnt) {
    					reboot = 1;
    				}  else {
    					mcnt--;
    				}
        		}
        		if (reboot) {
        			// reboot slave when master changed or when master did reboot
    				itm_debug1(DBG_ERR|DBG_OAM, "RESET", 0);
#ifndef TRAIN_SIMU
        			HAL_NVIC_SystemReset();
#endif
        		}
        	}
        	return 1;
        	break;
        default:
            return 0;
            break;
    }
}
static void handle_msg_slave(msg_64_t *m)
{
    if (_handle_msg_slave(m)) return;
    handle_msg_common(m);
}



enum slave_state {
	slave_unconfigured,
	slave_notpreset,
	slave_seen,
	slave_config0,
	slave_config,
	slave_ok,
};

static enum slave_state slv_state[16] = { slave_notpreset };

static int _handle_msg_master(msg_64_t *m)
{
    int bnum;
    uint8_t unum;
    switch (m->cmd) {
    default:
        return 0;
        break;
    case CMD_OAM_SLAVE:
        bnum = oam_boardForUuid(m->v32u);
        if (bnum == -1) {
            itm_debug1(DBG_OAM, "SLV/unk", m->v32u);
            // board unknown
            // TODO notif UI
            unum = 0xFF;
        } else {
            if (bnum<=0) FatalError("nBN", "null bnum", Error_NumBnum);
            if (bnum>15) FatalError("hBN", "15 bnum", Error_NumBnum);
            slv_state[bnum] = slave_seen;
            unum = (uint8_t) bnum;
        }
        itm_debug2(DBG_OAM, "SLV/k", bnum, m->v32u);
        msg_64_t r = {0};
        r.cmd = CMD_OAM_BNUM;
        r.subc = unum;
        r.from = MA0_OAM(0);
        r.to = MA3_SLV_OAM;
        r.v32u = m->v32u;
        mqf_write_from_oam(&r);
        return 1;
        break;
    case CMD_OAM_SLV_OK:
        unum = MA0_BOARD(m->from);
        if (unum > 15) FatalError("hrBN", "15 rbnum", Error_NumBnum2);
        slv_state[unum] = slave_config0;
        itm_debug1(DBG_OAM, "SLV/ok", unum);
        return 1;
        break;
    }
}

static void handle_msg_master(msg_64_t *m)
{
    if (_handle_msg_master(m)) return;
    handle_msg_common(m);
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
		if (oam_isMaster()) break;
		m->cmd = CMD_CANTEST_RESP;
		m->v2u = m->v1u*2;
		mqf_write_from_oam(m);
		break;

	case CMD_CANTEST_RESP:
		// also handled by IHM
		respok++;
		if (respok > 20) {
			respok = 0;
			exit_can_test();
		}

		break;
	}
}
    
    
static void handle_testcan_tick(uint32_t tick, _UNUSED_ uint32_t dt)
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
	case 99:
#ifdef BOARD_HAS_FLASH
		oam_flash_erase();
#endif
		break;
	}
}


// --------------------------------------------------------------

/* O&M slave handling */

static void handle_slave_tick(uint32_t tick, _UNUSED_ uint32_t dt)
{
	int tbc = 200;
	switch (slvState) {
	case oam_slv_bcast: tbc = 200; break;
	case oam_slv_unknown: tbc = 2000; break;
	default: tbc = 0; break;
	}
    if (tbc) {
        if (tick > lastBcast+tbc) {
        	//oam_flash_led();
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
/*
static void handle_slave_msg(msg_64_t *m)
{
	switch (m->cmd) {
	default: break;
	case CMD_OAM_BNUM:
		if (oam_getDeviceUniqueId() != m->v32u) {
			itm_debug2(DBG_ERR|DBG_OAM, "bad uniq", m->v32u, oam_getDeviceUniqueId());
			return;
		}
		switch (slvState) {
		case oam_slv_bcast:   break;
		case oam_slv_unknown: break;
		default:
			itm_debug3(DBG_OAM, "BNUM/slvst", slvState, m->subc, oam_localBoardNum());
			if (m->subc != oam_localBoardNum()) {
				itm_debug3(DBG_OAM|DBG_ERR, "BNUM/slvst2", slvState, m->subc, oam_localBoardNum());
				// what to do here ?
				return;
			}
			_send_slv_ok();
			return;
		}
		if (m->subc != 0xFF) {
			oam_localBoardNum_set(m->subc);
			slvState = oam_slv_ok;
			_send_slv_ok();
		} else {
			slvState = oam_slv_unknown;
		}
		break;

	}
}
 */
// --------------------------------------------------------------

/*
static void handle_master_msg(msg_64_t *m)
{
	int bnum;
	uint8_t unum;
	switch (m->cmd) {
	default:
		itm_debug1(DBG_OAM|DBG_ERR, "oam unk msg",m->cmd);
		break;
	case CMD_OAM_SLAVE:
		bnum = oam_boardForUuid(m->v32u);
		if (bnum == -1) {
			itm_debug1(DBG_OAM, "SLV/unk", m->v32u);
			// board unknown
			// TODO notif UI
			unum = 0xFF;
		} else {
			if (bnum<=0) FatalError("nBN", "null bnum", Error_NumBnum);
			if (bnum>15) FatalError("hBN", "15 bnum", Error_NumBnum);
			slv_state[bnum] = slave_seen;
			unum = (uint8_t) bnum;
		}
		itm_debug2(DBG_OAM, "SLV/k", bnum, m->v32u);
		msg_64_t r = {0};
		r.cmd = CMD_OAM_BNUM;
		r.from = MA0_OAM(0);
		r.to = MA3_SLV_OAM;
		r.v32u = m->v32u;
		mqf_write_from_oam(&r);
		break;
	case CMD_OAM_SLV_OK:
		unum = MA0_BOARD(m->from);
		if (unum > 15) FatalError("hrBN", "15 rbnum", Error_NumBnum2);
		slv_state[unum] = slave_config;
		break;
	}
}
 */


static void handle_master_tick(_UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	static uint32_t lbsc = 0;
	static int initial = 1;
	uint32_t tempo = initial ? 100 : 2000;
	if (tick>lbsc+tempo) {
		lbsc = tick;
		static int cnt = 0;
		if (initial && (cnt>100)) {
			initial=0;
			_bcast_normal();
			return;
		}
		cnt++;
		msg_64_t m = {0};
		itm_debug2(DBG_OAM, "send MST", tick, CMD_OAM_MASTER);
		m.cmd = CMD_OAM_MASTER;
		m.v1u = oam_getDeviceUniqueId() & 0xFFFF;
		m.v2u = bootcount;
		m.from = MA0_OAM(0);
		m.to = MA3_SLV_OAM;
    	mqf_write_from_oam(&m);
	}
#ifdef BOARD_HAS_FLASH
	int rc = 0;
	unsigned int confnum; unsigned int fieldnum;
	unsigned int confbrd; unsigned int instnum;
	int32_t v;

	for (unsigned int b = 1; b<16; b++) {
		switch (slv_state[b]) {
		default: continue;
		case slave_config0:
			oam_flashstore_rd_rewind();
			slv_state[b] = slave_config;
			// do NOT continue loop for board because
			// oam_flashstore_rd_rewind()/read() handles only one ptr
			// and thus we configure one board at a time
			OAM_NeedsReschedule = 1;
			return;
		case slave_config:
			// oam_flashstore_rd_rewind();
			for (;;) {
				rc = oam_flashstore_rd_next(&confnum, &fieldnum, &confbrd, &instnum, &v);
				if (rc<0) {
					// EOF
					// this time we can loop on board
					break;
				}
				if (confbrd != b) continue;
				break;
			}
			if (rc<0) {
				itm_debug1(DBG_OAM, "SLVpok", b);
				slv_state[b] = slave_ok;
				OAM_NeedsReschedule = 0;
				continue;
			}
			itm_debug3(DBG_OAM, "SLVprpg", b, confnum, fieldnum);
			msg_64_t m = {0};
			m.to = MA0_OAM(b);
			m.from = MA0_OAM(0);
			m.cmd = CMD_PARAM_PROPAG;
			uint64_t enc;
    		oam_encode_val40(&enc, confnum, confbrd, instnum, fieldnum, v);
            m.val40 = enc;
        	mqf_write_from_oam(&m);
			return;
		}
	}
#else
	FatalError("NoFl", "no flash on master", Error_NoFlash);
#endif
}

// --------------------------------------------------------------
// --------------------------------------------------------------
