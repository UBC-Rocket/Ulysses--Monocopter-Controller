/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32u5xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define User_Button_Pin GPIO_PIN_2
#define User_Button_GPIO_Port GPIOE
#define GPS_INT_Pin GPIO_PIN_2
#define GPS_INT_GPIO_Port GPIOA
#define uSD_Chip_Select_Pin GPIO_PIN_4
#define uSD_Chip_Select_GPIO_Port GPIOA
#define BMI_ACC_INT1_Pin GPIO_PIN_8
#define BMI_ACC_INT1_GPIO_Port GPIOE
#define BMI_ACC_INT2_Pin GPIO_PIN_9
#define BMI_ACC_INT2_GPIO_Port GPIOE
#define BMI_GYRO_INT1_Pin GPIO_PIN_10
#define BMI_GYRO_INT1_GPIO_Port GPIOE
#define BMI_GYRO_INT2_Pin GPIO_PIN_11
#define BMI_GYRO_INT2_GPIO_Port GPIOE
#define BARO1_Chip_Select_Pin GPIO_PIN_12
#define BARO1_Chip_Select_GPIO_Port GPIOE
#define BMI_ACC_Chip_Select_Pin GPIO_PIN_10
#define BMI_ACC_Chip_Select_GPIO_Port GPIOB
#define BMI_GYRO_Chip_Select_Pin GPIO_PIN_11
#define BMI_GYRO_Chip_Select_GPIO_Port GPIOB
#define BARO2_Chip_Select_Pin GPIO_PIN_10
#define BARO2_Chip_Select_GPIO_Port GPIOD
#define BNO_Chip_Select_Pin GPIO_PIN_11
#define BNO_Chip_Select_GPIO_Port GPIOD
#define BNO_RST_Pin GPIO_PIN_12
#define BNO_RST_GPIO_Port GPIOD
#define BNO_INT_Pin GPIO_PIN_13
#define BNO_INT_GPIO_Port GPIOD
#define BNO_WAKE_Pin GPIO_PIN_14
#define BNO_WAKE_GPIO_Port GPIOD
#define BOOTN_Pin GPIO_PIN_15
#define BOOTN_GPIO_Port GPIOD
#define STAT_LED_1_Pin GPIO_PIN_6
#define STAT_LED_1_GPIO_Port GPIOC
#define STAT_LED_2_Pin GPIO_PIN_7
#define STAT_LED_2_GPIO_Port GPIOC
#define Buzzer_Pin GPIO_PIN_9
#define Buzzer_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */
#define TURN_ON_STATUS_LED1() HAL_GPIO_WritePin(STAT_LED_1_GPIO_Port, STAT_LED_1_Pin, GPIO_PIN_SET);
#define TURN_OFF_STATUS_LED1() HAL_GPIO_WritePin(STAT_LED_1_GPIO_Port, STAT_LED_1_Pin, GPIO_PIN_RESET);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
