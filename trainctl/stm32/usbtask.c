/*
 * usbtask.c
 *
 *  Created on: 23 avr. 2022
 *      Author: danielbraun
 */



#include "cmsis_os.h"
#include "trainctl_config.h"


#if defined(BOARD_HAS_USB)

#include "usbd_cdc_if.h"
#include "usb_device.h"

#elif defined (BOARD_HAS_LPUART)
#else
#error board has no usb, remove from build
#endif

#include "../trainctl_iface.h"
#include "../misc.h"



#if defined(STM32F4)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

#elif defined(STM32G4)
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_gpio.h"

#elif defined(STM32F1)
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"

#else
#error no board hal
#endif

#ifdef BOARD_HAS_LPUART
#if defined(STM32_G4)
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_lpuart.h"

#else
#error only tested for G4
#endif
#endif // BOARD_HAS_LPUART



#include "../msg/trainmsg.h"
#include "../msg/tasklet.h"

#include "utils/framing.h"
#include "usbtask.h"
#include "../msg/msgrecord.h"

#include "main.h"

#if defined(BOARD_HAS_USB)
extern USBD_HandleTypeDef hUsbDeviceFS;
#elif defined(BOARD_HAS_LPUART)
extern UART_HandleTypeDef hlpuart1;
#endif

// ------------------------------------------------------
static void stat_poll(uint32_t t, uint32_t dt);

static const tasklet_def_t stattx_tdef = {
		.init 				= NULL,
		.poll_divisor		= NULL,
		.emergency_stop 	= NULL,
		.enter_runmode		= NULL,
		.pre_tick_handler	= NULL,
		.default_msg_handler = NULL,
		.default_tick_handler = stat_poll,
		.msg_handler_for	= NULL,
		.tick_handler_for 	= NULL

};
tasklet_t stattx_tasklet = { .def = &stattx_tdef, .init_done = 0, .queue=NULL};


static void USB_UART_Init(void);

static void _send_bytes(const uint8_t *b, int len);

#ifdef BOARD_HAS_LPUART
extern osThreadId usbTaskHandle;
#endif


// ------------------------------------------------------
// USB specific
// ------------------------------------------------------

#ifdef BOARD_HAS_USB

static void Force_USB_Enumerate(void)
{
	// https://stackoverflow.com/questions/20195175/stm32f107-usb-re-enumerate#20197455
#ifdef STM32F4
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GPIO_PIN_12;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	osDelay(5);
#else
#error check USB DP  is PA12
#endif
}



int8_t impl_CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
	static int pendlen = -1;
	static uint8_t pendmsg[8];

	uint8_t *p = Buf;
	int32_t rlen = *Len;
	for (; rlen > 0 ; p++, rlen--) {
		if (pendlen == -1) {
			if (*p == 0x7E) {
				pendlen = 0;
			}
			continue;
		}
		pendmsg[pendlen++] = *p;
		if (pendlen == 8) {
			// full msg
			msg_64_t msg;
			pendlen = -1;
			memcpy(&msg, pendmsg, 8);
			//if (msg.cmd == CMD_USB_RECORD_MSG) {
			//	itm_debug1(DBG_ERR, "plop", 0);
			//}
			mqf_write_from_usb(&msg);
		}
	}
	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
	USBD_CDC_ReceivePacket(&hUsbDeviceFS);

	return (USBD_OK);
}

static void _send_bytes(const uint8_t *b, int len)
{
	uint8_t rc;
	if (!CDC_Class_Init_Ok()) {
		return;
	}
	for (;;) {
		rc = CDC_Transmit_FS((uint8_t *)b, len);
		if (rc != USBD_BUSY) break;
		osDelay(2);
	}
	if (rc != USBD_OK) {
		itm_debug2(DBG_ERR|DBG_USB, "TXerr", len, rc);
	}
}

void StartUsbTask(_UNUSED_ const void *argument)
{
	Force_USB_Enumerate();
	MX_USB_DEVICE_Init();
	//CDC_Init_FS();
	//USBD_CDC_Init()
	for (;;) {
		osDelay(10);
		USB_Tasklet(0, 0, 0);
	}
}
#endif




