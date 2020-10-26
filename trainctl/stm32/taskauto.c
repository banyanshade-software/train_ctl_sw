/*
 * taskauto.c
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


#include "cmsis_os.h"
#include "main.h"
#include "task.h"

#include "../misc.h"
#include "taskauto.h"
#include "../auto1.h"

extern void StartTaskAuto(void *argument)
{
	uint32_t t0;
	t0 = HAL_GetTick();
	 for (;;) {
		  uint32_t notif;
		  xTaskNotifyWait(0, 0xFFFFFFFF, &notif, portMAX_DELAY);
		  uint32_t t = HAL_GetTick() - t0;
		  auto1_run(notif, t);
	 }
}

void task_auto_tick_isr(void)
{
	BaseType_t higher=0;
	xTaskNotifyFromISR(taskAutoHandle, AUTO1_NOTIF_TICK, eSetBits, &higher);
    portYIELD_FROM_ISR(higher);
}


void task_auto_tick(void)
{
	xTaskNotify(taskAutoHandle, AUTO1_NOTIF_TICK, eSetBits);
}

void task_auto_start_auto(void)
{
	xTaskNotify(taskAutoHandle, AUTO1_NOTIF_CMD_START, eSetBits);
}

void task_auto_stop_auto(void)
{
	xTaskNotify(taskAutoHandle, AUTO1_NOTIF_CMD_STOP, eSetBits);
}
