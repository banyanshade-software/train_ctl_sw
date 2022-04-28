/*
 * canmsg.c
 *
 *  Created on: Nov 22, 2021
 *      Author: danielbraun
 */



#include "canmsg.h"
#include "cmsis_os.h"

#include "../misc.h"



#ifndef BOARD_HAS_CAN
#error BOARD_HAS_CAN not defined, remove this file from build
#endif

#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#include "../msg/trainmsg.h"

#include "canmsg.h"

/*
 * error here on CAN_HandleTypeDef probably due to CAN is not activated in  ioc
 */
extern CAN_HandleTypeDef CAN_DEVICE;

static runmode_t run_mode = 0;
static int local_msg_process(msg_64_t *m, int localmsg);

/*
 *
 *
 *
 *
 * http://www.bittiming.can-wiki.info
 *
 * ABOM
 */

static void can_init(void)
{

	// CAN1_RX0_IRQHandler
	static CAN_FilterTypeDef canfil;
	canfil.FilterBank = 0;
	canfil.FilterMode = CAN_FILTERMODE_IDMASK;
	canfil.FilterFIFOAssignment = CAN_RX_FIFO0;
	canfil.FilterIdHigh = 0;
	canfil.FilterIdLow = 0;
	canfil.FilterMaskIdHigh = 0;
	canfil.FilterMaskIdLow = 0;
	canfil.FilterScale = CAN_FILTERSCALE_32BIT;
	canfil.FilterActivation = ENABLE;
	canfil.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&CAN_DEVICE, &canfil)) {
		FatalError("CANini", "CAN init failed", Error_CanInit);
	}



	if (HAL_CAN_Start(&CAN_DEVICE) != HAL_OK) {
		/* Start Error */
		FatalError("CANstart", "CAN start failed", Error_CanStart);
	}

	/* Activate CAN RX notification */
	if (HAL_CAN_ActivateNotification(&CAN_DEVICE,
			 CAN_IT_RX_FIFO0_MSG_PENDING
			|CAN_IT_RX_FIFO0_FULL
			|CAN_IT_RX_FIFO0_OVERRUN
			|CAN_IT_RX_FIFO1_MSG_PENDING
			|CAN_IT_TX_MAILBOX_EMPTY
			|CAN_IT_WAKEUP
			|CAN_IT_ERROR_WARNING
			|CAN_IT_ERROR_PASSIVE
			|CAN_IT_BUSOFF
			|CAN_IT_LAST_ERROR_CODE
			|CAN_IT_ERROR) != HAL_OK) {
		/* Notification Error */
		FatalError("CANact", "CAN act notif failed", Error_CanActNotif);
	}


}
/*

void StartCanTask(_UNUSED_ void *argument)
{
	//can_init();
	for (;;) osDelay(1000);
}

*/

/*
static int boardForAddr(uint8_t addr)
{
	if (0==(addr & 0x80)) {
		return MA_2_BOARD(addr);
	}
	if ((addr & MA_ADDR_MASK_5) == MA_ADDR_5_LED) { // XXX
		return (addr & 0x07);
	}
	return 0;
}
*/


/*
 * arbitration id (11 bit)
 * - it is mandatory that 2 stations never send msg with same id at the same time
 * - lower id have priority over higher id in CAN contention resolution
 *
 * id is form with msg priority (3 high bits of msg cmd) and sender/receiver board num
 *
 * msg sent by slave board to board 0 (main board) uses their board number
 * 		p p p  0 1 b b b b x x
 * msg sent from board 0 (main board) to slave board uses slave board number
 *      p p p  0 0 b b b b x x
 * msg sent to UI (since UI may be implemented by multiple board)
 *      p p p  1 0 1 s  s s x x
 * msg sent from UI
 *      p p p  1 1 0 s s s x x
 *
 * reserved:
 * 		p p p  1 1 1 ...
 * 		p p p  1 1 1 1 b b b b    broadcast
 *
 * 	x x is a 2 bits counter maintained by sender
 */


static uint32_t arbitration_id(msg_64_t *m)
{
	// 0x7FF 11 bits
	// 3 bits priority of msg, high 3 bits of m->cmd
	// 4 bits board num or function num
	// 1 bit function
	// 3 bits counter
	// p p p   b b b b  f n n n

	// master->slave : board num is dest board or dest function
	// slave->master : board num is sending board (or 0 if unknwon)

	// prio from msg
	uint32_t aid = (m->cmd & 0xE0) << 3;

	int lb = localBoardNum(); // -1 if unknown

	if ((0)) {
	} else if (MA3_IS_GLOB_ADDR(m->to)) {
		aid |= 0x80; // function bit
		aid |= (m->to & 0x0F)<<4;

	} else if (lb<0) {
			aid |= 0xF8; // function 0xF
	} else if (lb>0) {
		// slave->master, use boardnum
		aid |= lb<<4;

	} else if (MA0_ADDR_IS_BOARD_ADDR(m->to)) {
		aid |= MA0_BOARD(m->to) << 4;
	} else {
		FatalError("CANid", "CAN id calc", Error_CanId);
	}


	static uint8_t cnt = 0;
	aid |= cnt & 0x07;
	cnt++;
	return aid;
}

