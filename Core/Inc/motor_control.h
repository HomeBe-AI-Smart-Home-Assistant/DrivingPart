/*
 * motor_control.h
 *
 *  Created on: Jun 13, 2025
 *      Author: KyungWoon
 */

#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

#include <stdint.h>

#define DEFAULT_SPEED  450

void Motor_Init(void);
void Drive(int16_t left, int16_t right);

void Stop_All(void);
void Move_Forward(void);
void Move_Backward(void);
void Turn_Left(void);
void Turn_Right(void);

void Forward_Left(void);
void Forward_Right(void);
void Backward_Left(void);
void Backward_Right(void);

#endif /* INC_MOTOR_CONTROL_H_ */
