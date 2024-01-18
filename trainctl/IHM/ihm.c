/*
 * ihm.c
 *
 *  Created on: May 12, 2021
 *      Author: danielbraun
 */

// #define RECORD_MSG 1

#include <stdint.h>

#include "trainctl_config.h"


#include <stddef.h>
#include "../misc.h"

#include "ihm.h"
#include "disp.h"
#include "../utils/itm_debug.h"
#include "../msg/trainmsg.h"

#include "button.h"
#include "../ctrl/ctrl.h"
#include "screen.h"


#ifndef BOARD_HAS_IHM
#error BOARD_HAS_IHM not defined, remove this file from build
#endif



#ifndef TRAIN_SIMU



#if defined(STM32F4)
#include "stm32f4xx_hal.h"

#elif defined(STM32G4)
#include "stm32g4xx_hal.h"

#elif defined(STM32F1)
#include "stm32f1xx_hal.h"

#else
#error no board hal
#endif




#else
#error should not be used in simu
#include "train_simu.h"
#endif

#include "../ctrl/ctrl.h"
#ifdef BOARD_HAS_INA3221
#include "../stm32/ina3221/ina3221.h"
#endif


#ifdef BOARD_HAS_ROTARY_ENCODER
#define MAX_ROTARY MAX_DISP
#define UNSIGNED_ROT 0

#if UNSIGNED_ROT
static uint16_t rot_position[MAX_ROTARY]={0xFFFF};
#else
static int16_t rot_position[MAX_ROTARY]={0x7FFF};
#endif
static uint8_t  drive_mode[MAX_ROTARY]={1};
extern TIM_HandleTypeDef htim4;

#endif // BOARD_HAS_ROTARY_ENCODER

// ----------------------------------------------------------------

static void ihm_enter_runmode(runmode_t m);
static void ihm_init(void);
static void ihm_postmsg_tick(uint32_t, uint32_t);
static void ihm_handle_inputs(uint32_t, uint32_t);

static msg_handler_t msghandler_for_mode(runmode_t m);

static const tasklet_def_t ihm_tdef = {
		.init 				= ihm_init,
		.poll_divisor		= NULL,
		.emergency_stop 	= NULL,
		.enter_runmode		= ihm_enter_runmode,
		.pre_tick_handler	= ihm_handle_inputs,
		.default_msg_handler = NULL,
		.default_tick_handler = ihm_postmsg_tick,
		.msg_handler_for	= msghandler_for_mode,
		.tick_handler_for 	= NULL,

		.recordmsg			= RECORD_MSG,

};
tasklet_t ihm_tasklet = { .def = &ihm_tdef, .init_done = 0, .queue=&to_ui};

static int  ihmmsg_common(msg_64_t *m);
static void ihmmsg_off(msg_64_t *m);
static void ihmmsg_normal(msg_64_t *m);
static void ihmmsg_master(msg_64_t *m);
static void ihmmsg_slave(msg_64_t *m);
static void ihmmsg_testcan(msg_64_t *m);
static void ihmmsg_testcanton(msg_64_t *m);
static void ihmmsg_detect2(msg_64_t *m);

