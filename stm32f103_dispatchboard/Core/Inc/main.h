/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define LED3_Pin GPIO_PIN_13
#define LED3_GPIO_Port GPIOC
#define RELAY_SET2_Pin GPIO_PIN_14
#define RELAY_SET2_GPIO_Port GPIOC
#define RELAY_SET0_Pin GPIO_PIN_15
#define RELAY_SET0_GPIO_Port GPIOC
#define VOLT_0_SEL_0_Pin GPIO_PIN_4
#define VOLT_0_SEL_0_GPIO_Port GPIOA
#define VOLT_0_SEL_1_Pin GPIO_PIN_5
#define VOLT_0_SEL_1_GPIO_Port GPIOA
#define VOLT_0_SEL_2_Pin GPIO_PIN_6
#define VOLT_0_SEL_2_GPIO_Port GPIOA
#define RELAY0_Pin GPIO_PIN_0
#define RELAY0_GPIO_Port GPIOB
#define RELAY_SET1_Pin GPIO_PIN_1
#define RELAY_SET1_GPIO_Port GPIOB
#define TURN3B_Pin GPIO_PIN_12
#define TURN3B_GPIO_Port GPIOB
#define TURN3A_Pin GPIO_PIN_13
#define TURN3A_GPIO_Port GPIOB
#define TURN2B_Pin GPIO_PIN_14
#define TURN2B_GPIO_Port GPIOB
#define TURN2A_Pin GPIO_PIN_15
#define TURN2A_GPIO_Port GPIOB
#define TURN1B_Pin GPIO_PIN_8
#define TURN1B_GPIO_Port GPIOA
#define TURN1A_Pin GPIO_PIN_9
#define TURN1A_GPIO_Port GPIOA
#define LED0_Pin GPIO_PIN_10
#define LED0_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_4
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_5
#define LED2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
