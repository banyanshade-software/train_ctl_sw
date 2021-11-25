/*
 * cantask.c
 *
 *  Created on: Nov 22, 2021
 *      Author: danielbraun
 */



#include <stm32/canmsg.h>
#include "cmsis_os.h"

#include "../txrxcmd.h"
//#include "main.h"
#include "misc.h"
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#include "../msg/trainmsg.h"

#include "canmsg.h"

extern CAN_HandleTypeDef hcan1;

void can_init(void)
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

	if (HAL_CAN_ConfigFilter(&hcan1, &canfil)) {
		Error_Handler();
	}



	if (HAL_CAN_Start(&hcan1) != HAL_OK) {
		/* Start Error */
		Error_Handler();
	}

	/* Activate CAN RX notification */
	if (HAL_CAN_ActivateNotification(&hcan1,
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
		Error_Handler();
	}


}

void StartCanTask(_UNUSED_ void *argument)
{
	can_init();
}

static uint32_t arbitration_id(msg_64_t *m)
{
	// 0x7FF 11 bits
	// 3 bits priority of msg, high 3 bits of m->cmd
	// 6 bits board num, currently 0 0 b b b 0
	// 2 bits counter
	// p p p  0 0 b b b 0 n n

	static uint8_t cnt = 0;
	uint32_t aid = (m->cmd & 0xE0) >> 3;
	aid |= (m->to & MA_ADDR_MASK_BOARD) << 1;
	aid |= cnt & 0x03;
	cnt ++;
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

	if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, (uint8_t *)msg, &TxMailbox) != HAL_OK) {
		//printf("Error: CAN can't send msg.\r\n");
		Error_Handler();
		return 1;
	}
	if (f) itm_debug3(DBG_CAN, "Tx/I", msg->v1, msg->v2, TxMailbox);
	else   itm_debug3(DBG_CAN, "Tx/p", msg->v1, msg->v2, TxMailbox);
	return 0;
}
// HAL_NVIC_SetPriority SVCall

void CanTest(void)
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
		if (HAL_CAN_IsTxMessagePending(&hcan1, TxMailbox)) {
			itm_debug2(DBG_CAN, "sent", TxMailbox, i);
			break;
		}
		osDelay(10);
	}
	if (i==100) {
		itm_debug1(DBG_CAN|DBG_ERR, "CAN KO", TxMailbox);
	}
}


static void _send_msg_if_any(int fromirq)
{
	for (int i = 0; i<3; i++) {
		if (!HAL_CAN_GetTxMailboxesFreeLevel(&hcan1)) return;
		msg_64_t m;
		int rc = mqf_read_to_canbus(&m);
		if (rc) return;
		_can_send_msg(&m, fromirq);
	}
}

static void send_messages_if_any(void)
{
	if (!mqf_len(&to_canbus)) return;
	if (!HAL_CAN_GetTxMailboxesFreeLevel(&hcan1)) return;

	HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
	_send_msg_if_any(0);
	HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
}

static void bh(void)
{
	itm_debug1(DBG_CAN, "bh-------", 0);
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
	  if ((m.from == MA_CONTROL_T(1))
			  &&(m.to == MA_UI(0))
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
  mqf_write_from_canbus(&m);


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
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
	itm_debug1(DBG_CAN, "CAN ERR", 0);

  bh();

}

/// --------------------------------------------------------------------------

void CAN_Tasklet(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	static int init = 1;
	if (init) {
		init = 0;
		can_init();
	}
	send_messages_if_any();
	if ((1)) {
		static int lt = 0;
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
	}
}



