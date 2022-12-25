/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_Sockets.h"

  /* USER CODE END Includes */

  /* Exported types ------------------------------------------------------------*/
  /* USER CODE BEGIN ET */
  extern I2C_HandleTypeDef hi2c1;

  extern SPI_HandleTypeDef hspi2;

  extern UART_HandleTypeDef huart2;
  /* USER CODE END ET */

  /* Exported constants --------------------------------------------------------*/
  /* USER CODE BEGIN EC */
  extern uint8_t ucMACAddress[6];
  /* USER CODE END EC */

  /* Exported macro ------------------------------------------------------------*/
  /* USER CODE BEGIN EM */

  /* USER CODE END EM */

  /* Exported functions prototypes ---------------------------------------------*/
  void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define ENC_INT_Pin GPIO_PIN_0
#define ENC_INT_GPIO_Port GPIOC
#define ENC_CS_Pin GPIO_PIN_1
#define ENC_CS_GPIO_Port GPIOC
#define ENC_MISO_Pin GPIO_PIN_2
#define ENC_MISO_GPIO_Port GPIOC
#define ENC_MOSI_Pin GPIO_PIN_3
#define ENC_MOSI_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define ENC_RESET_Pin GPIO_PIN_0
#define ENC_RESET_GPIO_Port GPIOB
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define BQ_INT_Pin GPIO_PIN_4
#define BQ_INT_GPIO_Port GPIOB
#define BQ_CS_Pin GPIO_PIN_5
#define BQ_CS_GPIO_Port GPIOB
#define BQ_SCL_Pin GPIO_PIN_6
#define BQ_SCL_GPIO_Port GPIOB
#define BQ_SDA_Pin GPIO_PIN_7
#define BQ_SDA_GPIO_Port GPIOB
  /* USER CODE BEGIN Private defines */

// #define DEBUG

#ifdef DEBUG
#include <stdio.h>

#define debug(format, ...) printf(format, ##__VA_ARGS__)

  /*
   * Disable STDOUT buffering to enable printing before a newline
   * character or buffer flush.
   */
#else
#define debug(format, ...)
#endif /* DEBUG */
  /* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