// ------------------------------------------------------
// UART specific
// ------------------------------------------------------

#ifdef BOARD_HAS_LPUART


extern DMA_HandleTypeDef hdma_lpuart1_rx;
extern DMA_HandleTypeDef hdma_lpuart1_tx;

static void _start_rx(int);

void StartUsbTask(_UNUSED_ const void *argument)
{
	_start_rx(0);
	for (;;) {
		osDelay(10);
		USB_Tasklet(0, 0, 0);
	}
}

static uint8_t txbuf[128];
volatile uint8_t txonprogress = 0;

static uint8_t rxbuf[9]; // msg_64 + framing


static void _start_rx(int offset)
{
	//HAL_UART_Receive_DMA(&hlpuart1, rxbuf, 2*sizeof(msg_64_t));
	HAL_StatusTypeDef rc = UART_Start_Receive_IT(&hlpuart1, rxbuf+offset, sizeof(rxbuf)-offset);
	if (rc != HAL_OK) {
		itm_debug2(DBG_USB|DBG_ERR, "Ustrt", rc, hlpuart1.ErrorCode);
	}

}

static void _send_bytes(const uint8_t *b, int len)
{
	//if ((1)) return; //XXX

	if ((len<=0) || (len>(int)sizeof(txbuf))) {
		FatalError("sndbyt", "bad len for _send_bytes", Error_Uart_Len);
	}
	while (txonprogress) {
		uint32_t notif = 0;
		xTaskNotifyWait(0, 0xFFFFFFFF, &notif, portMAX_DELAY);
	}
 	memcpy(txbuf, b, len);
 	txonprogress = 1;
 	HAL_StatusTypeDef rc;
 	if ((0)) {
 		rc = HAL_UART_Transmit_DMA(&hlpuart1, (const uint8_t *)"hello world---", 12);
 	} else {
 		rc = HAL_UART_Transmit_DMA(&hlpuart1, txbuf, len);
 	}
	if (rc != HAL_OK) {
		itm_debug1(DBG_USB|DBG_ERR, "TxErr", rc);
	 	txonprogress = 0;
		return;
	}

}

//void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(_UNUSED_ UART_HandleTypeDef *huart)
{
	txonprogress = 0;
	BaseType_t higher=0;
	xTaskNotifyFromISR(usbTaskHandle, NOTIF_UART_TX, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
}
/*
void HAL_UART_RxHalfCpltCallback(_UNUSED_ UART_HandleTypeDef *huart)
{
}
*/

static void bh(void)
{

}
void HAL_UARTEx_RxFifoFullCallback(_UNUSED_  UART_HandleTypeDef *huart)
{
	itm_debug1(DBG_USB, "RxFifoFull", 0);
	bh();
}
void HAL_UART_RxHalfCpltCallback(_UNUSED_ UART_HandleTypeDef *huart)
{
	itm_debug1(DBG_USB, "RxHalfCplt", 0);
	bh();
}
void HAL_UART_RxCpltCallback(_UNUSED_ UART_HandleTypeDef *huart)
{
	//itm_debug2(DBG_USB, "Rx", huart->RxXferCount, huart->RxXferSize);
	//itm_debug1(DBG_USB, "RxCplt", 0);
	// 9 bytes received
	int offset = 0;
	if (FRAME_M64==rxbuf[0]) {
		// normal frame, aligned
		msg_64_t m;
		memcpy(&m, rxbuf+1, 8);
		itm_debug1(DBG_USB, "msg8", m.cmd);
		mqf_write_from_usb(&m);
	} else {
		// frame is unaligned
		for (int i=1;i<9; i++) {
			if (rxbuf[i]==FRAME_M64) {
				offset = i;
				memmove(rxbuf, rxbuf+offset, 9-offset);
			}
		}
		itm_debug1(DBG_ERR|DBG_USB, "unalgn", offset);
	}
	bh();
	_start_rx(offset);
}
void HAL_UARTEx_RxEventCallback(_UNUSED_ UART_HandleTypeDef *huart, _UNUSED_ uint16_t Size)
{
	itm_debug1(DBG_USB, "RxEvent", Size);
	bh();
}
void HAL_UART_ErrorCallback(_UNUSED_ UART_HandleTypeDef *huart)
{
	itm_debug1(DBG_USB, "RxErr", huart->ErrorCode);
	if (__HAL_UART_GET_FLAG(huart, UART_FLAG_PE) != RESET) {
		__HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_PEF); /* Clear PE flag */
	} else if (__HAL_UART_GET_FLAG(huart, UART_FLAG_FE) != RESET) {
		__HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_FEF); /* Clear FE flag */
	} else if (__HAL_UART_GET_FLAG(huart, UART_FLAG_NE) != RESET) {
		__HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_NEF); /* Clear NE flag */
	} else if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) != RESET) {
		__HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF); /* Clear ORE flag */
	}
	_start_rx(0);
}


