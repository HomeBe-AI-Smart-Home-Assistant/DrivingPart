/*
 * sensor.c
 *
 *  Created on: Jun 13, 2025
 *      Author: KyungWoon (modified for DMA)
 */

#include "sensor.h"
#include "stm32f1xx_hal.h"
#include "usart.h"

extern TIM_HandleTypeDef htim1;

volatile uint16_t dist_front = UINT16_MAX;
volatile uint16_t dist_right = UINT16_MAX;
volatile uint16_t dist_left  = UINT16_MAX;
volatile uint16_t dist_back  = UINT16_MAX;

static uint8_t  current_dir = 0;
static uint32_t ic_start    = 0;
static uint8_t  ic_state    = 0;

#define DIST_OFFSET_CM 40U

void Sensor_Init(void)
{
  if (HAL_TIM_Base_Start(&htim1) != HAL_OK) {
    Error_Handler();
  }
  EXTI->FTSR &= ~((1UL<<9)|(1UL<<6)|(1UL<<4)|(1UL<<3));
  EXTI->RTSR |=  ((1UL<<9)|(1UL<<6)|(1UL<<4)|(1UL<<3));
}

void Sensor_Trigger(uint8_t dir)
{
  GPIO_TypeDef *port;
  uint16_t      pin;

  current_dir = dir;
  switch (dir) {
    case 0: port = GPIOB; pin = GPIO_PIN_8; break;
    case 1: port = GPIOA; pin = GPIO_PIN_5; break;
    case 2: port = GPIOA; pin = GPIO_PIN_7; break;
    case 3: port = GPIOB; pin = GPIO_PIN_5; break;
    default: return;
  }

  HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
  for (volatile int i = 0; i < 10; ++i) { __NOP(); }
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}


/*
 * 센서는 아래의 콜백코드만 블록하면 주행은 가능
 *
 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  uint32_t now = __HAL_TIM_GET_COUNTER(&htim1);
  uint32_t bit;
  uint16_t raw_cm, cm;

  switch (GPIO_Pin) {
    case GPIO_PIN_9: bit = 9;  break;  // Front
    case GPIO_PIN_6: bit = 6;  break;  // Right
    case GPIO_PIN_4: bit = 4;  break;  // Left
    case GPIO_PIN_3: bit = 3;  break;  // Back
    default: return;
  }


  if ((EXTI->RTSR & (1UL<<bit)) != 0) {
    /* Rising edge */
    ic_start = now;
    EXTI->RTSR &= ~(1UL<<bit);
    EXTI->FTSR |=  (1UL<<bit);
  } else {
    /* Falling edge */
    uint32_t diff = (now >= ic_start)
                   ? (now - ic_start)
                   : (0xFFFF - ic_start + now);
    raw_cm = (uint16_t)(diff * 0.01715f);

    switch (current_dir) {
      case 0: /* Front: 오프셋 적용 */
        cm = (raw_cm > DIST_OFFSET_CM) ? raw_cm - DIST_OFFSET_CM : 0;
        dist_front = cm;
        break;
      case 1: /* Right: 오프셋 없이 원값 */
        dist_right = raw_cm;
        break;
      case 2: /* Left: 오프셋 적용 */
        cm = (raw_cm > DIST_OFFSET_CM) ? raw_cm - DIST_OFFSET_CM : 0;
        dist_left = cm;
        break;
      case 3: /* Back: 오프셋 없이 원값 */
        dist_back = raw_cm;
        break;
    }


    /*
    char msg[64];
    int n = snprintf(msg, sizeof(msg),
                     "DIR=%u PIN=%u raw=%u cm=%u\r\n",
                     current_dir,
                     (unsigned)GPIO_Pin,
                     raw_cm,
                     (uint16_t)(
                       (current_dir==0||current_dir==2)
                         ? ((raw_cm>DIST_OFFSET_CM)? raw_cm-DIST_OFFSET_CM:0)
                         : raw_cm
                     ));
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, n, 1000);
*/

    ic_state = 0;
    EXTI->FTSR &= ~(1UL<<bit);
    EXTI->RTSR |=  (1UL<<bit);
  }
}
