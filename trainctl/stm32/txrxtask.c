/*
 * txframe.c
 *
 *  Created on: Oct 16, 2020
 *      Author: danielbraun
 */


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#include "txrxtask.h"



#ifndef BOARD_HAS_USB
#error BOARD_HAS_USB not defined, remove this file from build
#endif


#include "cmsis_os.h"
//#include "usb_device.h"
//#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"

#include "../trainctl_iface.h"
#include "../txrxcmd.h"
//#include "main.h"
#include "misc.h"
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif



uint32_t num_msg_get = 0;
uint32_t num_msg_put = 0;
uint32_t num_msg_get_err = 0;

static void handleRxChars(frame_msg_t *m);
static void _send_bytes(uint8_t *b, int len);

void StartTxRxFrameTask(_UNUSED_ void *argument)
{
	  MX_USB_DEVICE_Init();

	if ((0)) {
		frame_msg_t tx;
		tx.t = RXFRAME_CHARS;
		tx.len = 2+4+2;
		memcpy(tx.frm, "|xT\0V__|", tx.len);
		osMessageQueuePut(frameQueueHandle, &tx, 0, portMAX_DELAY);
	}
	static frame_msg_t m;
	for (;;) {
		if ((0)) {
			osDelay(10000);
			continue;
		}
		uint8_t msg_prio;
		osStatus_t rc = osMessageQueueGet(frameQueueHandle, &m, &msg_prio, portMAX_DELAY);
		num_msg_get++;
		if ((0)) flash_led();
        usbPollQueues();
        if (rc == osErrorTimeout) continue;
		if (rc != osOK) {
			num_msg_get_err++;
			continue;
		}
		if (m.t == RXFRAME_CHARS) {
			//debug_info('G', 0, "RXFRM", m.len,0, 0);
			handleRxChars(&m);
			continue;
		}
		if (m.t == TXFRAME_TYPE_STAT) {
			uint32_t t = HAL_GetTick();   // XXX t0
			uint8_t b[]="|_NG\000X";
			//memcpy(b+6, &t, 4);
			_send_bytes(b, 6);
			frame_send_stat(_send_bytes, t);
			_send_bytes((uint8_t *)"|", 1);
			continue;
		}
		_send_bytes(m.frm, m.len);

	}
}
//void frame_send_stat(void(*cb)(uint8_t *d, int l));

static void _send_bytes(uint8_t *b, int len)
{
	for (;;) {
		uint8_t rc = CDC_Transmit_FS(b, len);
		if (rc != USBD_BUSY) break;
		osDelay(1);
	}
}

int txframe_queue_full = 0;

void txframe_send(frame_msg_t *m, int discardable)
{
	int s = osMessageQueueGetSpace(frameQueueHandle);
	if (s<=0) {
		txframe_queue_full++;
		if (discardable) return;
	}
	if ((s<=20) && discardable) {
		// we use a single queue, and no priority available with freertos
		// so we just keep some space for non discardable frames
		txframe_queue_full++;
		return;
	}
	uint32_t t = discardable ? 0 : portMAX_DELAY;
	if (m->len>FRM_MAX_LEN) m->len=FRM_MAX_LEN;
	num_msg_put++;
	osMessageQueuePut(frameQueueHandle, m, 0, t);
}

/* ------ RX ----- */

static void handleRxChars(frame_msg_t *m)
{
    frame_msg_t frresp;
	frresp.t = TXFRAME_TYPE_RESP;
	/*
	 * int rlen = FRM_MAX_LEN;
	    	frame_process(cRxedChar,m.frm, &rlen);
	 */
	for (int i=0; i<m->len; i++) {
		int rlen = FRM_MAX_LEN;
		txrx_process_char(m->frm[i], frresp.frm, &rlen);
		if (rlen>0) {
			//debug_info('G', 0, "RESP", rlen,0, 0);
			// would deadlock if we send (non discardable) through the queue
			//txframe_send_response(&frresp, rlen);
			_send_bytes(frresp.frm, rlen);
			//frresp.len = rlen;
			//_send_frm(&frresp);
		}
	}
}


extern USBD_HandleTypeDef hUsbDeviceFS;


int8_t impl_CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
	static frame_msg_t frrx;

	/*
	if ((0)) {
		sprintf(str, "[B%d]\r\n", *Len);
		CDC_Transmit_FS(str, strlen(str));
	}
	*/
	uint8_t *p = Buf;
	int32_t rlen = *Len;
	for (;;) {
		//BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		frrx.t = RXFRAME_CHARS;
		int l = MIN(rlen, RXFRAME_CHARS);
		memcpy(frrx.frm, p, l);
		rlen -= l;
		p += l;
		frrx.len = l;
		osMessageQueuePut(frameQueueHandle, &frrx, 0, 0);
		if (rlen <= 0) break;
	}

	//CDC_Transmit_FS(Buf, *Len); // ADD THIS LINE to echo back all incoming data

	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
	USBD_CDC_ReceivePacket(&hUsbDeviceFS);

	return (USBD_OK);
}

