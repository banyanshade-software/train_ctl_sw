/*
 * taskctrl.h
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

#ifndef STM32_TASKCTRL_H_
#define STM32_TASKCTRL_H_



extern osThreadId_t taskCtrlHandle;

void run_task_ctrl(void);



#endif /* STM32_TASKCTRL_H_ */
