/*
 * bletask.h
 *
 *  Created on: Nov 19, 2020
 *      Author: danielbraun
 */

#ifndef BLETASK_H_
#define BLETASK_H_

#include "main.h"

void RunBleTask(UART_HandleTypeDef *_uart, DMA_HandleTypeDef *_hdma_uart_rx, DMA_HandleTypeDef *_hdma_uart_tx,
		osMessageQueueId_t hbleRespQueue);


typedef struct {
	uint8_t numbuf; // buf0 or buf1
	uint8_t len;
} at_msg_t;

extern volatile int num_cmd;
extern volatile int num_rx;
extern volatile int8_t ble_spd1;
extern volatile int8_t ble_spd2;

#endif /* BLETASK_H_ */