static msg_handler_t msghandler_for_mode(runmode_t m)
{
	switch (m) {
	default:	//FALLTHRU
	case runmode_off: 		return ihmmsg_off;
	case runmode_normal:	return ihmmsg_normal;
	case runmode_master:	return ihmmsg_master;
	case runmode_slave:		return ihmmsg_slave;
	case runmode_testcan:	return ihmmsg_testcan;
	case runmode_testcanton:return ihmmsg_testcanton;
	case runmode_detect2:	return ihmmsg_detect2;
	}
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------

static ihm_disp_state_t ihmdisp_state[MAX_DISP];

// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

//static train_mode_t curtrainmode[MAX_DISP] = {0};

static void ihm_init(void)
{
	itm_debug1(DBG_UI, "UI init", 0);
	memset(ihmdisp_state, 0, sizeof(ihmdisp_state));

	// xxx set_dispmode(mode_init);
	for (int d=0; d < MAX_DISP; d++) {
		for (int i = 0; i<DISP_MAX_REGS; i++) {
			disp_setvar(0, i, 0);
		}
		ihmdisp_state[d].screen = &ihm_screen_init;
		ihm_screen_set_train_mode(&ihmdisp_state[d], train_notrunning);
		disp_setlayout(d, ihmdisp_state[d].screen->layout);
		disp_layout(d);

	}
}

// ----------------------------------------------------------------


static uint8_t needsrefresh_mask = 0;

#define SET_NEEDSREFRESH(_i) do { needsrefresh_mask = (needsrefresh_mask | (1<<(_i)));} while(0)
#define NEEDSREFRESH(_i) ((needsrefresh_mask & (1<<(_i))) ? 1 : 0)



static void ihm_postmsg_tick(_UNUSED_ uint32_t t, _UNUSED_ uint32_t dt)
{
	for (int i=0; i<MAX_DISP; i++) {
		int rc = ihm_screen_handle(&ihmdisp_state[i]);
		if (rc) {
			disp_setlayout(i, ihmdisp_state[i].screen->layout);
			SET_NEEDSREFRESH(i);
		}
		if (NEEDSREFRESH(i)) {
			disp_layout(i);
		}
	}
	needsrefresh_mask = 0;
}
// ----------------------------------------------------------------


static void ihm_enter_runmode(runmode_t m)
{
	for (int i = 0; i<DISP_MAX_REGS; i++) {
		disp_setvar(0, i, 0);
	}
	const screen_t *s = NULL;
	switch (m) {
	default: //FALLTHRU
	case runmode_off: 				s = &ihm_screen_off;  break;
	case runmode_normal:			s = &ihm_screen_train_off; break;
	case runmode_detect2:			s = &ihm_screen_detect2; break;
	case runmode_detect_experiment: s = &ihm_screen_detect2; break;
	case runmode_master:			s = &ihm_screen_master; break;
	case runmode_slave:				s = &ihm_screen_slave; break;
	case runmode_testcan:			s = &ihm_screen_testcan; break;
	case runmode_testcanton:		s = &ihm_screen_testcanton; break;
	/*LAYOUT_DEFAULT
	LAYOUT_INA3221_DETECT
	LAYOUT_INA3221_VAL
	LAYOUT_MANUAL
	LAYOUT_AUTO*/
	}
	if (s) {
		ihmdisp_state[0].screen = s;
		disp_setlayout(0, ihmdisp_state[0].screen->layout);
		SET_NEEDSREFRESH(0);
	}
	// TODO : other display
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------



static int  ihmmsg_common(msg_64_t *m)
{
	int tn;
	switch (m->cmd) {
	default : return 0;

	case CMD_TRMODE_NOTIF:
			// TODO
			tn = MA1_TRAIN(m->from);
			if (tn >= MAX_DISP) break;
			ihm_screen_set_train_mode(&ihmdisp_state[tn], (train_mode_t) m->v1u);
			break;
			/*
			if (!is_special_dispmode()) {
				train_mode_t cm = (train_mode_t) m->v1u;
				switch (cm) {
				default:
				case train_notrunning:
					set_dispmode(mode_init);
					break;
				case train_manual:
				case train_fullmanual:
					set_dispmode(mode_manual);
					break;
				case train_auto:
					set_dispmode(mode_auto);
					break;
				}
			}*/
	}
	return 0;
}
static void ihmmsg_off(msg_64_t *m)
{
	if (ihmmsg_common(m)) return;
	// ...
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------


static void ihmmsg_master(_UNUSED_ msg_64_t *m)
{
	if (ihmmsg_common(m)) return;
}
static void ihmmsg_slave(_UNUSED_ msg_64_t *m)
{
	if (ihmmsg_common(m)) return;
}
static void ihmmsg_testcan(msg_64_t *m)
{
	if (ihmmsg_common(m)) return;

	switch(m->cmd) {
    case CMD_CANTEST:
    	disp_setvar(0, 0, m->v1u);
    	SET_NEEDSREFRESH(0);
    	break;
    case CMD_CANTEST_RESP:
    	disp_setvar(0, 1, m->v1u);
    	disp_setvar(0, 2, m->v2u);
    	SET_NEEDSREFRESH(0);
    	break;

    default:
    	itm_debug1(DBG_ERR|DBG_UI, "unhd msg1", m->cmd);
    	break;
	}
}


static void ihmmsg_testcanton(msg_64_t *m)
{
	if (ihmmsg_common(m)) return;

	switch(m->cmd) {

	case CMD_BEMF_NOTIF:
		itm_debug3(DBG_UI, "BEMF", m->subc, m->v1, m->v2);
		int32_t voff = m->v1;
		int32_t von = m->v2;
		disp_setvar(0, 0, m->subc);
		disp_setvar(0, 1, von);
		disp_setvar(0, 2, voff);
		SET_NEEDSREFRESH(0);
		break;
	case CMD_SETVPWM:
		// forwarded by canton for displau
		itm_debug2(DBG_ERR|DBG_UI, "pvm", m->cmd, m->subc);
		// we should check m->subc but we dont
		disp_setvar(0, 3, m->v1u);	// volt idx
		disp_setvar(0, 4, m->v2);		// pwm
		break;
	default:
		itm_debug1(DBG_ERR|DBG_UI, "unhd msg2", m->cmd);
		break;
	}
}

static void ihmmsg_detect2(msg_64_t *m)
{
	if (ihmmsg_common(m)) return;

	switch(m->cmd) {


	case CMD_UI_DETECT:
		if (m->v1 >= 0) {
			disp_setvar(0, 0, m->v1u);
			disp_setvar(0, 1, m->v2u);
		} else {
			disp_setvar(0, 0, 999);
		}
		SET_NEEDSREFRESH(0);
		break;

		/*
        case CMD_BEMF_NOTIF:
    		disp_setvar(0, 5, m->v2); //Von
    		disp_setvar(0, 6, m->v1); //Voff
    		SET_NEEDSREFRESH(0);
    		break;
        case CMD_INA3221_VAL1:
        	ihm_setvar(0, 7, m->v1);
        	SET_NEEDSREFRESH(0);
        	break;
		 */

	default:
		itm_debug1(DBG_ERR|DBG_UI, "unhd msg3", m->cmd);
		break;
	}
}

/*
typedef enum {
	mode_init = 0,
	mode_ina_detect,
	mode_ina_val,
	mode_manual,
	mode_auto
} ihm_mode_t;

// TODO : change this for per display struct
static ihm_mode_t ihm_dispmode = mode_init;
//static int ihm_train = 0;

static void set_displayout(void)
{
	switch (ihm_dispmode) {
	default:
	case mode_init: 	  	ihm_setlayout(0, LAYOUT_INIT); break;
	case mode_ina_detect:	ihm_setlayout(0, LAYOUT_INA3221_DETECT); break;
	case mode_ina_val:		ihm_setlayout(0, LAYOUT_INA3221_VAL); break;

	case mode_manual:		ihm_setlayout(0, LAYOUT_MANUAL); break;
	case mode_auto: 		ihm_setlayout(0, LAYOUT_AUTO); break;
	}
	SET_NEEDSREFRESH(0);
}

static int is_special_dispmode(void)
{
	switch (ihm_dispmode) {
	default:
	case mode_init: 	  	return 0;
	case mode_ina_detect:	return 1;
	case mode_ina_val:		return 1;

	case mode_manual:		return 0;
	case mode_auto: 		return 0;
	}
}

static void set_dispmode(ihm_mode_t m)
{
	if (ihm_dispmode == m) return;
	ihm_dispmode = m;
	set_displayout();
}
*/

static void ihmmsg_normal(msg_64_t *m)
{
	// handle CMD_TRMODE_NOTIF explicitly
	if (ihmmsg_common(m)) return;

	switch(m->cmd) {
	default:
		break;

	case CMD_SETVPWM:	// TODO remove
		//if (test_mode) ui_canton_pwm(m->from, m->v1u, m->v2);
		return;
		break;
	case CMD_VOFF_NOTIF:
		if (NOTIF_VOFF /*&& (!is_special_dispmode())*/) {
			disp_setvar(0, 2, m->v1/2);
			SET_NEEDSREFRESH(0);
		}
		return;
		break;
	}
	if (MA1_IS_CTRL(m->from)) {
		int trnum = MA1_TRAIN(m->from);
		if (trnum != 0) return; // TODO here we only display train 0
		switch (m->cmd) {
		case CMD_TRTSPD_NOTIF:
			itm_debug2(DBG_UI|DBG_CTRL, "rx tspd notif", trnum, m->v1u);
			// TODO trnum -> display num
			if (!NOTIF_VOFF /*&& ((ihm_dispmode == mode_manual) || (ihm_dispmode == mode_auto))*/) {
				disp_setvar(0, 2, m->v2 * m->v1u);
				SET_NEEDSREFRESH(0);
			}
			return;
			break;



		case CMD_TRSTATE_NOTIF:
			if (/*!is_special_dispmode()*/(1)) {
				disp_setvar(0, 3, 10+m->v1u);
				SET_NEEDSREFRESH(0);
			}
			return;
			break;
			//case CMD_UI_MSG:
			//	break; // see below
		default:
			itm_debug1(DBG_UI, "unk ctl", m->cmd);
			return;
			break;
		}
	}
	if (MA3_UI_GEN == m->to) {
		int dn = m->to & 0x1F;
		if (dn != 1) {
			itm_debug1(DBG_UI, "?dn", dn);
			return;
		}
		switch (m->cmd) {
		//case CMD_UI_MSG:
		//	//ui_msg5(dn, (char *) m->rbytes+1);
		break;
		/*case CMD_INA3221_REPORT:
			if (ihm_dispmode == mode_ina_val) {
				int16_t *values = (int16_t *) m->v32u;
				for (int i =0; i<12; i++) {
					ihm_setvar(0, i, values[i]);
				}
				SET_NEEDSREFRESH(0);
			}
			break;
		 */
		default:
			itm_debug1(DBG_UI, "cmd?", m->cmd);
			break;
		}
	} else {
		itm_debug1(DBG_UI, "non ui msg", 0);
	}
}




// ----------------------------------------------------------------
// ----------------------------------------------------------------

void local_ui_fatal(void)
{
	disp_setlayout(0, LAYOUT_FATAL);
	disp_layout(0);
	//Error_Handler();
	//for (;;);
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------


#ifdef BOARD_HAS_ROTARY_ENCODER

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

#endif // BOARD_HAS_ROTARY_ENCODER

#ifdef BOARD_HAS_TWO_BUTTONS
static const ihm_button_def_t b1def = { .port = ROT1_GPIO_Port, .pin = ROT1_Pin};
static ihm_button_t button1 = { .def = &b1def };
static const ihm_button_def_t b2def = { .port = ROT2_GPIO_Port, .pin = ROT2_Pin};
static ihm_button_t button2 = { .def = &b2def };

#endif


static void ihm_handle_inputs(_UNUSED_ uint32_t t, _UNUSED_ uint32_t dt)
{
#ifdef BOARD_HAS_ROTARY_ENCODER
	// scan rotary encoder -----------
	for (int i=0; i<MAX_ROTARY; i++) {
#if UNSIGNED_ROT
		// obsolete and untested now
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
		// unmaintained too, rotary display is not ok for speed control
		int16_t p = get_srotary(&htim4);
		if (p != rot_position[i]) {
			// pos changed
			rot_position[i] = p;
			//if (ihm_dispmode==mode_manual) {
			disp_setvar(0, 1, (uint16_t) rot_position[0]);
			SET_NEEDSREFRESH(0);
			//}
			if (drive_mode[i]) {	// TODO refactor drive_mode
				msg_64_t m;
				m.from = MA3_UI_GEN;//(i);
				m.to = MA1_CTRL(i);
				m.cmd = CMD_MDRIVE_SPEED_DIR;
				m.v1u = abs(rot_position[i]);
				m.v2 = SIGNOF0(rot_position[i]);
				// TODO handle dir
				mqf_write_from_ui(&m);
			}
		}
#endif
	}
#endif //  BOARD_HAS_ROTARY_ENCODER
#ifdef BOARD_HAS_TWO_BUTTONS
	int bt1 = ihm_poll_button(&button1, t);
	int bt2 = ihm_poll_button(&button2, t);
	// button1 and button2 are linked to display 0
	// TODO : a more generic button->display mapping
	if (bt1) ihmdisp_state[0].pending_events |= IHM_EVT_BUTTON_A;
	if (bt2) ihmdisp_state[0].pending_events |= IHM_EVT_BUTTON_B;
	if (bt1 || bt2) {
		itm_debug2(DBG_UI, "button", bt1, bt2);
	}

#endif
}




#if 0
static void ihm_runtick_normal(int);
static void ihm_runtick_off(int);
//static void ihm_runtick_detect(int);
static void ihm_runtick_detect1(int);
static void ihm_runtick_detect2(int);
static void ihm_runtick_testcan(int);
static void ihm_runtick_master(int);
static void ihm_runtick_slave(int);
static void ihm_runtick_testcanton(int);

void ihm_runtick(void)
{
	static int performInit = 1;
	runmode_t orm = run_mode;
	switch (run_mode) {
	case runmode_normal:	ihm_runtick_normal(performInit); 		break;
	default: // FALLTHRU
	case runmode_off:		ihm_runtick_off(performInit); 			break;
	case runmode_detect_experiment:	ihm_runtick_detect1(performInit);	break;
	case runmode_detect2:	ihm_runtick_detect2(performInit);		break;
	case runmode_testcan:	ihm_runtick_testcan(performInit);		break;
	case runmode_master:	ihm_runtick_master(performInit);		break;
	case runmode_slave:		ihm_runtick_slave(performInit);			break;
	case runmode_testcanton:ihm_runtick_testcanton(performInit); 	break;

	}
	performInit = (run_mode == orm) ? 0 : 1;
}
// -----------------------------------------------------------------------

#endif

#if 0 // OLD

void local_ui_fatal(void)
{
	disp_setlayout(0, LAYOUT_FATAL);
	disp_layout(0);
	Error_Handler();
	for (;;);
}


static void ui_process_msg(void);

typedef enum {
	mode_init = 0,
	mode_ina_detect,
	mode_ina_val,
	mode_manual,
	mode_auto
} ihm_mode_t;

// TODO : change this for per display struct
static ihm_mode_t ihm_dispmode = mode_init;
//static int ihm_train = 0;

static void set_displayout(void)
{
	switch (ihm_dispmode) {
	default:
	case mode_init: 	  	disp_setlayout(0, LAYOUT_INIT); break;
	case mode_ina_detect:	disp_setlayout(0, LAYOUT_INA3221_DETECT); break;
	case mode_ina_val:		disp_setlayout(0, LAYOUT_INA3221_VAL); break;

	case mode_manual:		disp_setlayout(0, LAYOUT_MANUAL); break;
	case mode_auto: 		disp_setlayout(0, LAYOUT_AUTO); break;
	}
	SET_NEEDSREFRESH(0);
}

static int is_special_dispmode(void)
{
	switch (ihm_dispmode) {
	default:
	case mode_init: 	  	return 0;
	case mode_ina_detect:	return 1;
	case mode_ina_val:		return 1;

	case mode_manual:		return 0;
	case mode_auto: 		return 0;
	}
}

static void set_dispmode(ihm_mode_t m)
{
	if (ihm_dispmode == m) return;
	ihm_dispmode = m;
	set_displayout();
}

void ihm_runtick_normal(int init)
{
	//static int cnt=0;
	if (init) {
		itm_debug1(DBG_UI, "UI init", 0);
		set_dispmode(mode_init);
		for (int i = 0; i<DISP_MAX_REGS; i++) {
			ihm_setvar(0, i, 0);
		}
	}
	itm_debug1(DBG_UI, "UI tick", 0);

	needsrefresh_mask = 0;
#ifdef BOARD_HAS_ROTARY_ENCODER
	// scan rotary encoder -----------
	for (int i=0; i<MAX_ROTARY; i++) {
#if UNSIGNED_ROT
		// obsolete and untested now
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
			if (ihm_dispmode==mode_manual) {
				ihm_setvar(0, 1, (uint16_t) rot_position[0]);
				//ihm_setvar(0, 1, ((int)rot0_position - 50));
				SET_NEEDSREFRESH(0);
			}
			if (drive_mode[i]) {	// TODO refactor drive_mode
				msg_64_t m;
				m.from = MA3_UI_GEN;//(i);
				m.to = MA1_CTRL(i);
				m.cmd = CMD_MDRIVE_SPEED_DIR;
				m.v1u = abs(rot_position[i]);
				m.v2 = SIGNOF0(rot_position[i]);
				// TODO handle dir
				mqf_write_from_ui(&m);
			}
		}
#endif
	}
#endif //  BOARD_HAS_ROTARY_ENCODER


	// scan buttons ------------------

	// mode test hoo
#ifdef BOARD_HAS_INA3221
	if (ihm_dispmode==mode_ina_detect) {
		// ina3221 detection
		for (int i=0; i<4; i++) {
			ihm_setvar(0, i, ina3221_devices[i]);
		}
		SET_NEEDSREFRESH(0);
	}
#endif
	// process messages --------------
	ui_process_msg();

	// update displays ---------------
	for (int i=0; i<MAX_DISP; i++) {
		if (NEEDSREFRESH(i)) {
			disp_layout(i);
		}
	}
}


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
		default:
			break;
        case CMD_SETRUN_MODE:
            run_mode = m.v1u;
            return;
            break;

        case CMD_SETVPWM:	// TODO remove
        	//if (test_mode) ui_canton_pwm(m.from, m.v1u, m.v2);
        	return;
        	break;
        case CMD_VOFF_NOTIF:
        	if (NOTIF_VOFF && (!is_special_dispmode())) {
        		ihm_setvar(0, 2, m.v1/2);
        		SET_NEEDSREFRESH(0);
        	}
        	return;
        	break;
        }
		if (MA1_IS_CTRL(m.from)) {
			int trnum = MA1_TRAIN(m.from);
			if (trnum != 0) break; // TODO
			switch (m.cmd) {
			case CMD_TRTSPD_NOTIF:
				itm_debug2(DBG_UI|DBG_CTRL, "rx tspd notif", trnum, m.v1u);
				// TODO trnum -> display num
				if (!NOTIF_VOFF && ((ihm_dispmode == mode_manual) || (ihm_dispmode == mode_auto))) {
					ihm_setvar(0, 2, m.v2 * m.v1u);
					SET_NEEDSREFRESH(0);
				}
				return;
				break;

			case CMD_TRMODE_NOTIF:
				// TODO
				if (!is_special_dispmode()) {
					train_mode_t cm = (train_mode_t) m.v1u;
					switch (cm) {
					default:
					case train_notrunning:
						set_dispmode(mode_init);
						break;
					case train_manual:
					case train_fullmanual:
						set_dispmode(mode_manual);
						break;
					case train_auto:
						set_dispmode(mode_auto);
						break;
					}
				}
				return;
				break;
			case CMD_TRSTATE_NOTIF:
				if (!is_special_dispmode()) {
					ihm_setvar(0, 3, 10+m.v1u);
					SET_NEEDSREFRESH(0);
				}
				return;
				break;
			//case CMD_UI_MSG:
			//	break; // see below
			default:
				itm_debug1(DBG_UI, "unk ctl", m.cmd);
				return;
				break;
			}
		}
		if (MA3_UI_GEN == m.to) {
			int dn = m.to & 0x1F;
			if (dn != 1) {
				itm_debug1(DBG_UI, "?dn", dn);
				continue;
			}
			switch (m.cmd) {
			//case CMD_UI_MSG:
			//	//ui_msg5(dn, (char *) m.rbytes+1);
				break;
			case CMD_INA3221_REPORT:
				if (ihm_dispmode == mode_ina_val) {
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
// run mode OFF
// ---------------------------------



static void ui_process_msg_off(void)
{
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_ui(&m);
		if (rc) break;

		switch(m.cmd) {
		default:
			break;
        case CMD_SETRUN_MODE:
            run_mode = m.v1u;
            return;
            break;
		}
	}
}

void ihm_runtick_off(int init)
{
	needsrefresh_mask = 0;

	if (init) {
		itm_debug1(DBG_UI, "UI init", 0);
		disp_setlayout(0, LAYOUT_OFF);
		for (int i = 0; i<DISP_MAX_REGS; i++) {
			ihm_setvar(0, i, 0);
		}
		SET_NEEDSREFRESH(0);
	}
	// process messages --------------
	ui_process_msg_off();

	// update displays ---------------
	for (int i=0; i<MAX_DISP; i++) {
		if (NEEDSREFRESH(i)) {
			disp_layout(i);
		}
	}
}


// ---------------------------------
// run mode detect1
// ---------------------------------

static void ui_process_msg_d1(void);

static void ihm_runtick_detect1(int init)
{
	static int voltidx = 7;
	needsrefresh_mask = 0;

	if (init) {
		voltidx = 7;
		osDelay(500); // ugly : make sure other tasklet are ready. TODO : fix this
		itm_debug1(DBG_UI, "UI init", 0);
		disp_setlayout(0, LAYOUT_DETECT1);
		for (int i = 0; i<DISP_MAX_REGS; i++) {
			ihm_setvar(0, i, 0);
		}
		ihm_setvar(0, 9, voltidx);
		SET_NEEDSREFRESH(0);
		msg_64_t m;
		m.from = MA3_UI_GEN; //(1);
		//m.to = MA0_CANTON(0);
		xblkaddr_t tb0 = {.v = 0 };
		TO_CANTON(m, tb0);
		m.subc = 1;
		m.cmd = CMD_BEMF_ON;
		mqf_write_from_ui(&m);

		m.from = MA3_UI_GEN;//(1);
		//m.to = MA_CANTON(0);
		TO_CANTON(m, tb0);
		m.subc = 1;
		m.cmd = CMD_SETVPWM;
		m.v1u = voltidx;
		m.v2 = 0;
		mqf_write_from_ui(&m);
	}
#ifdef BOARD_HAS_ROTARY_ENCODER
	// rotary encoder
	static  int16_t rotpos = 0x7FFF;
	int16_t p = get_srotary(&htim4);
	if (p != rotpos) {
		// pos changed
		rotpos = p;
		ihm_setvar(0, 8, rotpos);
		SET_NEEDSREFRESH(0);

		msg_64_t m;
		m.from = MA3_UI_GEN; //(1);
		xblkaddr_t tb1 = {.v = 1};
		TO_CANTON(m, tb1);
		m.cmd = CMD_SETVPWM;
		m.v1u = voltidx;
		m.v2 = rotpos;
		mqf_write_from_ui(&m);

	}
#endif
	// process messages --------------
	ui_process_msg_d1();

	// update displays ---------------
	for (int i=0; i<MAX_DISP; i++) {
		if (NEEDSREFRESH(i)) {
			disp_layout(i);
		}
	}
}



static void ui_process_msg_d1(void)
{
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_ui(&m);
		if (rc) break;

		switch(m.cmd) {

        case CMD_SETRUN_MODE:
            run_mode = m.v1u;
            return;
            break;


        case CMD_BEMF_NOTIF:
    		ihm_setvar(0, 5, m.v2); //Von
    		ihm_setvar(0, 6, m.v1); //Voff
    		SET_NEEDSREFRESH(0);
    		break;
        case CMD_INA3221_VAL1:
        	ihm_setvar(0, 7, m.v1);
        	SET_NEEDSREFRESH(0);
        	break;

        default:
        	itm_debug1(DBG_ERR|DBG_UI, "unhnd msgd1", m.cmd);
        	break;
		}
	}
}


// ---------------------------------
// run mode detect
// ---------------------------------
static void ui_process_msg_d2(void);

static void ihm_runtick_detect2(int init)
{
	//static int voltidx = 7;
	needsrefresh_mask = 0;

	if (init) {
		//osDelay(500); // ugly : make sure other tasklet are ready. TODO : fix this
		itm_debug1(DBG_UI, "UI init", 0);
		disp_setlayout(0, LAYOUT_DETECT2);
    	SET_NEEDSREFRESH(0);

		for (int i = 0; i<DISP_MAX_REGS; i++) {
			ihm_setvar(0, i, 0);
		}

	}
	// process messages --------------
	ui_process_msg_d2();

	// update displays ---------------
	for (int i=0; i<MAX_DISP; i++) {
		if (NEEDSREFRESH(i)) {
			disp_layout(i);
		}
	}
}


static void ui_process_msg_d2(void)
{
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_ui(&m);
		if (rc) break;

		switch(m.cmd) {

        case CMD_SETRUN_MODE:
            run_mode = m.v1u;
            return;
            break;
        case CMD_TRMODE_NOTIF:
        	break;
        case CMD_UI_DETECT:
        	if (m.v1 >= 0) {
        		ihm_setvar(0, 0, m.v1u);
        		ihm_setvar(0, 1, m.v2u);
        	} else {
        		ihm_setvar(0, 0, 999);
        	}
        	SET_NEEDSREFRESH(0);
        	break;

/*
        case CMD_BEMF_NOTIF:
    		ihm_setvar(0, 5, m.v2); //Von
    		ihm_setvar(0, 6, m.v1); //Voff
    		SET_NEEDSREFRESH(0);
    		break;
        case CMD_INA3221_VAL1:
        	ihm_setvar(0, 7, m.v1);
        	SET_NEEDSREFRESH(0);
        	break;
        	*/

        default:
        	itm_debug1(DBG_ERR|DBG_UI, "unhnd msgd2", m.cmd);
        	break;
		}
	}
}


// ---------------------------------
// run mode runmode_testcan
// ---------------------------------
static void ui_process_msg_testcan(void);

static void ihm_runtick_testcan(int init)
{
	needsrefresh_mask = 0;

	if (init) {
		itm_debug1(DBG_UI, "UI init", 0);
		disp_setlayout(0, LAYOUT_TESTCAN);
    	SET_NEEDSREFRESH(0);

		for (int i = 0; i<DISP_MAX_REGS; i++) {
			ihm_setvar(0, i, 0);
		}

	}
	// process messages --------------
	ui_process_msg_testcan();

	// update displays ---------------
	for (int i=0; i<MAX_DISP; i++) {
		if (NEEDSREFRESH(i)) {
			disp_layout(i);
		}
	}
}


static void ui_process_msg_testcan(void)
{
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_ui(&m);
		if (rc) break;

		switch(m.cmd) {

        case CMD_SETRUN_MODE:
            run_mode = m.v1u;
            return;
            break;
        case CMD_CANTEST:
        	ihm_setvar(0, 0, m.v1u);
        	SET_NEEDSREFRESH(0);
        	break;
        case CMD_CANTEST_RESP:
        	ihm_setvar(0, 1, m.v1u);
        	ihm_setvar(0, 2, m.v2u);
        	SET_NEEDSREFRESH(0);
        	break;

        default:
        	itm_debug1(DBG_ERR|DBG_UI, "unhnd msgc", m.cmd);
        	break;
		}
	}
}




// ---------------------------------
// run mode runmode_master
// ---------------------------------
static void ui_process_msg_master(void);

static void ihm_runtick_master(int init)
{
	needsrefresh_mask = 0;

	if (init) {
		itm_debug1(DBG_UI, "UI init", 0);
		disp_setlayout(0, LAYOUT_MASTER);
    	SET_NEEDSREFRESH(0);

		for (int i = 0; i<DISP_MAX_REGS; i++) {
			ihm_setvar(0, i, 0);
		}

	}
	// process messages --------------
	ui_process_msg_master();

	// update displays ---------------
	for (int i=0; i<MAX_DISP; i++) {
		if (NEEDSREFRESH(i)) {
			disp_layout(i);
		}
	}
}


static void ui_process_msg_master(void)
{
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_ui(&m);
		if (rc) break;

		switch(m.cmd) {

        case CMD_SETRUN_MODE:
            run_mode = m.v1u;
            return;
            break;

        case CMD_PARAM_LUSER_VAL:
        	break; /* TODO: when OS X GUI request param values, reply is sent to both
        			* USB and local GUI. Fix this by using separate address ?
        			*/
        default:
        	itm_debug1(DBG_ERR|DBG_UI, "unhnd msgm", m.cmd);
        	break;
		}
	}
}

