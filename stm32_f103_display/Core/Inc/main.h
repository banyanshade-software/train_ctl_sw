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
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#define COMMAND_INT_MAX_OUTPUT_SIZE 2096
#define VOLT_SEL0_Pin GPIO_PIN_0
#define VOLT_SEL0_GPIO_Port GPIOA
#define VOLT_SEL2_Pin GPIO_PIN_2
#define VOLT_SEL2_GPIO_Port GPIOA
#define VOLT_SEL3_Pin GPIO_PIN_4
#define VOLT_SEL3_GPIO_Port GPIOA
#define VOLT_SEL1_Pin GPIO_PIN_5
#define VOLT_SEL1_GPIO_Port GPIOA
#define M0_D1_x_Pin GPIO_PIN_10
#define M0_D1_x_GPIO_Port GPIOA
#define TURN1A_Pin GPIO_PIN_3
#define TURN1A_GPIO_Port GPIOB
#define TURN1B_Pin GPIO_PIN_4
#define TURN1B_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
