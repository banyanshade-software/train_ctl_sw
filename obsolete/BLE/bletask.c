/*
 * bletask.c
 *
 *  Created on: Nov 19, 2020
 *      Author: danielbraun
 */

#include <memory.h>
#include <stdio.h>
#include "cmsis_os.h"
#include "bletask.h"

#ifdef HAL_UART_ERROR_NONE
// UART defined in .ioc file

static UART_HandleTypeDef *huart;
static DMA_HandleTypeDef  *hdma_uart_rx;
static DMA_HandleTypeDef  *hdma_uart_tx;
static osMessageQueueId_t bleRespQ;


static volatile int tx_on_progress = 0;
static volatile int rx_on_progress = 0;

static uint8_t buf0[32];
static uint8_t buf1[32];
static uint8_t curbuf;

typedef struct {
	const char *cmd;
	const char *resp;
	uint8_t flags;
} at_command_t;


static int mode_lofi = 1;

#define FLAG_BAUDS 	0x01
#define FLAG_PASS	0x02
#define FLAG_SLEEP  0x03

static const at_command_t cmds[] = {
		/* 00 */ { "AT\r\n", "OK" , 0},
		//{ "AT+VERSION\r\n", "xx" , 0},
		/* 01 */ { "AT\r\n", "OK" , 0},

		//{ "AT+RESET\r\n", "+RESET\r\nOK",	FLAG_SLEEP},
		//{ "AT+BAUD4\n", "+BAUD=4",	FLAG_BAUDS},
		/* 02 */ { "AT\r\n", "OK", 0 },
		/* 03 */ { "AT+NAMEtrainctl\r\n", "+NAME=trainctl\r\nOK", 0},
		/* 04 */ { "AT+PASS123456\r\n", "+PASS=123456\r\nOK\r\n", 	0},
		{ NULL, NULL}
};

static void bh(void);
volatile int num_cmd = 0;
static int num_qget = 0;



static void process_lofi(uint8_t *msg, int len);


void RunBleTask(UART_HandleTypeDef *_uart, DMA_HandleTypeDef *_hdma_uart_rx, DMA_HandleTypeDef *_hdma_uart_tx, osMessageQueueId_t _bleRespQ)
{
	huart = _uart;
	hdma_uart_rx = _hdma_uart_rx;
	hdma_uart_tx = _hdma_uart_tx;
	bleRespQ     = _bleRespQ;

	curbuf = 0;
	HAL_UART_Receive_DMA(huart, buf0, sizeof(buf0));
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE );
	__HAL_DMA_ENABLE_IT(hdma_uart_rx, DMA_IT_TC /*|DMA_IT_HT*/);
	//rx_on_progress = 1;

	printf("BLE start\n");
	if ((1)) {
		for (;;) {
			uint8_t msg_prio;
			at_msg_t m;
			osStatus_t rc = osMessageQueueGet(bleRespQ, &m, &msg_prio, 300);
			if (rc == osOK) {
				continue;
			}
			break;
		}
	}
	int ncmd = 0;
	for (;;) {
		const at_command_t *pat = &cmds[ncmd];
		printf("BLE cmd %d\n", ncmd);
		if (!pat->cmd) break;
		HAL_UART_Transmit_DMA(huart, (uint8_t *)pat->cmd, strlen(pat->cmd));
		uint8_t msg_prio;
		at_msg_t m;
		osStatus_t rc = osMessageQueueGet(bleRespQ, &m, &msg_prio, portMAX_DELAY);
		if (rc != osOK) {
			//num_msg_get_err++;
			bh();
			osDelay(500);
			continue;
		}
		num_qget++;
		//uint8_t *pass;

		uint8_t *msg = m.numbuf ? buf1 : buf0;
		// process response
		if (m.len<sizeof(buf0)) msg[m.len]='\0';
		int el = strlen(pat->resp);
		if (memcmp(msg, pat->resp, el)) {
			// failed ?
			bh();
			osDelay(500);
			continue;
		}
		// response flags
		switch (pat->flags) {
		case FLAG_SLEEP:
			osDelay(1000);
			break;
		case FLAG_BAUDS:
			/*
			if (HAL_UART_DeInit(huart) != HAL_OK) {
				Error_Handler();
			}
			huart->Init.BaudRate = 115200;
			if (HAL_UART_Init(huart) != HAL_OK) {
				Error_Handler();
			}*/
			//https://stackoverflow.com/questions/57283327/how-to-change-the-uart-baud-rate-after-running-on-stm32-board#58970240
			huart->Instance->CR1 &= ~(USART_CR1_UE);
			uint32_t pclk = HAL_RCC_GetPCLK1Freq();
			huart->Instance->BRR = UART_BRR_SAMPLING8(pclk, 115200);
			huart->Instance->CR1 |= USART_CR1_UE;
			osDelay(1000);
			break;
		case FLAG_PASS:
			//pass = msg+el;
			break;
		}
		ncmd++;
		num_cmd++;
	}
	// init done
	bh();
	osDelay(200);
