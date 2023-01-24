// this file is generated automatically, do not edit
#include <stdint.h>
#include "trainmsgdef.h"
#include "trainmsgstr.h"

const char *traincmd_name(uint8_t cmd)
{
    switch(cmd) {
    default : return "???";
	case CMD_EMERGENCY_STOP  : return "CMD_EMERGENCY_STOP";
	case CMD_TIM_SYNC        : return "CMD_TIM_SYNC";
	case CMD_RESET           : return "CMD_RESET";
	case CMD_SETRUN_MODE     : return "CMD_SETRUN_MODE";
	case CMD_SET_C4          : return "CMD_SET_C4";
	case CMD_SET_C1_C2old    : return "CMD_SET_C1_C2old";
	case CMD_SETVPWM         : return "CMD_SETVPWM";
	case CMD_BEMF_ON         : return "CMD_BEMF_ON";
	case CMD_BEMF_OFF        : return "CMD_BEMF_OFF";
	case CMD_SET_TARGET_SPEED: return "CMD_SET_TARGET_SPEED";
	case CMD_STOP            : return "CMD_STOP";
	case CMD_BRAKE           : return "CMD_BRAKE";
	case CMD_BEMF_DETECT_ON_C2: return "CMD_BEMF_DETECT_ON_C2";
	case CMD_BEMF_NOTIF      : return "CMD_BEMF_NOTIF";
	case CMD_BEMF_DETECT_ON_C2ALT: return "CMD_BEMF_DETECT_ON_C2ALT";
	case CMD_PRESENCE_SUB_CHANGE: return "CMD_PRESENCE_SUB_CHANGE";
	case CMD_POSE_SET_TRIG   : return "CMD_POSE_SET_TRIG";
	case CMD_POSE_TRIGGERED  : return "CMD_POSE_TRIGGERED";
	case CMD_STOP_DETECTED   : return "CMD_STOP_DETECTED";
	case CMD_TURNOUT_A       : return "CMD_TURNOUT_A";
	case CMD_TURNOUT_B       : return "CMD_TURNOUT_B";
	case CMD_TURNOUT_HI_A    : return "CMD_TURNOUT_HI_A";
	case CMD_TURNOUT_HI_B    : return "CMD_TURNOUT_HI_B";
	case CMD_TURNOUT_HI_TOG  : return "CMD_TURNOUT_HI_TOG";
	case CMD_START_DETECT_TRAIN: return "CMD_START_DETECT_TRAIN";
	case CMD_STOP_DETECT_TRAIN: return "CMD_STOP_DETECT_TRAIN";
	case CMD_START_INA_MONITOR: return "CMD_START_INA_MONITOR";
	case CMD_INA_REPORT      : return "CMD_INA_REPORT";
	case CMD_UI_DETECT       : return "CMD_UI_DETECT";
	case CMD_SERVO_SET       : return "CMD_SERVO_SET";
	case CMD_SERVO_ACK       : return "CMD_SERVO_ACK";
	case CMD_SERVODOOR_SET   : return "CMD_SERVODOOR_SET";
	case CMD_SERVODOOR_ACK   : return "CMD_SERVODOOR_ACK";
	case CMD_MDRIVE_SPEED_DIR: return "CMD_MDRIVE_SPEED_DIR";
	case CMD_TRTSPD_NOTIF    : return "CMD_TRTSPD_NOTIF";
	case CMD_TRMODE_NOTIF    : return "CMD_TRMODE_NOTIF";
	case CMD_TRSTATE_NOTIF   : return "CMD_TRSTATE_NOTIF";
	case CMD_BLK_CHG_NOTIF   : return "CMD_BLK_CHG_NOTIF";
	case CMD_TN_RESER_NOTIF  : return "CMD_TN_RESER_NOTIF";
	case CMD_SET_TRAIN_MODE  : return "CMD_SET_TRAIN_MODE";
	case CMD_START_AUTO      : return "CMD_START_AUTO";
	case CMD_TN_CHG_NOTIF    : return "CMD_TN_CHG_NOTIF";
	case CMD_PARAM_LUSER_COMMIT: return "CMD_PARAM_LUSER_COMMIT";
	case CMD_PARAM_LUSER_GET : return "CMD_PARAM_LUSER_GET";
	case CMD_PARAM_LUSER_SET : return "CMD_PARAM_LUSER_SET";
	case CMD_PARAM_LUSER_VAL : return "CMD_PARAM_LUSER_VAL";
	case CMD_PARAM_USER_SET  : return "CMD_PARAM_USER_SET";
	case CMD_PARAM_USER_GET  : return "CMD_PARAM_USER_GET";
	case CMD_PARAM_USER_VAL  : return "CMD_PARAM_USER_VAL";
	case CMD_PARAM_PROPAG    : return "CMD_PARAM_PROPAG";
	case CMD_NOOP            : return "CMD_NOOP";
	case CMD_NOTIF_PRES      : return "CMD_NOTIF_PRES";
	case CMD_TIM_SYNC_ACK    : return "CMD_TIM_SYNC_ACK";
	case CMD_LED_RUN         : return "CMD_LED_RUN";
	case CMD_INA3221_REPORT  : return "CMD_INA3221_REPORT";
	case CMD_VOFF_NOTIF      : return "CMD_VOFF_NOTIF";
	case CMD_INA3221_VAL1    : return "CMD_INA3221_VAL1";
	case CMD_NOTIF_SPEED     : return "CMD_NOTIF_SPEED";
	case CMD_TRIG_OSCILLO    : return "CMD_TRIG_OSCILLO";
	case CMD_CANTEST         : return "CMD_CANTEST";
	case CMD_CANTEST_RESP    : return "CMD_CANTEST_RESP";
	case CMD_USB_STATS       : return "CMD_USB_STATS";
	case CMD_USB_OSCILLO     : return "CMD_USB_OSCILLO";
	case CMD_USB_RECORD_MSG  : return "CMD_USB_RECORD_MSG";
	case CMD_OAM_MASTER      : return "CMD_OAM_MASTER";
	case CMD_OAM_SLAVE       : return "CMD_OAM_SLAVE";
	case CMD_OAM_BNUM        : return "CMD_OAM_BNUM";
	case CMD_OAM_SLV_OK      : return "CMD_OAM_SLV_OK";
	case CMD_OAM_CUSTOM      : return "CMD_OAM_CUSTOM";
	case CMD_PLANNER_RESET   : return "CMD_PLANNER_RESET";
	case CMD_PLANNER_ADD     : return "CMD_PLANNER_ADD";
	case CMD_PLANNER_CANCEL  : return "CMD_PLANNER_CANCEL";
	case CMD_PLANNER_COMMIT  : return "CMD_PLANNER_COMMIT";
    }
}

