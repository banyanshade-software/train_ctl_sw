/*
 * oamtask.c
 *
 *  Created on: Apr 15, 2022
 *      Author: danielbraun
 */




#include "cmsis_os.h"
#include "main.h"
#include "task.h"
#include "taskctrl.h"

#include "../misc.h"
#include "../msg/trainmsg.h"

#include "../oam/oam.h"
//#include "../../w25qxx/w25qxx.h"

//extern osThreadId_t taskOamHandle;

void StartOamTask(_UNUSED_ void *argument)
{

	OAM_Init();

	uint32_t lt = 0;
	for (;;) {
		osDelay(OAM_NeedsReschedule ? 1 : 20);
		uint32_t tick = HAL_GetTick();
		uint32_t notif = 0;

		//itm_debug1(DBG_LOWCTRL, "--msg", dt);
		msgsrv_tick(notif, tick, tick-lt);

		//itm_debug1(DBG_LOWCTRL, "--oam", dt);
		OAM_Tasklet(notif, tick, tick-lt);

		lt = tick;
	}
}
