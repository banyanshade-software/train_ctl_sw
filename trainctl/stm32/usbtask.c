/*
 * usbtask.c
 *
 *  Created on: 23 avr. 2022
 *      Author: danielbraun
 */



#include "cmsis_os.h"
#include "trainctl_config.h"

#ifndef BOARD_HAS_USB
#error board has no usb, remove from build
#endif

#include "usbd_cdc_if.h"
#include "usb_device.h"

#include "../trainctl_iface.h"
//#include "../txrxcmd.h"
#include "../misc.h"

#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif


#include "../msg/trainmsg.h"
#include "../msg/tasklet.h"

#include "utils/framing.h"
#include "usbtask.h"

#include "main.h"
#include "stm32f4xx_hal_gpio.h"

extern USBD_HandleTypeDef hUsbDeviceFS;



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


// ------------------------------------------------------


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

static void USB_Init(void)
{
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


__weak void frame_send_oscillo(_UNUSED_ void(*cb)(const uint8_t *d, int l))
{
}
__weak void frame_send_stat(_UNUSED_ void(*cb)(const uint8_t *d, int l), _UNUSED_ uint32_t tick)
{
}


static int initdone = 1;
static volatile int sending_oscillo = 0;
static volatile int sending_stats = 0;

void USB_Tasklet(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	if (!initdone) {
		USB_Init();
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
			sending_stats = 1;
			const uint8_t b[]="|S";
			_send_bytes(b, 6);
			frame_send_stat(_send_bytes, t);
			_send_bytes((uint8_t *)"|", 1);
			sending_stats = 0;
			}
			break;
		case CMD_USB_OSCILLO: {
			sending_oscillo = 1;
			const uint8_t b[]="|V"; //
			_send_bytes(b, 6);
			frame_send_oscillo(_send_bytes);
			_send_bytes((uint8_t *)"|", 1);
			sending_oscillo = 0;
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
	return 0; // XXX
	if (sending_stats) return 0;
	if (sending_oscillo) return 0;
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

