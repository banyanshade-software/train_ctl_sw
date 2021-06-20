/*
 * ihm.c
 *
 *  Created on: May 12, 2021
 *      Author: danielbraun
 */

#include <stdint.h>

#include "trainctl_config.h"


#include <stddef.h>
#include "misc.h"

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

#include "../stm32dev/ina3221/ina3221.h"

#include "railconfig.h"

#define MAX_ROTARY MAX_DISP
#define UNSIGNED_ROT 0

#if UNSIGNED_ROT
static uint16_t rot_position[MAX_ROTARY]={0xFFFF};
#else
static int16_t rot_position[MAX_ROTARY]={0x7FFF};
#endif
static uint8_t  drive_mode[MAX_ROTARY]={1};
extern TIM_HandleTypeDef htim4;

static uint8_t needsrefresh_mask;

#define SET_NEEDSREFRESH(_i) do { needsrefresh_mask = (needsrefresh_mask | (1<<(_i)));} while(0)
#define NEEDSREFRESH(_i) ((needsrefresh_mask & (1<<(_i))) ? 1 : 0)

// ----------------------------------------------------------------

#define ENC_MUL2  		1
#define ENC_DIV2	 	0
#define ENC_MAX ((100<<ENC_DIV2)>>ENC_MUL2)
#define MIDDLE_ZERO 4

_UNUSED_ static uint16_t get_rotary(TIM_HandleTypeDef *ptdef)
{
	uint16_t p = __HAL_TIM_GET_COUNTER(ptdef);
	if (p>0x7FFF) {
		p = 0;
		__HAL_TIM_SET_COUNTER(ptdef, p);
	} else if (p>=ENC_MAX) {
		p=ENC_MAX;
		__HAL_TIM_SET_COUNTER(ptdef, p);
	}
	return ((p<<ENC_MUL2)>>ENC_DIV2);//>>1;
}


static int16_t get_srotary(TIM_HandleTypeDef *ptdef)
{
	int16_t p = __HAL_TIM_GET_COUNTER(ptdef);
	if (p<-ENC_MAX-MIDDLE_ZERO) {
		p = -ENC_MAX-MIDDLE_ZERO;
		__HAL_TIM_SET_COUNTER(ptdef, p);
	} else if (p>=ENC_MAX+MIDDLE_ZERO) {
		p=ENC_MAX+MIDDLE_ZERO;
		__HAL_TIM_SET_COUNTER(ptdef, p);
	}
	if (abs(p)<MIDDLE_ZERO) p=0;
	else if (p>0) p=p-MIDDLE_ZERO;
	else p=p+MIDDLE_ZERO;
	return ((p<<ENC_MUL2)>>ENC_DIV2);//>>1;
}

// -----------------------------------------------------------------------

static void ui_process_msg(void);

static int ihm_mode = 0;

void ihm_runtick(void)
{
	static int cnt=0;
	static int first = 0;
	if (!first) {
		first = 1;
		itm_debug1(DBG_UI, "UI init", 0);
		switch(ihm_mode) {
		case 0:
			ihm_setlayout(0, LAYOUT_MANUAL);
			break;
		case 1:
			ihm_setlayout(0, LAYOUT_INA3221_DETECT);
			break;
		case 2:
			ihm_setlayout(0, LAYOUT_INA3221_VAL);
			break;
		}
		for (int i = 0; i<DISP_MAX_REGS; i++) {
			ihm_setvar(0, i, 0);
		}
	}
	itm_debug1(DBG_UI, "UI tick", 0);

	needsrefresh_mask = 0;
	// scan rotary encoder -----------
	for (int i=0; i<MAX_ROTARY; i++) {
#if UNSIGNED_ROT
		uint16_t p = get_rotary(&htim4);
		if (p != rot_position[i]) {
			// pos changed
			rot_position[i] = p;
			if (ihm_mode==0) {
				ihm_setvar(0, 1, rot_position[0]);
				//ihm_setvar(0, 1, ((int)rot0_position - 50));
				SET_NEEDSREFRESH(0);
			}
			if (drive_mode[i]) {
				msg_64_t m;
				m.from = MA_UI(i);
				m.to = MA_CONTROL_T(i);
				m.cmd = CMD_MDRIVE_SPEED;
				m.v1u = rot_position[i];
				mqf_write_from_ui(&m);
			}
		}
#else
		int16_t p = get_srotary(&htim4);
		if (p != rot_position[i]) {
			// pos changed
			rot_position[i] = p;
			if (ihm_mode==0) {
				ihm_setvar(0, 1, (uint16_t) rot_position[0]);
				//ihm_setvar(0, 1, ((int)rot0_position - 50));
				SET_NEEDSREFRESH(0);
			}
			if (drive_mode[i]) {
				msg_64_t m;
				m.from = MA_UI(i);
				m.to = MA_CONTROL_T(i);
				m.cmd = CMD_MDRIVE_SPEED_DIR;
				m.v1u = abs(rot_position[i]);
				m.v2 = SIGNOF0(rot_position[i]);
				// TODO handle dir
				mqf_write_from_ui(&m);
			}
		}
#endif

	}

	// scan buttons ------------------

	// mode test hook
	if (ihm_mode==1) {
		// ina3221 detection
		for (int i=0; i<4; i++) {
			ihm_setvar(0, i, ina3221_devices[i]);
		}
		SET_NEEDSREFRESH(0);
	} else if (ihm_mode==2) {

	}
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

		if (m.cmd == CMD_TRTSPD_NOTIF) {
			itm_debug1(DBG_UI, "hop", 0);
		}

		switch(m.cmd) {
        case CMD_TEST_MODE:
            test_mode = m.v1u;
            //ui_write_mode(0);
    		//ui_msg5(0, "T");
            return;
            break;
        case CMD_SETVPWM:	// TODO remove
        	//if (test_mode) ui_canton_pwm(m.from, m.v1u, m.v2);
        	return;
        	break;
        case CMD_VOFF_NOTIF:
        	if (NOTIF_VOFF && (ihm_mode == 0)) {
        		ihm_setvar(0, 2, m.v1/2);
        		SET_NEEDSREFRESH(0);
        	}
        	return;
        	break;
        }
		if (IS_CONTROL_T(m.from)) {
			int trnum = m.from & 0x07;
			switch (m.cmd) {
			case CMD_TRSTATUS_NOTIF:
				// TODO trnum -> display num
				if (ihm_mode == 0) {
					//TODO
					//ihm_setvar(0, 2, m.v1u);
					SET_NEEDSREFRESH(0);
				}
				return;
				break;
			case CMD_TRTSPD_NOTIF:
				itm_debug2(DBG_UI|DBG_CTRL, "rx tspd notif", trnum, m.v1u);
				// TODO trnum -> display num
				if (!NOTIF_VOFF && (ihm_mode == 0)) {
					ihm_setvar(0, 2, m.v2 * m.v1u);
					SET_NEEDSREFRESH(0);
				}
				return;
				break;

			case CMD_TRDIR_NOTIF:
				//TODO
				return;
				break;
			case CMD_TRMODE_NOTIF:
				// TODO
				return;
				break;
			case CMD_UI_MSG:
				break; // see below
			default:
				itm_debug1(DBG_UI, "unk ctl", m.cmd);
				return;
				break;
			}
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
			case CMD_INA3221_REPORT:
				if (ihm_mode == 2) {
					int16_t *values = (int16_t *) m.v32u;
					for (int i =0; i<12; i++) {
						ihm_setvar(0, i, values[i]);
					}
					SET_NEEDSREFRESH(0);
				}
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
