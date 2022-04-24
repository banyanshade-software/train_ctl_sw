/*
 * taskauto.h
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

#error obsolete

#ifndef STM32_TASKAUTO_H_
#define STM32_TASKAUTO_H_

#include "task.h"
#include "../trainctl_iface.h"

extern osThreadId_t taskAutoHandle;

extern void StartTaskAuto(void *argument);




void task_auto_tick_isr(void);
void task_auto_tick(void);
void task_auto_start_auto(void);
void task_auto_stop_auto(void);



#endif /* STM32_TASKAUTO_H_ */
