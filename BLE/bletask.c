/*
 * bletask.c
 *
 *  Created on: Nov 19, 2020
 *      Author: danielbraun
 */

#include <memory.h>

#include "cmsis_os.h"
#include "bletask.h"


static UART_HandleTypeDef *huart;
static DMA_HandleTypeDef  *hdma_uart_rx;
static DMA_HandleTypeDef  *hdma_uart_tx;



static volatile int tx_on_progress = 0;
static volatile int rx_on_progress = 0;

static uint8_t buf[128];



void RunBleTask(UART_HandleTypeDef *_uart, DMA_HandleTypeDef *_hdma_uart_rx, DMA_HandleTypeDef *_hdma_uart_tx)
{
	huart = _uart;
	hdma_uart_rx = _hdma_uart_rx;
	hdma_uart_tx = _hdma_uart_tx;

	for (;;) {
#define MSG "AT\r\nAT+NAMEtrain\r\nAT+PASS?\r\nAT+ADDR?\r\nAT+ADVI9\r\nAT+ADTY0\r\n"
		//HAL_UART_Transmit(&huart4, MSG, sizeof(MSG), 100);
		__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE );
		__HAL_DMA_ENABLE_IT(hdma_uart_rx, DMA_IT_TC);
		tx_on_progress=1;
		HAL_UART_Transmit_DMA(huart, (uint8_t *)MSG, strlen(MSG));
		while (tx_on_progress) {

		}
		/*if (rx_on_progress) {
			osDelay(100);
			continue;
		}*/
		rx_on_progress = 1;
		HAL_UART_Receive_DMA(huart, buf, sizeof(buf));
		osDelay(3000);
	}

}


static void bh(void)
{

}
/*void HAL_UART_IRQHandler(UART_HandleTypeDef *huart)
{
	bh();
}*/

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	tx_on_progress=0;
	bh();
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	//bh();
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	rx_on_progress=0;
	int l = sizeof(buf) - __HAL_DMA_GET_COUNTER(hdma_uart_rx);
	bh();
}
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	bh();
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

