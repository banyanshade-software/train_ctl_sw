//
//  trainmsgdef.h
//  train_throttle
//
//  Created by Daniel BRAUN on 15/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef trainmsgdef_h
#define trainmsgdef_h

// hi 3 bits of cmd define prioriry
typedef enum {
    // highest prio   000 xxxxx
    CMD_EMERGENCY_STOP = 0,			// ui,any->(all)							stop/reset all tasklet
    CMD_TIM_SYNC,                   //
    CMD_RESET,						// ui,any->(all)							reset all tasklet, currently do same as CMD_EMERGENCY_STOP, but could do more, e.g. resetting some params
    CMD_SETRUN_MODE,				// ui,any->(all)	v1u=mode				start given runmode
    
    // high prio 001 xxxxx 0x20
    CMD_SET_C1_C2 = 0x20,			// ctrl->spdctl		b:c1,dir1,c2,dir2		set C1,C2 for sender train
    CMD_SETVPWM,					// spdctl->canton	v1u=voltidx,v2=pwm		set power on canton
    CMD_BEMF_ON,					// spdctl->canton							start measure bemf
    CMD_BEMF_OFF,					// spdctl->canton							stop measure bemf
    CMD_SET_TARGET_SPEED,			// ctrl->spdctl		v1u=spd					set (unsigned) speed
    CMD_STOP, 						// ctrl->spdctl								stop canton (min volt, pwv=0), equiv to CMD_SETVPWM 7,0
    
    // 010 xxxxx
    CMD_BEMF_DETECT_ON_C2 = 0x40,	// spdctl->ctrl		v1u=C2 v2=POSE/10		sent when C2 canton detect BEMF
    CMD_BEMF_NOTIF,					// canton->spdctl	v1=voff v2=pose subc=trig tag canton BEMF measurement
    CMD_BEMF_DETECT_ON_C2ALT,		// 											unused for now
    CMD_PRESENCE_SUB_CHANGE,		// ina->ctrl		subc=ch v1=pres v2=vi   ina change value notification
    
    CMD_POSE_SET_TRIG,              // ctrl->canton     subc=tag v1=POSE/10   v2=dir    set POSE trigger
    //CMD_POSE_SET_TRIG0,			// ctrl->spdctl		v1=POSE/10				set POSE trigger 0
    //CMD_POSE_SET_TRIG_U1,			// ctrl->spdctl		v1=POSE/10				set POSE trigger U1
    CMD_POSE_TRIGGERED,				// spdctl->ctrl		s=trignum v1u=C1 v2=P/10 pose trigger notification
    CMD_STOP_DETECTED,				// spdctl->ctrl		v32=POSE				stop condition (after pid and inertia) reached
    
    // 011 xxxxx
    CMD_TURNOUT_A = 0x60,			// ctrl->turnout,ui	v2=tn					set turnout pos to straight (low level)
    CMD_TURNOUT_B,					// ctrl->turnout,ui v2=tn					set turnout pos to turn (low level)
    CMD_TURNOUT_HI_A,				// ui,auto->ctrl	v1=tn					set turnout to straight (registering with occupency/train ctl)
    CMD_TURNOUT_HI_B,				// ui,auto->ctrl	v1=tn					set turnout to turn (registering with occupency/train ctl)
    CMD_TURNOUT_HI_TOG,				// ui->ctrl			v1=tn					toggle turnout pos
    
    // 100 xxxxx = 0x80
    CMD_START_DETECT_TRAIN = 0x80,			// [detect2] ctrl->canton					start train detection
    CMD_STOP_DETECT_TRAIN,			// [detect2] ctrl->canton					stop train detection
	CMD_START_INA_MONITOR,			// [detect2] ctrl->ina3221					start monitoring current
	CMD_INA_REPORT,					// [detect2] ina3221->ctrl					report measured current
    CMD_UI_DETECT,					// [detect2] ctrl->ui						detection info
    
    // 101 xxxxx = 0xA0
    // 110 xxxxx = 0xC0 GUI
    CMD_MDRIVE_SPEED_DIR = 0xC0,	// ui->ctrl			v1u=spd v2=dir			set dir and speed (desired speed)
    //CMD_TRDIR_NOTIF,				// ctrl->ui	obsolete ?
    CMD_TRTSPD_NOTIF,				// ctrl->ui			v1u=spd v2=dir			notify spd and dir
    //CMD_TRSTATUS_NOTIF,				// ctrl->ui
    CMD_TRMODE_NOTIF,				// ctrl->ui			v1u=mode				mode notif
    CMD_TRSTATE_NOTIF,				// ctrl->ui			v1u=mode				state chg notif
    CMD_BLK_CHG_NOTIF,				// ctrl->ui			b:blk,occ,trnum,sblk	occupency change notif
    CMD_TN_RESER_NOTIF,				// ctrl->ui			v1=turnout v2=trnum		turnout lock/unlock notif
    CMD_SET_TRAIN_MODE,				// ui,?->ctrl		v1u=tr v2u=mode			chg train mode, mostly to off
    CMD_START_AUTO,					// ui->ctrl			mv1u=autonum			start auto global scenario
    //CMD_UI_MSG,
    
    // local master-only store
    CMD_PARAM_LUSER_COMMIT,            // ui(?) -> oam(0)
    CMD_PARAM_LUSER_GET,               // ui ->oam(0)
    CMD_PARAM_LUSER_SET,               // ui ->oam(0)
    CMD_PARAM_LUSER_VAL,                // oam(0) ->ui
    
    // normal multiboard store
	CMD_PARAM_USER_SET,				// ui(?) -> oam(0)							user parameter chage
	CMD_PARAM_USER_GET,				// ui(?) -> oam(0)
	CMD_PARAM_USER_VAL,				// oam(0) -> ui(?)
	CMD_PARAM_PROPAG,				// oam(0) -> oam(slave)
    
    // 111 xxxxx = 0xE0 debug and misc low prio
    CMD_NOOP = 0xE0,				// ?->can									msg intended for CAN testing, do nothing
    CMD_TIM_SYNC_ACK,               //                                          slave->master ack synchro
    CMD_LED_RUN,					// auto,ui->led		v1u=lednum v2u=prg		run led program
    CMD_INA3221_REPORT,				// ina->ui (debug)
    CMD_VOFF_NOTIF,					// canton->ui (debug)						used if NOTIF_VOFF=1
    CMD_INA3221_VAL1,
    CMD_NOTIF_SPEED,				// spdctl->ui	(debug)
	CMD_TRIG_OSCILLO,				// any -> spdctl

	CMD_CANTEST,					// any (cantest) -> any (cantest)
	CMD_CANTEST_RESP				// any - > any (ui)
} cmd_msg_t;

#endif /* trainmsgdef_h */


