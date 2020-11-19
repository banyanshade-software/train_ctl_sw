/*
 * bletask.h
 *
 *  Created on: Nov 19, 2020
 *      Author: danielbraun
 */

#ifndef BLETASK_H_
#define BLETASK_H_

#include "main.h"

void RunBleTask(UART_HandleTypeDef *_uart, DMA_HandleTypeDef *_hdma_uart_rx, DMA_HandleTypeDef *_hdma_uart_tx);



#endif /* BLETASK_H_ */
