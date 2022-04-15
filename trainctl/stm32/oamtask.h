/*
 * oamtask.h
 *
 *  Created on: Apr 15, 2022
 *      Author: danielbraun
 */

#ifndef STM32_OAMTASK_H_
#define STM32_OAMTASK_H_


extern osThreadId_t taskOamHandle;

void StartOamTask(void *argument);


#endif /* STM32_OAMTASK_H_ */
