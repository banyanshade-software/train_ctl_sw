/*
 * taskctrl.c
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
#include "../misc.h"
#include "../trainctl_iface.h"
#include "taskauto.h"
#include "taskctrl.h"
#include "stm32f1xx_hal.h"
//#include "../traincontrol.h"









void run_task_ctrl(void)
{
	if ((0))   calibrate_bemf(); //XXXX
	for (;;) {
		uint32_t notif;
		xTaskNotifyWait(0, 0xFFFFFFFF, &notif, portMAX_DELAY);
		//debug_info('G', 0, "HOP", 0, 0);
		static uint32_t oldt = 0;
		static uint32_t t0 = 0;
		uint32_t t = HAL_GetTick();
		if (!t0) t0 = t;
		int32_t dt = (oldt) ? (t-oldt) : 1;
		oldt = t;

		train_run_tick(notif, t, dt);
		if ((0)) { // XXX
			static int s=0;
			if (!s && (t-t0>5000)) {
				train_set_target_speed(0, 50);
				s = 1;
			}
		}
		task_auto_tick();
	}

}
