/*
 * oam_error.h
 *
 *  Created on: Apr 24, 2022
 *      Author: danielbraun
 */

#ifndef OAM_OAM_ERROR_H_
#define OAM_OAM_ERROR_H_

/*
 list of all fatal errors
 */

enum fatal_error_code {
	Error_None = 0,
	Error_Stack,
	Error_Other,		// Error_Handler
	Error_Check,
	Error_NoC2s,
	Error_WrongC1,
	Error_Slipping,
	Error_CtrlTimerNum,
	Error_CtrlTSpeed,
	Error_CtrlNoDir,
	Error_CtrlNoC1,
	Error_CtrlNoC1l,
	Error_CtrlBadPose,
	Error_DetectBadNum,
	Error_Ledc,
	Error_LEdn,
	Error_BrdUnknown,
	Error_FlashInit,
	Error_Sizeof,
	Error_FlashNbDesc,
	Error_NotMaster,
	Error_NotSlave,
	Error_CanInit,
	Error_CanStart,
	Error_CanActNotif,
	Error_CanId,
	Error_CanTx,
	Error_Occupency,
	Error_OccDelay,
	Error_OccTrn,
	Error_MsgQBig,
	Error_MsgQLen,
	Error_BrdSlvMaster,
	Error_NumBnum,
	Error_NumBnum2,
	Error_NoFlash,
	Error_FlashRentered,

	Error_CAN_Rx_Overrun0,
	Error_CAN_Rx_Overrun1,

	Error_PlannerTooManyBlocks,
    Error_Hash,
    
    Error_FSM_Nothandled,
    Error_FSM_DSpd,
    Error_FSM_SignDir,
    Error_FSM_NoDir,
    Error_FSM_ShouldEotOcc,
    Error_FSM_ChkNeg1,
    Error_FSM_ChkNeg2,
    Error_FSM_RunToEot,
    Error_FSM_BlkToEot,
    Error_FSM_C1C2Zero,
    Error_FSM_NotImplemented,
    Error_FSM_BadFut2,
    Error_FSM_Sanity1,
    Error_FSM_Sanity2,
    Error_FSM_Sanity3,
    Error_FSM_Sanity4,
    Error_FSM_Sanity5,
    Error_FSM_Sanity6,
    Error_FSM_Sanity7,
    Error_FSM_Sanity8,
    Error_FSM_Sanity9,

};


/// FatalError handling
///
/// fatal error stop execution and
///
///      - try to display error msg on SSD1306 localy attached, if any (through IHM module)
///
///      - disable interrupt (TODO)
///
///      - infinite loop
/// @param shortsmsg message associated with error, short enough for ssd1306 (typ 4 or 5 chars)
/// @param longmsg long message, usable e.g. with debugger
/// @param errcode numeric errorcode (fatal_error_code), displayable and usable as debug
void FatalError(const char *shortsmsg, const char *longmsg, enum fatal_error_code errcode);



extern const char *_fatal;


/// local_ui_fatal is called by FatalError(), thus in the failing task, to display message on ssd1306 display (or other)
///
/// it can be (and actually is) implemented by ihm/local_ui module,
/// a weak empty implementation is defined in oam_error.c in case it is not defined in ihm or if ihm is not included in build
void local_ui_fatal(void);

#endif /* OAM_OAM_ERROR_H_ */