/* obsolete -------------------------------------------------------- */

/* obsolete code using FreeRTOSCli (uses too much resources and
 * no more used)
 */

#if 0
/*
 * void StartCliTask(void *argument)*/
{
  /* USER CODE BEGIN StartCliTask */
  /* Infinite loop */
	  MX_USB_DEVICE_Init();
	vRegisterCLICommands();
	static const int8_t * const pcWelcomeMessage = ( int8_t * ) "Echo for CLI\r\n\r\n";

	//Peripheral_Descriptor_t xConsole = FreeRTOS_open("CDC", NULL);
    //configASSERT( xConsole );

	int8_t cRxedChar, cInputIndex = 0;
	BaseType_t xMoreDataToFollow;
	/* The input and output buffers are declared static to keep them off the stack. */
#define MAX_OUTPUT_LENGTH 256
#define MAX_INPUT_LENGTH  256

	static int8_t pcOutputString[ MAX_OUTPUT_LENGTH ]; // only for CLI
	static int8_t pcInputString[ MAX_INPUT_LENGTH ];
	static frame_msg_t m;
	/* This code assumes the peripheral being used as the console has already
	    been opened and configured, and is passed into the task as the task
	    parameter.  Cast the task parameter to the correct type. */
	//xConsole = ( Peripheral_Descriptor_t ) pvParameters;


	/* Send a welcome message to the user knows they are connected. */
	//FreeRTOS_write( xConsole, pcWelcomeMessage, strlen( pcWelcomeMessage ) );
	//CDC_Transmit_FS(pcWelcomeMessage, sizeof(pcWelcomeMessage)-1);
    //osDelay(100);
	for( ;; ) {
		//osDelay(1000); // XXX
		//continue;

		size_t rc = xStreamBufferReceive(streamCli, &cRxedChar, 1, portMAX_DELAY);
		/*if ((0)) {
			char str[64];
			sprintf(str, "<rx:(%d)%2.2X (%2.2X)>\r\n", rc, cRxedChar, '\n');
			CDC_Transmit_FS((uint8_t *)str, strlen(str));
		}*/

	    HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

	    if (cli_frame_mode) {
	    	int rlen = FRM_MAX_LEN;
	    	frame_process(cRxedChar,m.frm, &rlen);
	    	if (rlen>0) {
	    		txframe_send_response(&m, rlen);
	    	}
	    	continue;
	    }
	    // CLI mode
		if (cRxedChar == '\n' || cRxedChar == '\r') {
			/* A newline character was received, so the input command string is
	            complete and can be processed.  Transmit a line separator, just to
	            make the output easier to read. */
#define _NL "\r\n"
			CDC_Transmit_FS( (uint8_t *)_NL, strlen( _NL ));

			/* The command interpreter is called repeatedly until it returns
	            pdFALSE.  See the "Implementing a command" documentation for an
	            exaplanation of why this is. */
			do  {
				/* Send the command string to the command interpreter.  Any
	                output generated by the command interpreter will be placed in the
	                pcOutputString buffer. */
				xMoreDataToFollow = FreeRTOS_CLIProcessCommand(
						        pcInputString,   /* The command string.*/
								pcOutputString,  /* The output buffer. */
								MAX_OUTPUT_LENGTH/* The size of the output buffer. */
						);

				/* Write the output generated by the command interpreter to the
	                console. */
				CDC_Transmit_FS((uint8_t *)pcOutputString, strlen( pcOutputString ) );

			} while( xMoreDataToFollow != pdFALSE );

			/* All the strings generated by the input command have been sent.
	            Processing of the command is complete.  Clear the input string ready
	            to receive the next command. */
			cInputIndex = 0;
			memset( pcInputString, 0x00, MAX_INPUT_LENGTH );
			CDC_Transmit_FS((uint8_t *)pcWelcomeMessage, strlen(pcWelcomeMessage));

		} else {
			/* The if() clause performs the processing after a newline character
	            is received.  This else clause performs the processing if any other
	            character is received. */
			CDC_Transmit_FS(&cRxedChar, 1); //  echo

			if( cRxedChar == '\r' ) {
				CDC_Transmit_FS( (uint8_t *)"CR\r\n", strlen( "CR\r\n"));
				/* Ignore carriage returns. */
			} else if( cRxedChar == '\b' ) {
				/* Backspace was pressed.  Erase the last character in the input
	                buffer - if there are any. */
				if( cInputIndex > 0 ) {
					cInputIndex--;
					pcInputString[ cInputIndex ] = '\0';
				}
			} else {
				/* A character was entered.  It was not a new line, backspace
	                or carriage return, so it is accepted as part of the input and
	                placed into the input buffer.  When a n is entered the complete
	                string will be passed to the command interpreter. */
				if( cInputIndex < MAX_INPUT_LENGTH ) {
					pcInputString[ cInputIndex ] = cRxedChar;
					cInputIndex++;
				}
			}
		}
	}

  /* USER CODE END StartCliTask */
}


#endif




