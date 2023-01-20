// this file is generated automatically, do not edit
#ifndef trainmsgdef_h
#define trainmsgdef_h

typedef enum {
	// ---------------------------- group 0x00
	CMD_EMERGENCY_STOP      = 0x00, // 0x00 (0) // ui,any->(all)	stop/reset all tasklet
	CMD_TIM_SYNC                  , // 0x01 (1) //
	CMD_RESET                     , // 0x02 (2) // ui,any->(all)	reset all tasklet, currently do same as CMD_EMERGENCY_STOP, but could do more, e.g. resetting some params
	CMD_SETRUN_MODE               , // 0x03 (3) // ui,any->(all)	v1u=mode				start given runmode

	// ---------------------------- group 0x20
	CMD_SET_C1_C2           = 0x20, // 0x20 (32) // ctrl->spdctl		b:c1,dir1,c2,dir2		set C1,C2 for sender train
	CMD_SETVPWM                   , // 0x21 (33) // spdctl->canton	v1u=voltidx,v2=pwm		set power on canton
	CMD_BEMF_ON                   , // 0x22 (34) // spdctl->canton							start measure bemf
	CMD_BEMF_OFF                  , // 0x23 (35) // spdctl->canton							stop measure bemf
	CMD_SET_TARGET_SPEED          , // 0x24 (36) // ctrl->spdctl		v1u=spd					set (unsigned) speed
	CMD_STOP                      , // 0x25 (37) // ctrl->spdctl								stop canton (min volt, pwv=0), equiv to CMD_SETVPWM 7,0
	CMD_BRAKE                     , // 0x26 (38) // ctrl->spdctl subc: 1=set, 0=clear, v32=pos

	// ---------------------------- group 0x40
	CMD_BEMF_DETECT_ON_C2   = 0x40, // 0x40 (64) // spdctl->ctrl		v1u=C2 v2=POSE/10		sent when C2 canton detect BEMF
	CMD_BEMF_NOTIF                , // 0x41 (65) // canton->spdctl	v1=voff v2=pose subc=trig tag canton BEMF measurement
	CMD_BEMF_DETECT_ON_C2ALT      , // 0x42 (66) // 											unused for now
	CMD_PRESENCE_SUB_CHANGE       , // 0x43 (67) // ina->ctrl		subc=ch v1=pres v2=vi   ina change value notification
	CMD_POSE_SET_TRIG             , // 0x44 (68) // ctrl->canton     vcu8=tag va16=POSE/10   vb8=dir    set POSE trigger
	CMD_POSE_TRIGGERED            , // 0x45 (69) // bemf->ctrl		va16=pos/100, vb=dir, vc=tag  trigger notification
	CMD_STOP_DETECTED             , // 0x46 (70) // spdctl->ctrl		v32=POSE				stop condition (after pid and inertia) reached

	// ---------------------------- group 0x60
	CMD_TURNOUT_A           = 0x60, // 0x60 (96) // ctrl->turnout,ui	subc=tn					set turnout pos to straight (low level)
	CMD_TURNOUT_B                 , // 0x61 (97) // ctrl->turnout,ui subc=tn					set turnout pos to turn (low level)
	CMD_TURNOUT_HI_A              , // 0x62 (98) // ui,auto->ctrl	v1=tn					set turnout to straight (registering with occupency/train ctl)
	CMD_TURNOUT_HI_B              , // 0x63 (99) // ui,auto->ctrl	v1=tn					set turnout to turn (registering with occupency/train ctl)
	CMD_TURNOUT_HI_TOG            , // 0x64 (100) // ui->ctrl			v1=tn					toggle turnout pos

	// ---------------------------- group 0x80
	CMD_START_DETECT_TRAIN  = 0x80, // 0x80 (128) // [detect2] ctrl->canton					start train detection
	CMD_STOP_DETECT_TRAIN         , // 0x81 (129) // [detect2] ctrl->canton					stop train detection
	CMD_START_INA_MONITOR         , // 0x82 (130) // [detect2] ctrl->ina3221					start monitoring current
	CMD_INA_REPORT                , // 0x83 (131) // [detect2] ina3221->ctrl					report measured current
	CMD_UI_DETECT                 , // 0x84 (132) // [detect2] ctrl->ui						detection info
	CMD_SERVO_SET                 , // 0x85 (133) // any -> servo  subc=servn v1u=pos v2u=spd_or_0
	CMD_SERVO_ACK                 , // 0x86 (134) // servo->sender subc=servn v1u=pos
	CMD_SERVODOOR_SET             , // 0x87 (135) // any -> servo  subc=servn v1u=1 open, 0 close
	CMD_SERVODOOR_ACK             , // 0x88 (136) // servo->sender subc=servn v1u=1 open, 0 close

	// ---------------------------- group 0xA0

	// ---------------------------- group 0xC0
	CMD_MDRIVE_SPEED_DIR    = 0xC0, // 0xc0 (192) // ui->ctrl			v1u=spd v2=dir			set dir and speed (desired speed)
	CMD_TRTSPD_NOTIF              , // 0xc1 (193) // ctrl->ui			v1u=spd v2=dir			notify spd and dir
	CMD_TRMODE_NOTIF              , // 0xc2 (194) // ctrl->ui			v1u=mode				mode notif
	CMD_TRSTATE_NOTIF             , // 0xc3 (195) // ctrl->ui			v1u=state v2u=oldstate				state chg notif
	CMD_BLK_CHG_NOTIF             , // 0xc4 (196) // ctrl->ui			b:blk,occ,trnum,sblk	occupency change notif
	CMD_TN_RESER_NOTIF            , // 0xc5 (197) // ctrl->ui			v1=turnout v2=trnum		turnout lock/unlock notif
	CMD_SET_TRAIN_MODE            , // 0xc6 (198) // ui,?->ctrl		v1u=tr v2u=mode			chg train mode, mostly to off
	CMD_START_AUTO                , // 0xc7 (199) // ui->ctrl			mv1u=autonum			start auto global scenario
	CMD_TN_CHG_NOTIF              , // 0xc8 (200) // ctrl->ui         subc=turnout v1=pos     notif UI turnout change (used to be done by SETA/SETB)
	CMD_PARAM_LUSER_COMMIT        , // 0xc9 (201) // ui(?) -> oam(0)
	CMD_PARAM_LUSER_GET           , // 0xca (202) // ui ->oam(0)
	CMD_PARAM_LUSER_SET           , // 0xcb (203) // ui ->oam(0)
	CMD_PARAM_LUSER_VAL           , // 0xcc (204) // oam(0) ->ui
	CMD_PARAM_USER_SET            , // 0xcd (205) // ui(?) -> oam(0)							user parameter chage
	CMD_PARAM_USER_GET            , // 0xce (206) // ui(?) -> oam(0)
	CMD_PARAM_USER_VAL            , // 0xcf (207) // oam(0) -> ui(?)
	CMD_PARAM_PROPAG              , // 0xd0 (208) // oam(0) -> oam(slave)

	// ---------------------------- group 0xE0
	CMD_NOOP                = 0xE0, // 0xe0 (224) // ?->can									msg intended for CAN testing, do nothing
	CMD_NOTIF_PRES                , // 0xe1 (225) // ctrl -> ui							    ina based presence on sblk (bitfield)
	CMD_TIM_SYNC_ACK              , // 0xe2 (226) //                                          slave->master ack synchro
	CMD_LED_RUN                   , // 0xe3 (227) // auto,ui->led		v1u=lednum v2u=prg		run led program
	CMD_INA3221_REPORT            , // 0xe4 (228) // ina->ui (debug)
	CMD_VOFF_NOTIF                , // 0xe5 (229) // canton->ui (debug)						used if NOTIF_VOFF=1
	CMD_INA3221_VAL1              , // 0xe6 (230) 
	CMD_NOTIF_SPEED               , // 0xe7 (231) // spdctl->ui	(debug)
	CMD_TRIG_OSCILLO              , // 0xe8 (232) // any -> spdctl	v1=canton_of_interest
	CMD_CANTEST                   , // 0xe9 (233) // any (cantest) -> any (cantest)
	CMD_CANTEST_RESP              , // 0xea (234) // any - > any (ui)
	CMD_USB_STATS                 , // 0xeb (235) // any -> usb								send stats
	CMD_USB_OSCILLO               , // 0xec (236) // any -> usb								send oscillo values
	CMD_USB_RECORD_MSG            , // 0xed (237) // any -> usb								dump recorded msg if enabled (see msgrecord.h)
	CMD_OAM_MASTER                , // 0xee (238) 
	CMD_OAM_SLAVE                 , // 0xef (239) 
	CMD_OAM_BNUM                  , // 0xf0 (240) 
	CMD_OAM_SLV_OK                , // 0xf1 (241) 
	CMD_OAM_CUSTOM                , // 0xf2 (242) // any -> OAM(), not defined (for test)
	CMD_PLANNER_RESET             , // 0xf3 (243) // UI->planner								stop all trains, reset planner
	CMD_PLANNER_ADD               , // 0xf4 (244) // UI->planner
	CMD_PLANNER_CANCEL            , // 0xf5 (245) // UI->planner
	CMD_PLANNER_COMMIT            , // 0xf6 (246) // UI->planner
} cmd_msg_t;

#endif