msg_type_t traincmd_format(uint8_t cmd)
{
    switch(cmd) {
    default : return CMD_TYPE_V32;
	case CMD_EMERGENCY_STOP  : return CMD_TYPE_V32;
	case CMD_TIM_SYNC        : return CMD_TYPE_V32;
	case CMD_RESET           : return CMD_TYPE_V32;
	case CMD_SETRUN_MODE     : return CMD_TYPE_V16;
	case CMD_SET_C4          : return CMD_TYPE_B4;
	case CMD_SET_C1_C2old    : return CMD_TYPE_B4;
	case CMD_SETVPWM         : return CMD_TYPE_V16;
	case CMD_BEMF_ON         : return CMD_TYPE_V32;
	case CMD_BEMF_OFF        : return CMD_TYPE_V32;
	case CMD_SET_TARGET_SPEED: return CMD_TYPE_V16;
	case CMD_STOP            : return CMD_TYPE_V32;
	case CMD_BRAKE           : return CMD_TYPE_V32;
	case CMD_BEMF_DETECT_ON_C2: return CMD_TYPE_V16;
	case CMD_BEMF_NOTIF      : return CMD_TYPE_V16;
	case CMD_BEMF_DETECT_ON_C2ALT: return CMD_TYPE_V16;
	case CMD_PRESENCE_SUB_CHANGE: return CMD_TYPE_V16;
	case CMD_POSE_SET_TRIG   : return CMD_TYPE_VCU;
	case CMD_POSE_TRIGGERED  : return CMD_TYPE_VCU;
	case CMD_STOP_DETECTED   : return CMD_TYPE_V32;
	case CMD_TURNOUT_A       : return CMD_TYPE_V32;
	case CMD_TURNOUT_B       : return CMD_TYPE_V32;
	case CMD_TURNOUT_HI_A    : return CMD_TYPE_V16;
	case CMD_TURNOUT_HI_B    : return CMD_TYPE_V16;
	case CMD_TURNOUT_HI_TOG  : return CMD_TYPE_V16;
	case CMD_START_DETECT_TRAIN: return CMD_TYPE_V16;
	case CMD_STOP_DETECT_TRAIN: return CMD_TYPE_V32;
	case CMD_START_INA_MONITOR: return CMD_TYPE_V16;
	case CMD_INA_REPORT      : return CMD_TYPE_V16;
	case CMD_UI_DETECT       : return CMD_TYPE_V16;
	case CMD_SERVO_SET       : return CMD_TYPE_V16;
	case CMD_SERVO_ACK       : return CMD_TYPE_V16;
	case CMD_SERVODOOR_SET   : return CMD_TYPE_V16;
	case CMD_SERVODOOR_ACK   : return CMD_TYPE_V16;
	case CMD_MDRIVE_SPEED_DIR: return CMD_TYPE_V16;
	case CMD_TRTSPD_NOTIF    : return CMD_TYPE_V16;
	case CMD_TRMODE_NOTIF    : return CMD_TYPE_V16;
	case CMD_TRSTATE_NOTIF   : return CMD_TYPE_V16;
	case CMD_BLK_CHG_NOTIF   : return CMD_TYPE_B4;
	case CMD_TN_RESER_NOTIF  : return CMD_TYPE_V16;
	case CMD_SET_TRAIN_MODE  : return CMD_TYPE_V16;
	case CMD_START_AUTO      : return CMD_TYPE_V16;
	case CMD_TN_CHG_NOTIF    : return CMD_TYPE_V16;
	case CMD_PARAM_LUSER_COMMIT: return CMD_TYPE_V32;
	case CMD_PARAM_LUSER_GET : return CMD_TYPE_V40;
	case CMD_PARAM_LUSER_SET : return CMD_TYPE_V40;
	case CMD_PARAM_LUSER_VAL : return CMD_TYPE_V40;
	case CMD_PARAM_USER_SET  : return CMD_TYPE_V40;
	case CMD_PARAM_USER_GET  : return CMD_TYPE_V40;
	case CMD_PARAM_USER_VAL  : return CMD_TYPE_V40;
	case CMD_PARAM_PROPAG    : return CMD_TYPE_V40;
	case CMD_NOOP            : return CMD_TYPE_V32;
	case CMD_NOTIF_PRES      : return CMD_TYPE_V32;
	case CMD_TIM_SYNC_ACK    : return CMD_TYPE_V32;
	case CMD_LED_RUN         : return CMD_TYPE_V16;
	case CMD_INA3221_REPORT  : return CMD_TYPE_B4;
	case CMD_VOFF_NOTIF      : return CMD_TYPE_V16;
	case CMD_INA3221_VAL1    : return CMD_TYPE_V32;
	case CMD_NOTIF_SPEED     : return CMD_TYPE_V16;
	case CMD_TRIG_OSCILLO    : return CMD_TYPE_V16;
	case CMD_CANTEST         : return CMD_TYPE_V16;
	case CMD_CANTEST_RESP    : return CMD_TYPE_V16;
	case CMD_USB_STATS       : return CMD_TYPE_V32;
	case CMD_USB_OSCILLO     : return CMD_TYPE_V32;
	case CMD_USB_RECORD_MSG  : return CMD_TYPE_V32;
	case CMD_OAM_MASTER      : return CMD_TYPE_V32;
	case CMD_OAM_SLAVE       : return CMD_TYPE_V32;
	case CMD_OAM_BNUM        : return CMD_TYPE_V32;
	case CMD_OAM_SLV_OK      : return CMD_TYPE_V32;
	case CMD_OAM_CUSTOM      : return CMD_TYPE_V32;
	case CMD_PLANNER_RESET   : return CMD_TYPE_V32;
	case CMD_PLANNER_ADD     : return CMD_TYPE_V32;
	case CMD_PLANNER_CANCEL  : return CMD_TYPE_V32;
	case CMD_PLANNER_COMMIT  : return CMD_TYPE_V32;
    }
}

