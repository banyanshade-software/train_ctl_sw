/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define VOLT_4_SEL0_Pin GPIO_PIN_2
#define VOLT_4_SEL0_GPIO_Port GPIOE
#define VOLT_4_SEL1_Pin GPIO_PIN_3
#define VOLT_4_SEL1_GPIO_Port GPIOE
#define USER_BTN_Pin GPIO_PIN_4
#define USER_BTN_GPIO_Port GPIOE
#define VOLT_4_SEL2_Pin GPIO_PIN_5
#define VOLT_4_SEL2_GPIO_Port GPIOE
#define ADC_3_V1_Pin GPIO_PIN_0
#define ADC_3_V1_GPIO_Port GPIOC
#define ADC_4_V0_Pin GPIO_PIN_1
#define ADC_4_V0_GPIO_Port GPIOC
#define ADC_4_V1_Pin GPIO_PIN_2
#define ADC_4_V1_GPIO_Port GPIOC
#define ADC_5_V0_Pin GPIO_PIN_3
#define ADC_5_V0_GPIO_Port GPIOC
#define ADC_2_V1_Pin GPIO_PIN_5
#define ADC_2_V1_GPIO_Port GPIOA
#define BOARD_LED_Pin GPIO_PIN_6
#define BOARD_LED_GPIO_Port GPIOA
#define LED0_Pin GPIO_PIN_7
#define LED0_GPIO_Port GPIOA
#define ADC_5_V1_Pin GPIO_PIN_4
#define ADC_5_V1_GPIO_Port GPIOC
#define ADC_3_V0_Pin GPIO_PIN_0
#define ADC_3_V0_GPIO_Port GPIOB
#define PWM_4_1_Pin GPIO_PIN_1
#define PWM_4_1_GPIO_Port GPIOB
#define TURN3B_Pin GPIO_PIN_7
#define TURN3B_GPIO_Port GPIOE
#define TURN3A_Pin GPIO_PIN_8
#define TURN3A_GPIO_Port GPIOE
#define PWM_0_0_Pin GPIO_PIN_9
#define PWM_0_0_GPIO_Port GPIOE
#define PWM_0_1_Pin GPIO_PIN_11
#define PWM_0_1_GPIO_Port GPIOE
#define TURN4A_Pin GPIO_PIN_12
#define TURN4A_GPIO_Port GPIOE
#define PWM_1_0_Pin GPIO_PIN_13
#define PWM_1_0_GPIO_Port GPIOE
#define PWM_1_1_Pin GPIO_PIN_14
#define PWM_1_1_GPIO_Port GPIOE
#define TURN4B_Pin GPIO_PIN_15
#define TURN4B_GPIO_Port GPIOE
#define PWM_2_0_Pin GPIO_PIN_10
#define PWM_2_0_GPIO_Port GPIOB
#define PWM_2_1_Pin GPIO_PIN_11
#define PWM_2_1_GPIO_Port GPIOB
#define PWM_5_0_Pin GPIO_PIN_14
#define PWM_5_0_GPIO_Port GPIOB
#define PWM_5_1_Pin GPIO_PIN_15
#define PWM_5_1_GPIO_Port GPIOB
#define VOLT_2_SEL2_Pin GPIO_PIN_8
#define VOLT_2_SEL2_GPIO_Port GPIOD
#define VOLT_3_SEL0_Pin GPIO_PIN_9
#define VOLT_3_SEL0_GPIO_Port GPIOD
#define VOLT_3_SEL1_Pin GPIO_PIN_10
#define VOLT_3_SEL1_GPIO_Port GPIOD
#define VOLT_3_SEL2_Pin GPIO_PIN_11
#define VOLT_3_SEL2_GPIO_Port GPIOD
#define ROT1_Pin GPIO_PIN_12
#define ROT1_GPIO_Port GPIOD
#define ROT2_Pin GPIO_PIN_13
#define ROT2_GPIO_Port GPIOD
#define PWM_3_0_Pin GPIO_PIN_6
#define PWM_3_0_GPIO_Port GPIOC
#define PWM_3_1_Pin GPIO_PIN_7
#define PWM_3_1_GPIO_Port GPIOC
#define PWM_4_0_Pin GPIO_PIN_8
#define PWM_4_0_GPIO_Port GPIOC
#define TURN1A_Pin GPIO_PIN_9
#define TURN1A_GPIO_Port GPIOA
#define TURN1B_Pin GPIO_PIN_10
#define TURN1B_GPIO_Port GPIOA
#define VOLT_0_SEL0_Pin GPIO_PIN_0
#define VOLT_0_SEL0_GPIO_Port GPIOD
#define VOLT_0_SEL1_Pin GPIO_PIN_1
#define VOLT_0_SEL1_GPIO_Port GPIOD
#define VOLT_0_SEL2_Pin GPIO_PIN_2
#define VOLT_0_SEL2_GPIO_Port GPIOD
#define VOLT_1_SEL0_Pin GPIO_PIN_3
#define VOLT_1_SEL0_GPIO_Port GPIOD
#define VOLT_1_SEL1_Pin GPIO_PIN_4
#define VOLT_1_SEL1_GPIO_Port GPIOD
#define VOLT_1_SEL2_Pin GPIO_PIN_5
#define VOLT_1_SEL2_GPIO_Port GPIOD
#define VOLT_2_SEL0_Pin GPIO_PIN_6
#define VOLT_2_SEL0_GPIO_Port GPIOD
#define VOLT_2_SEL1_Pin GPIO_PIN_7
#define VOLT_2_SEL1_GPIO_Port GPIOD
#define TURN2A_Pin GPIO_PIN_0
#define TURN2A_GPIO_Port GPIOE
#define TURN2B_Pin GPIO_PIN_1
#define TURN2B_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
