/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32c0xx_hal.h"

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
#define sda_Pin GPIO_PIN_9
#define sda_GPIO_Port GPIOB
#define C1101__clk_Pin GPIO_PIN_0
#define C1101__clk_GPIO_Port GPIOA
#define DO0_Pin GPIO_PIN_1
#define DO0_GPIO_Port GPIOA
#define c1101_clk_Pin GPIO_PIN_2
#define c1101_clk_GPIO_Port GPIOA
#define C1101_miso_Pin GPIO_PIN_3
#define C1101_miso_GPIO_Port GPIOA
#define C1101__mosi_Pin GPIO_PIN_4
#define C1101__mosi_GPIO_Port GPIOA
#define DIR_Pin GPIO_PIN_5
#define DIR_GPIO_Port GPIOA
#define ENABLE_Pin GPIO_PIN_6
#define ENABLE_GPIO_Port GPIOA
#define STEP_Pin GPIO_PIN_7
#define STEP_GPIO_Port GPIOA
#define can_rx_Pin GPIO_PIN_0
#define can_rx_GPIO_Port GPIOB
#define can_tx_Pin GPIO_PIN_1
#define can_tx_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_6
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_10
#define LED2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_11
#define LED3_GPIO_Port GPIOA
#define LED4_Pin GPIO_PIN_12
#define LED4_GPIO_Port GPIOA
#define ws_Pin GPIO_PIN_15
#define ws_GPIO_Port GPIOA
#define sck_Pin GPIO_PIN_3
#define sck_GPIO_Port GPIOB
#define sd_Pin GPIO_PIN_5
#define sd_GPIO_Port GPIOB
#define gsm_tx_Pin GPIO_PIN_6
#define gsm_tx_GPIO_Port GPIOB
#define gsm_rx_Pin GPIO_PIN_7
#define gsm_rx_GPIO_Port GPIOB
#define scl_Pin GPIO_PIN_8
#define scl_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