static uint32_t TxMailbox;

static int _can_send_msg(msg_64_t *msg, int f)
{
	CAN_TxHeaderTypeDef txHeader;

	txHeader.RTR = CAN_RTR_DATA;
	txHeader.IDE = CAN_ID_STD;
	txHeader.DLC = 8;
	txHeader.TransmitGlobalTime = DISABLE;

	txHeader.StdId = arbitration_id(msg); // Arbitration id

	if (HAL_CAN_AddTxMessage(&CAN_DEVICE, &txHeader, (uint8_t *)msg, &TxMailbox) != HAL_OK) {
		//printf("Error: CAN can't send msg.\r\n");
		FatalError("CANtx", "CAN tx ", Error_CanTx);
		return 1;
	}
	if (f) itm_debug3(DBG_CAN, "Tx/I", msg->v1, msg->v2, TxMailbox);
	else   itm_debug3(DBG_CAN, "Tx/p", msg->v1, msg->v2, TxMailbox);
	return 0;
}
// HAL_NVIC_SetPriority SVCall


#if 0
void CanTest(void) // XXX to be removed
{
	msg_64_t m = {0};
	m.from = MA_CONTROL_T(1);
	m.to = MA_UI(0);
	m.cmd = CMD_NOOP;
	m.v1 = 422;
	m.v2 = -4242;

	can_init();
	TxMailbox = 42;
	_can_send_msg(&m,0);

	int i;
	for (i=0; i<100; i++) {
		if (HAL_CAN_IsTxMessagePending(&CAN_DEVICE, TxMailbox)) {
			itm_debug2(DBG_CAN, "sent", TxMailbox, i);
			break;
		}
		osDelay(10);
	}
	if (i==100) {
		itm_debug1(DBG_CAN|DBG_ERR, "CAN KO", TxMailbox);
	}
}
#endif


static void _send_msg_if_any(int fromirq)
{
	for (int i = 0; i<3; i++) {
		if (!HAL_CAN_GetTxMailboxesFreeLevel(&CAN_DEVICE)) return;
		msg_64_t m;
		int rc = mqf_read_to_canbus(&m);
		if (rc) return;
		rc = local_msg_process(&m, 1);
		if (rc) continue;
		_can_send_msg(&m, fromirq);
	}
}


static void send_messages_if_any(void)
{
	if (!mqf_len(&to_canbus)) return;
	/*
	 * TODO : if CAN Tx is stuck (eg because not connected to transceiver)
	 * we will stop reading msgq and have a q full condition here !!
	 */
	static uint32_t tlocked = 0;

	if (!HAL_CAN_GetTxMailboxesFreeLevel(&CAN_DEVICE))  {
		if (!tlocked) tlocked = HAL_GetTick();
		else {
			if (HAL_GetTick() > tlocked+1000) {
				// no free mbox for 1 second at least
				// probably CAN is stuck (eg no transciever connected)
				itm_debug1(DBG_ERR|DBG_CAN, "CAN lock", 0);
				for (;;) {
					msg_64_t m;
					int rc = mqf_read_to_canbus(&m);
					if (rc) break;
					local_msg_process(&m, 1);
				}
				tlocked = 0;
			}
		}
		return;
	}
	// not locked
	tlocked = 0;


	HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
	_send_msg_if_any(0);
	HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
}

static void bh(void)
{
	//itm_debug1(DBG_CAN, "bh-------", 0);
}