// ---------------------------------
// run mode runmode_slave
// ---------------------------------
static void ui_process_msg_slave(void);

static void ihm_runtick_slave(int init)
{
	needsrefresh_mask = 0;

	if (init) {
		itm_debug1(DBG_UI, "UI init", 0);
		disp_setlayout(0, LAYOUT_SLAVE);
    	SET_NEEDSREFRESH(0);

		for (int i = 0; i<DISP_MAX_REGS; i++) {
			ihm_setvar(0, i, 0);
		}

	}
	// process messages --------------
	ui_process_msg_slave();

	// update displays ---------------
	for (int i=0; i<MAX_DISP; i++) {
		if (NEEDSREFRESH(i)) {
			disp_layout(i);
		}
	}
}


static void ui_process_msg_slave(void)
{
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_ui(&m);
		if (rc) break;

		switch(m.cmd) {

        case CMD_SETRUN_MODE:
            run_mode = m.v1u;
            return;
            break;

        default:
        	itm_debug1(DBG_ERR|DBG_UI, "unhand msgs", m.cmd);
        	break;
		}
	}
}

// ---------------------------------
// run mode runmode_testcanton
// ---------------------------------
static void ui_process_msg_testcanton(void);

static void ihm_runtick_testcanton(int init)
{
	needsrefresh_mask = 0;

	if (init) {
		itm_debug1(DBG_UI, "UI init", 0);
		disp_setlayout(0, LAYOUT_TESTCANTON);
    	SET_NEEDSREFRESH(0);

		for (int i = 0; i<DISP_MAX_REGS; i++) {
			ihm_setvar(0, i, 0);
		}

	}
	// process messages --------------
	ui_process_msg_testcanton();

	// update displays ---------------
	for (int i=0; i<MAX_DISP; i++) {
		if (NEEDSREFRESH(i)) {
			disp_layout(i);
		}
	}
}


static void ui_process_msg_testcanton(void)
{
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_ui(&m);
		if (rc) break;

		switch(m.cmd) {

        case CMD_SETRUN_MODE:
            run_mode = m.v1u;
            return;
            break;

        case CMD_BEMF_NOTIF:
        	itm_debug3(DBG_UI, "BEMF", m.subc, m.v1, m.v2);
        	int32_t voff = m.v1;
        	int32_t von = m.v2;
        	ihm_setvar(0, 0, m.subc);
			ihm_setvar(0, 1, von);
			ihm_setvar(0, 2, voff);
			SET_NEEDSREFRESH(0);
        	break;
        case CMD_TRMODE_NOTIF: break;
        case CMD_SETVPWM:
        	// forwarded by canton for displau
        	itm_debug2(DBG_ERR|DBG_UI, "pvm", m.cmd, m.subc);
        	// we should check m.subc but we dont
        	ihm_setvar(0, 3, m.v1u);	// volt idx
        	ihm_setvar(0, 4, m.v2);		// pwm
        	break;
        default:
        	itm_debug1(DBG_ERR|DBG_UI, "unhnd msgt", m.cmd);
        	break;
		}
	}
}

#endif // OLD
