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
#include "oam/oam_error.h"
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
#define CT1_3_Pin GPIO_PIN_2
#define CT1_3_GPIO_Port GPIOE
#define CT1_2_Pin GPIO_PIN_3
#define CT1_2_GPIO_Port GPIOE
#define CT0_1_Pin GPIO_PIN_4
#define CT0_1_GPIO_Port GPIOE
#define CT1_1_Pin GPIO_PIN_5
#define CT1_1_GPIO_Port GPIOE
#define CT0_2_Pin GPIO_PIN_6
#define CT0_2_GPIO_Port GPIOE
#define TURN3A_Pin GPIO_PIN_13
#define TURN3A_GPIO_Port GPIOC
#define ADC_3_V0_Pin GPIO_PIN_0
#define ADC_3_V0_GPIO_Port GPIOC
#define ADC_3_V1_Pin GPIO_PIN_1
#define ADC_3_V1_GPIO_Port GPIOC
#define ADC_4_V0_Pin GPIO_PIN_2
#define ADC_4_V0_GPIO_Port GPIOC
#define ADC_4_V1_Pin GPIO_PIN_3
#define ADC_4_V1_GPIO_Port GPIOC
#define ADC_0_V0_Pin GPIO_PIN_0
#define ADC_0_V0_GPIO_Port GPIOA
#define ADC_0_V1_Pin GPIO_PIN_1
#define ADC_0_V1_GPIO_Port GPIOA
#define ADC_1_V0_Pin GPIO_PIN_2
#define ADC_1_V0_GPIO_Port GPIOA
#define ADC_1_V1_Pin GPIO_PIN_3
#define ADC_1_V1_GPIO_Port GPIOA
#define ADC_2_V0_Pin GPIO_PIN_4
#define ADC_2_V0_GPIO_Port GPIOA
#define ADC_2_V1_Pin GPIO_PIN_5
#define ADC_2_V1_GPIO_Port GPIOA
#define TURN4A_BOARD_LED_Pin GPIO_PIN_6
#define TURN4A_BOARD_LED_GPIO_Port GPIOA
#define TURN4B_Pin GPIO_PIN_7
#define TURN4B_GPIO_Port GPIOA
#define ADC_5_V0_Pin GPIO_PIN_4
#define ADC_5_V0_GPIO_Port GPIOC
#define ADC_5_V1_Pin GPIO_PIN_5
#define ADC_5_V1_GPIO_Port GPIOC
#define FLASH_CS_Pin GPIO_PIN_0
#define FLASH_CS_GPIO_Port GPIOB
#define PWM_4_1_Pin GPIO_PIN_1
#define PWM_4_1_GPIO_Port GPIOB
#define CT0_3_Pin GPIO_PIN_7
#define CT0_3_GPIO_Port GPIOE
#define LED5_Pin GPIO_PIN_8
#define LED5_GPIO_Port GPIOE
#define PWM_0_0_Pin GPIO_PIN_9
#define PWM_0_0_GPIO_Port GPIOE
#define LED4_Pin GPIO_PIN_10
#define LED4_GPIO_Port GPIOE
#define PWM_0_1_Pin GPIO_PIN_11
#define PWM_0_1_GPIO_Port GPIOE
#define LED3_Pin GPIO_PIN_12
#define LED3_GPIO_Port GPIOE
#define PWM_1_0_Pin GPIO_PIN_13
#define PWM_1_0_GPIO_Port GPIOE
#define PWM_1_1_Pin GPIO_PIN_14
#define PWM_1_1_GPIO_Port GPIOE
#define LED2_Pin GPIO_PIN_15
#define LED2_GPIO_Port GPIOE
#define PWM_2_0_Pin GPIO_PIN_10
#define PWM_2_0_GPIO_Port GPIOB
#define PWM_2_1_Pin GPIO_PIN_11
#define PWM_2_1_GPIO_Port GPIOB
#define CT2_1_Pin GPIO_PIN_12
#define CT2_1_GPIO_Port GPIOB
#define CT2_2_Pin GPIO_PIN_13
#define CT2_2_GPIO_Port GPIOB
#define CT2_3_Pin GPIO_PIN_14
#define CT2_3_GPIO_Port GPIOB
#define TURN5B_Pin GPIO_PIN_15
#define TURN5B_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_8
#define LED1_GPIO_Port GPIOD
#define CT5_1_Pin GPIO_PIN_9
#define CT5_1_GPIO_Port GPIOD
#define CT5_2_Pin GPIO_PIN_10
#define CT5_2_GPIO_Port GPIOD
#define CT5_3_Pin GPIO_PIN_11
#define CT5_3_GPIO_Port GPIOD
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
#define TURN5A_Pin GPIO_PIN_15
#define TURN5A_GPIO_Port GPIOA
#define TURN6A_Pin GPIO_PIN_10
#define TURN6A_GPIO_Port GPIOC
#define TURN3B_Pin GPIO_PIN_11
#define TURN3B_GPIO_Port GPIOC
#define TURN6B_Pin GPIO_PIN_12
#define TURN6B_GPIO_Port GPIOC
#define TURN2A_Pin GPIO_PIN_0
#define TURN2A_GPIO_Port GPIOD
#define TURN2B_Pin GPIO_PIN_1
#define TURN2B_GPIO_Port GPIOD
#define CT4_3_Pin GPIO_PIN_2
#define CT4_3_GPIO_Port GPIOD
#define CT4_2_Pin GPIO_PIN_3
#define CT4_2_GPIO_Port GPIOD
#define CT4_1_Pin GPIO_PIN_4
#define CT4_1_GPIO_Port GPIOD
#define CT3_1_Pin GPIO_PIN_5
#define CT3_1_GPIO_Port GPIOD
#define CT3_2_Pin GPIO_PIN_6
#define CT3_2_GPIO_Port GPIOD
#define CT3_3_Pin GPIO_PIN_7
#define CT3_3_GPIO_Port GPIOD
#define TURN1A_Pin GPIO_PIN_0
#define TURN1A_GPIO_Port GPIOE
#define TURN1B_Pin GPIO_PIN_1
#define TURN1B_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