#define MSG "0123456789\r\n"
	HAL_UART_Transmit_DMA(huart, (uint8_t *) MSG, strlen(MSG));
	for (;;) {
		uint8_t msg_prio;
		at_msg_t m;
		osStatus_t rc = osMessageQueueGet(bleRespQ, &m, &msg_prio, portMAX_DELAY);
		//bh();
		if (rc != osOK) {
			//num_msg_get_err++;
			osDelay(500);
			continue;
		}
		uint8_t *msg = m.numbuf ? buf1 : buf0;
		// process response
		if (mode_lofi) {
			process_lofi(msg, m.len);
		} else {
			if (m.len<sizeof(buf0)) msg[m.len]='\0';
		                 //012345678
			static char str[]="resp x ok";
			static int cnt = 0;
			str[5] = '0'+(cnt++ % 10);
			HAL_UART_Transmit_DMA(huart, (uint8_t *)str, strlen(str));
		}
	}

}

void bh(void)
{

}

static void bh2(void)
{

}

volatile int8_t ble_spd1;
volatile int8_t ble_spd2;

// https://github.com/generationmake/LOFI_on_Bluefruit_nRF52/blob/master/LOFI_on_Bluefruit_nRF52.ino

static void process_lofi(uint8_t *msg, int len)
{
	for (int i=0; i<len; i+=2) {
		int8_t v = (int8_t)(msg[i+1]);
		printf("  %d=%d ", msg[i], v);
		if (v) bh();
		switch (msg[i]) {
		case 202: // motor1 //
			ble_spd1 = v;
			break;
		case 203: // motor2 // fwd
			ble_spd2 = v;
			break;
		case 204: // output1 // btn B
		case 205: // output2
		case 206: // output3
		case 207: // output4
			break;
		case 208: // servo output1
			break;
		case 209: // servo output2
		case 210: // servo output3
		case 211: // servo output4
		case 212: // ? 99
			break;
		case 201:  // button A (val 1)
			break;
		default:
			bh2();
			break;
		}
	}
	printf("\n");
}

/*void HAL_UART_IRQHandler(UART_HandleTypeDef *huart)
{
	bh();
}*/

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	tx_on_progress=0;
	//bh();
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	//bh();
}

static int num_empty_rx = 0;
volatile int num_rx = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	rx_on_progress=0;
	int l = sizeof(buf0) - __HAL_DMA_GET_COUNTER(hdma_uart_rx); // buf0 and buf1 same size
	at_msg_t m;
	m.numbuf = curbuf;
	m.len = l;
	num_rx++;
	if (l) {
		curbuf = curbuf ? 0 : 1;
		osMessageQueuePut(bleRespQ, &m, 0, 0);
	} else {
		num_empty_rx++;
	}
	uint8_t *newbuf = curbuf ? buf1 : buf0;

	HAL_UART_Receive_DMA(huart, newbuf, sizeof(buf0));
	//__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE );
	//__HAL_DMA_ENABLE_IT(hdma_uart_rx, DMA_IT_TC /*|DMA_IT_HT*/);
}
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	//bh();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	bh();
}
void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart)
{
	bh();
}

void HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef *huart)
{
	bh();
}
void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart)
{
	bh();
}

#endif
