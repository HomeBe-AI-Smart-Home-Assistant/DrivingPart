/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>    // atoi, abs
#include <string.h>    // strlen
#include <stdio.h>     // snprintf
#include "motor_control.h"   // Motor_Init(), Stop_All(), Drive(), Move_xxx()
#include "sensor.h"          // Sensor_Init(), Sensor_Update(), dist_front, dist_right, ...
#include "robot_control.h"   // Control_Init(), Control_SetCmd(), Control_Update()
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CMD_BUF_SIZE  32
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char cmdBuf[CMD_BUF_SIZE];
char    ack[64];                // 디버그 메시지용
uint8_t cmdIdx = 0;
uint8_t rx;   // UART 블로킹 수신 버퍼
volatile uint8_t flag_control; // 10 ms 타이머 ISR 신호
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  Motor_Init();                  // 모터 초기화
  Stop_All();                    // 즉시 모터 정지
  HAL_Delay(10);                 // 레벨 안정화
  Sensor_Init();                 // 초음파 센서 초기화
  Control_Init();                // 제어 모듈 초기화

    /* 2) TIM4 10ms 주기 인터럽트 시작 */
    if (HAL_TIM_Base_Start_IT(&htim4) != HAL_OK) {
      Error_Handler();
    }

  // Ready 메시지 출력
  HAL_UART_Transmit(&huart2, (uint8_t *)"UART2 Ready\r\n", 14, HAL_MAX_DELAY);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	          // 2) UART 비동기 수신 처리 (기존 코드 그대로)
	          if (HAL_UART_Receive(&huart2, &rx, 1, 0) == HAL_OK)
	          {
	              // 2-1) 받은 문자 에코
	              HAL_UART_Transmit(&huart2, &rx, 1, HAL_MAX_DELAY);

	              // 2-2) Backspace 처리
	              if (rx == '\b' || rx == 0x7F) {
	                  if (cmdIdx > 0) {
	                      cmdIdx--;
	                      HAL_UART_Transmit(&huart2, (uint8_t*)"\b \b", 3, 10);
	                  }
	                  continue;
	              }

	              // 2-3) 개행 문자 처리
	              if (rx == '\r' || rx == '\n') {
	                  if (cmdIdx > 0) {
	                      cmdBuf[cmdIdx] = '\0';
	                      int n = snprintf(ack, sizeof(ack),
	                                       "[CMD] %s 진행중\r\n",
	                                       cmdBuf);
	                      HAL_UART_Transmit(&huart2,
	                                        (uint8_t*)ack,
	                                        n,
	                                        HAL_MAX_DELAY);
	                      Control_SetCmd(cmdBuf);
	                      cmdIdx = 0;
	                  }
	                  continue;
	              }

	              // 2-4) 일반 문자 버퍼링
	              if (cmdIdx < sizeof(cmdBuf)-1) {
	                  cmdBuf[cmdIdx++] = rx;
	              }
	          }
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
}
  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim TIM handle
  */
/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim4)
  {
	  flag_control = 1;
  }
}
*/


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
