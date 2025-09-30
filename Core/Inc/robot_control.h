/*
 * robot_control.h
 *
 *  Created on: Jun 13, 2025
 *      Author: KyungWoon
 */

#ifndef INC_ROBOT_CONTROL_H_
#define INC_ROBOT_CONTROL_H_

#include <stdint.h>
#define CMD_BUF_SIZE 32

void Control_Init(void);
void Control_SetCmd(const char *cmd);
void Control_Update(void);

#endif /* INC_ROBOT_CONTROL_H_ */