/**
  * @brief  Transmission Mailbox 0 complete callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  itm_debug1(DBG_CAN, "TxM0cplt", 0);
  _send_msg_if_any(1);
}

/**
  * @brief  Transmission Mailbox 1 complete callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  itm_debug1(DBG_CAN, "TxM1cplt", 0);
  _send_msg_if_any(1);
}

/**
  * @brief  Transmission Mailbox 2 complete callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  itm_debug1(DBG_CAN, "TxM2cplt", 0);
  _send_msg_if_any(1);
}

/**
  * @brief  Transmission Mailbox 0 Cancellation callback.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_TxMailbox0AbortCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  itm_debug1(DBG_CAN, "TxAbort0", 0);
  bh();

}

/**
  * @brief  Transmission Mailbox 1 Cancellation callback.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_TxMailbox1AbortCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  itm_debug1(DBG_CAN, "TxAbort1", 0);

  bh();

}

/**
  * @brief  Transmission Mailbox 2 Cancellation callback.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_TxMailbox2AbortCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  itm_debug1(DBG_CAN, "TxAbort2", 0);

  bh();

}

/**
  * @brief  Rx FIFO 0 message pending callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  itm_debug1(DBG_CAN, "RxF0Pend", 0);
  CAN_RxHeaderTypeDef rxHeader = {0};
  msg_64_t m;
  if (HAL_OK != HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, (uint8_t *)&m)) {
	  itm_debug1(DBG_CAN|DBG_ERR, "Rx Err", 0);
	  return;
  }
  if ((0)) {
	  if ((m.from == MA1_CTRL(1))
			  &&(m.to == MA3_UI_GEN)
			  &&(m.cmd==CMD_NOOP)
			  &&(m.v1 == 422)
			  &&(m.v2 == -4242)) {
		  itm_debug1(DBG_CAN, "RX OK", 0);
	  } else {
		  itm_debug1(DBG_CAN|DBG_ERR, "RX BAD", 0);
	  }
  }
  bh();
  itm_debug3(DBG_CAN, "RX : ", m.cmd, m.v1, m.v2);
  if ((1)) {
	  static int expn = 0;
	  if (m.cmd == CMD_NOOP) {
		  if (m.v1 != expn) {
			  itm_debug2(DBG_CAN, "exp", expn, m.v1);
		  }
		  expn = m.v1+1;
	  } else {
		  itm_debug3(DBG_CAN, "other", m.cmd, m.v1, m.v2);
	  }
  }
  flash_led();

  int rc = local_msg_process(&m, 0);
  if (!rc) mqf_write_from_canbus(&m);

}

void  HAL_CAN_TxCpltCallback(CAN_HandleTypeDef* hcan)
{
	// not called, does not exist
	UNUSED(hcan);
	itm_debug1(DBG_CAN, "TxCplt", 0);
	bh();
}
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
	// not called, does not exist
	UNUSED(hcan);
	itm_debug1(DBG_CAN, "RxCplt", 0);
	bh();
}
/**
  * @brief  Rx FIFO 0 full callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  itm_debug1(DBG_CAN, "RxF0Full", 0);
  bh();

}

/**
  * @brief  Rx FIFO 1 message pending callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
	itm_debug1(DBG_CAN, "RxF1Pend", 0);

  bh();

}

/**
  * @brief  Rx FIFO 1 full callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_RxFifo1FullCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
	itm_debug1(DBG_CAN, "RxF1Full", 0);

  bh();

}

/**
  * @brief  Sleep callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_SleepCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
	itm_debug1(DBG_CAN, "sleep", 0);

  bh();

}

/**
  * @brief  WakeUp from Rx message callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */

void HAL_CAN_WakeUpFromRxMsgCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
	itm_debug1(DBG_CAN, "wkup msg", 0);

  bh();

}

/**
  * @brief  Error CAN callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	static int errcnt = 0;
	if (errcnt++ < 500) {  // stop display error if too many
		itm_debug1(DBG_CAN, "CAN ERR", hcan->ErrorCode);
	}
	bh();
	HAL_CAN_ResetError(hcan);
}

/// --------------------------------------------------------------------------

void CAN_Tasklet(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	if ((1)) return; //XXX XXX
	static int init = 1;
	if (init) {
		init = 0;
		can_init();
	}

	send_messages_if_any();

#ifdef TRN_BOARD_DISPATCHER
	/*if ((0)) {
		static uint32_t lt = 0;
		if (!lt) {
			lt = tick;
			return;
		}
		if (tick<lt+500) return;
		lt = tick;
		msg_64_t m = {0};
		static int cnt=0;
		for (int i=0; i<3; i++) {
			m.from = MA_BROADCAST;
			m.to = MA_CONTROL();
			m.cmd = CMD_NOOP;
			m.v1 = cnt++;
			m.v2 = 1000+i;
			mqf_write_to_canbus(&m);
		}
	}*/
#endif
}

// returns non zero if message shall be skipped
static int local_msg_process(msg_64_t *m, _UNUSED_ int loc)
{
	if (m->cmd == CMD_SETRUN_MODE) {
		run_mode = (runmode_t) m->v1u;
		return 0; // always broadcast it
	}
	switch (run_mode) {
	case runmode_off:
		return 1;
	/* unlike other tasklet, default is normal mode
	 * (forward msg to CAN bus
	 */
	default:
		return 0;
	}
}



