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




extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim12;
extern ADC_HandleTypeDef hadc1;

extern osThreadId_t taskCtrlHandle;

//void run_task_ctrl(void);
void StartCtrlTask(void *argument);


void set_pwm_freq(int freqhz);

extern int cur_freqhz;

#endif /* STM32_TASKCTRL_H_ */
