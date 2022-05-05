/*
 * oam_error.h
 *
 *  Created on: Apr 24, 2022
 *      Author: danielbraun
 */

#ifndef OAM_OAM_ERROR_H_
#define OAM_OAM_ERROR_H_



enum fatal_error_code {
	Error_None = 0,
	Error_Other,		// Error_Handler
	Error_NoC2s,
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
};


void FatalError(const char *shortsmsg, const char *longmsg, enum fatal_error_code errcode);



extern const char *_fatal;

/* local_ui_fatal may be implemented by local_ui
 * to display msg
 */
void local_ui_fatal(void);

#endif /* OAM_OAM_ERROR_H_ */
