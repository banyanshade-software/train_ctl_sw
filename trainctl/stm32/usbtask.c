/*
 * usbtask.c
 *
 *  Created on: 23 avr. 2022
 *      Author: danielbraun
 */



#include "cmsis_os.h"

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

#include "utils/framing.h"
#include "usbtask.h"

extern USBD_HandleTypeDef hUsbDeviceFS;


void StartTxRxFrameTask(_UNUSED_ void *argument)
{
	for (;;) {

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
	for (; rlen > 0 ; p++) {
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
	for (;;) {
		uint8_t rc = CDC_Transmit_FS((uint8_t *)b, len);
		if (rc != USBD_BUSY) break;
		osDelay(1);
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
			buf[0] = 0x7E;
			memcpy(buf+1, &m, 8);
			_send_bytes(buf, 9);
			break;
		case CMD_USB_STATS: {
			sending_stats = 1;
			const uint8_t b[]="|_NG\000X";
			_send_bytes(b, 6);
			frame_send_stat(_send_bytes, t);
			_send_bytes((uint8_t *)"|", 1);
			sending_stats = 0;
			}
			break;
		case CMD_USB_OSCILLO: {
			sending_oscillo = 1;
			const uint8_t b[]="|_NG\000Y"; //
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

int can_send_stat(void)
{
	if (sending_stats) return 0;
	if (sending_oscillo) return 0;
	if (oscillo_running()) return 0;
	return 1;
}