// htim8

#endif




// ------------------------------------------------------
// generic part, both usb and uart
// ------------------------------------------------------


static void USB_UART_Init(void)
{
}
__weak void frame_send_oscillo(_UNUSED_ void(*cb)(const uint8_t *d, int l))
{
}
__weak void frame_send_stat(_UNUSED_ void(*cb)(const uint8_t *d, int l), _UNUSED_ uint32_t tick)
{
}
__weak void frame_send_recordmsg(_UNUSED_ void(*cb)(const uint8_t *d, int l))
{
}
__weak void frame_send_trace(_UNUSED_ void(*cb)(const uint8_t *d, int l), _UNUSED_ int train)
{
}

static int initdone = 1;

enum special_txmode {
	txnormal = 0,
	sending_stats,
	sending_oscillo,
	sending_recordmsg
};

static volatile enum special_txmode sending_longframe = txnormal;


void USB_Tasklet(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	if (!initdone) {
		USB_UART_Init();
	}
	static uint8_t buf[9];
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_usb(&m);
		if (rc) break;

		uint32_t t = HAL_GetTick();

		switch (m.cmd) {
		default:
			// forward packet
			buf[0] = FRAME_M64;
			memcpy(buf+1, &m, 8);
			_send_bytes(buf, 9);
			break;
		case CMD_USB_STATS: {
			if (sending_longframe != txnormal) break;
			sending_longframe = sending_stats;
			const uint8_t b[]="|S";
			_send_bytes(b, 2);
			frame_send_stat(_send_bytes, t);
			_send_bytes((uint8_t *)"|", 1);
			sending_longframe = txnormal;
			}
			break;
		case CMD_USB_TRACETRAIN: {
			if (sending_longframe != txnormal) break;
			sending_longframe = sending_oscillo;
			const uint8_t b[]="|K";
			_send_bytes(b, 2);
			frame_send_trace(_send_bytes,  m.v1);
			_send_bytes((uint8_t *)"|", 1);
			sending_longframe = txnormal;
			}
			break;
		case CMD_USB_OSCILLO: {
			if (sending_longframe != txnormal) break;
			sending_longframe = sending_oscillo;
			const uint8_t b[]="|V"; //
			_send_bytes(b, 2);
			frame_send_oscillo(_send_bytes);
			_send_bytes((uint8_t *)"|", 1);
			sending_longframe = txnormal;
			}
			break;
		case CMD_USB_RECORD_MSG: {
			if (sending_longframe != txnormal) break;
			sending_longframe = sending_oscillo;
			const uint8_t b[]="|M"; //
			_send_bytes(b, 2);
			frame_send_recordmsg(_send_bytes);
			_send_bytes((uint8_t *)"|", 1);
			sending_longframe = txnormal;
			}
			break;
		}
	}
}



__weak int oscillo_running(void)
{
	return 0;
}

static int can_send_stat(void)
{
	//return 0; // XXX
	if (sending_longframe) return 0;
	if (oscillo_running()) return 0;
	return 1;
}



static void stat_poll(_UNUSED_ uint32_t t, _UNUSED_ uint32_t dt)
{
	int st = can_send_stat();
	if (st) {
		msg_64_t m = {0};
		m.from = MA1_CONTROL();
		m.to = MA2_USB_LOCAL;
		m.cmd = CMD_USB_STATS;
        mqf_write_from_ctrl(&m);
	}
}

