/*
 * taskled.c
 *
 *  Created on: Oct 24, 2021
 *      Author: danielbraun
 */



#include "cmsis_os.h"
#include "main.h"
#include "task.h"
#include "taskled.h"

#include "../misc.h"
#include "../leds/ledtask.h"
#include "../msg/notif.h"
#include "../msg/tasklet.h"

#ifdef BOARD_HAS_SERVOS
#include "../low/servo.h"
#endif


#ifndef BOARD_HAS_LED
#error BOARD_HAS_LED not defined, remove this file from build
#endif

static tasklet_t *ledTasklets[] = {
		&led_tasklet,

#ifdef BOARD_HAS_SERVOS
#if SERVO_RUNS_ON_LED_TASK
		&servo_tasklet,
#endif
#endif
		NULL
};

void start_led_task(_UNUSED_ void *argument)
{

  uint32_t prevt = xTaskGetTickCount();

  for(;;)  {
#if 0
	  uint32_t notif;
	  xTaskNotifyWait(0, 0xFFFFFFFF, &notif, portMAX_DELAY);
#endif
	  vTaskDelayUntil(&prevt, 4); // 3=>333 Hz 4=>250 Hz 5=> 200 Hz 8=>125 Hz
	  // 200Hz is minimal to have 25% on PWM without flickering
	  // 250Hz selected

	  //static uint32_t oldt = 0;
	  //static uint32_t t0 = 0;
	  //uint32_t t = HAL_GetTick();
	  // XXX we should have a global t0
	  //if (!t0) t0 = t;
	  //int32_t dt = (oldt) ? (t-oldt) : 1;
	  //oldt = t;
	 //led_tasklet(NOTIF_SYSTICK, t, dt);
	  //tasklet_run(&led_tasklet, t);

	  tasklet_run_all(ledTasklets);
  }
  /* USER CODE END start_led_task */
}
