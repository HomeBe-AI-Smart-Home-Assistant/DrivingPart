/*
 * sensor.h
 *
 *  Created on: Jun 13, 2025
 *      Author: KyungWoon
 */

#ifndef __SENSOR_H
#define __SENSOR_H

#include "stm32f1xx_hal.h"

/* 4방향 센서 측정값(cm), UINT16_MAX = out-of-range */
extern volatile uint16_t dist_front;
extern volatile uint16_t dist_right;
extern volatile uint16_t dist_left;
extern volatile uint16_t dist_back;

/**
 * @brief  TIM1 IC + Counter, IRQ 준비
 *         → MX_TIM1_Init() 이후 한 번만 호출
 */
void Sensor_Init(void);

/**
 * @brief  지정 방향(0=Front,1=Right,2=Left,3=Back) 트리거 펄스 발생
 *         → 10µs, 비블로킹
 */
void Sensor_Trigger(uint8_t dir);

#endif /* __SENSOR_H */
