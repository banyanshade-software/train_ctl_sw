/*
 * ihm.c
 *
 *  Created on: May 12, 2021
 *      Author: danielbraun
 */

#include <stdint.h>

#include "trainctl_config.h"

#include "ihm.h"
#include "disp.h"
#include "../utils/itm_debug.h"
#include "../msg/trainmsg.h"

#ifndef TRAIN_SIMU
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#else
#include "stm32f1xx_hal.h"
#error not tested
#endif
#else
#error should not be used in simu
#include "train_simu.h"
#endif

static uint16_t rot0_position=0xFFFF;
extern TIM_HandleTypeDef htim4;

static uint8_t needsrefresh_mask;

#define SET_NEEDSREFRESH(_i) do { needsrefresh_mask = (needsrefresh_mask | (1<<(_i)));} while(0)
#define NEEDSREFRESH(_i) ((needsrefresh_mask & (1<<(_i))) ? 1 : 0)

// ----------------------------------------------------------------

static uint16_t get_rotary(TIM_HandleTypeDef *ptdef)
{
	uint16_t p = __HAL_TIM_GET_COUNTER(ptdef);
	if (p>0x7FFF) {
		p = 0;
		__HAL_TIM_SET_COUNTER(ptdef, p);
	} else if (p>=200) {
		p=200;
		__HAL_TIM_SET_COUNTER(ptdef, p);
	}
	return p>>1;
}


// -----------------------------------------------------------------------

static void ui_process_msg(void);

void ihm_runtick(void)
{
	static int cnt=0;
	needsrefresh_mask = 0;
	// scan rotary encoder -----------
	uint16_t p = get_rotary(&htim4);
	if (p != rot0_position) {
		// pos changed
		rot0_position = p;
		if (1/*rot0 is displayed*/) {
			ihm_setvar(0, 0, rot0_position);
			ihm_setvar(0, 1, rot0_position - 50);
			SET_NEEDSREFRESH(0);
		}
	}
	// scan buttons ------------------

	// process messages --------------
	ui_process_msg();

	if ((0)) {
		ihm_setvar(0, 0, cnt);
		ihm_setvar(0, 1, -cnt);
		SET_NEEDSREFRESH(0);
		cnt++;
	}
	// update displays ---------------
	for (int i=0; i<MAX_DISP; i++) {
		if (NEEDSREFRESH(i)) {
			disp_layout(i);
		}
	}
}

static int test_mode=0;

static void ui_process_msg(void)
{
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_ui(&m);
		if (rc) break;
		if (IS_CONTROL_T(m.from) || IS_TRAIN_SC(m.from)) {
			static char t[3] = "Tx";
			t[1] = (m.from & 0x7) + '0';
			//ui_write_status(0, t);
		} else if (IS_CANTON(m.from)) {
			static char t[] = "Blk--";
			t[4] = (m.from & 0x7) + '0';
			t[3] = (MA_2_BOARD(m.from))+'0';
			//ui_write_status(0, t);
		} else if (IS_TURNOUT(m.from)) {
			static char t[] = "Trn--";
			t[4] = (m.from & 0x7) + '0';
			t[3] = (MA_2_BOARD(m.from))+'0';
			//ui_write_status(0, t);
		} else {
			//ui_write_status(0, "...");
		}
		switch(m.cmd) {
        case CMD_TEST_MODE:
            test_mode = m.v1u;
            //ui_write_mode(0);
    		//ui_msg5(0, "T");
            break;
        case CMD_SETVPWM:
        	//if (test_mode) ui_canton_pwm(m.from, m.v1u, m.v2);
        	break;
        }
		if (IS_UI(m.to)) {
			int dn = m.to & 0x1F;
			if (dn != 1) {
				itm_debug1(DBG_UI, "?dn", dn);
				continue;
			}
			switch (m.cmd) {
			case CMD_UI_MSG:
				//ui_msg5(dn, (char *) m.rbytes+1);
				break;
			default:
				itm_debug1(DBG_UI, "cmd?", m.cmd);
				break;
			}
		} else {
			itm_debug1(DBG_UI, "non ui msg", 0);
		}
	}
}


// ---------------------------------
