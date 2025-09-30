/*
 * motor_control.c
 *
 *  Created on: Jun 13, 2025
 *      Author: KyungWoon
 */
// Core/Src/motor_control.c
#include "motor_control.h"
#include "tim.h"
#include "gpio.h"
#include <stdlib.h>

#define R_DIR_GPIO_Port GPIOB
#define R_DIR_Pin       GPIO_PIN_0
#define R_PWM_TIMER     htim2
#define R_PWM_CHANNEL   TIM_CHANNEL_3

#define L_DIR_GPIO_Port GPIOA
#define L_DIR_Pin       GPIO_PIN_0
#define L_PWM_TIMER     htim3
#define L_PWM_CHANNEL   TIM_CHANNEL_1

void Motor_Init(void) {
  // 방향 핀 초기화
  HAL_GPIO_WritePin(R_DIR_GPIO_Port, R_DIR_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(L_DIR_GPIO_Port, L_DIR_Pin, GPIO_PIN_RESET);
  // 1) CCR(비교 레지스터)을 0으로 세팅 → PWM 시작 전 기본 듀티 0%
  __HAL_TIM_SET_COMPARE(&htim2, R_PWM_CHANNEL, 0);
  __HAL_TIM_SET_COMPARE(&htim3, L_PWM_CHANNEL, 0);
  // 2) PWM 채널 켜기
  HAL_TIM_PWM_Start(&htim2, R_PWM_CHANNEL);
  HAL_TIM_PWM_Start(&htim3, L_PWM_CHANNEL);
}

void Drive(int16_t left, int16_t right)
{
  // 우측 모터
  if (abs(right)>0) HAL_TIM_PWM_Start(&R_PWM_TIMER, R_PWM_CHANNEL);
  else              HAL_TIM_PWM_Stop (&R_PWM_TIMER, R_PWM_CHANNEL);
  HAL_GPIO_WritePin(R_DIR_GPIO_Port,R_DIR_Pin,(right>=0)?GPIO_PIN_RESET:GPIO_PIN_SET);
  __HAL_TIM_SET_COMPARE(&R_PWM_TIMER,R_PWM_CHANNEL,abs(right));
  // 좌측 모터
  if (abs(left)>0)  HAL_TIM_PWM_Start(&L_PWM_TIMER, L_PWM_CHANNEL);
  else              HAL_TIM_PWM_Stop (&L_PWM_TIMER, L_PWM_CHANNEL);
  HAL_GPIO_WritePin(L_DIR_GPIO_Port,L_DIR_Pin,(left>=0)?GPIO_PIN_RESET:GPIO_PIN_SET);
  __HAL_TIM_SET_COMPARE(&L_PWM_TIMER,L_PWM_CHANNEL,abs(left));
}

void Stop_All(void)     	 { Drive(0,0); }
void Move_Forward(void)      { Drive(-DEFAULT_SPEED+20, +DEFAULT_SPEED+30) ; }
void Move_Backward(void)     { Drive(+DEFAULT_SPEED-20, -DEFAULT_SPEED-30) ; }
void Turn_Left(void)         { Drive(+DEFAULT_SPEED-20, +DEFAULT_SPEED+30) ; }
void Turn_Right(void)        { Drive(-DEFAULT_SPEED+20, -DEFAULT_SPEED-30) ; }

// 대각선 이동(전진+좌회전, 전진+우회전, 후진+좌회전, 후진+우회전)
void Forward_Left(void)      { Drive(-DEFAULT_SPEED/2, +DEFAULT_SPEED+30) ;    }
void Forward_Right(void)     { Drive(-DEFAULT_SPEED+20,   +DEFAULT_SPEED/2);  }
void Backward_Left(void)     { Drive(+DEFAULT_SPEED/2, -DEFAULT_SPEED-30) ;    }
void Backward_Right(void)    { Drive(+DEFAULT_SPEED-20,   -DEFAULT_SPEED/2);  }

// … 필요 함수들 구현

