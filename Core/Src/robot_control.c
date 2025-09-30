/*
 * robot_control.c
 *
 *  Created on: Jun 13, 2025
 *      Author: KyungWoon
 */
// robot_control.c
#include "robot_control.h"
#include "motor_control.h"
#include "sensor.h"     // dist_front, dist_right, dist_left, dist_back 전역 제공
#include <string.h>
#include <strings.h>    // strcasecmp

// 장애물 임계거리 (cm)
#define OBST_DIST_FRONT 25

static char last_cmd[CMD_BUF_SIZE];

void Control_Init(void) {
    last_cmd[0] = '\0';    // 초기엔 정지 상태
}

/** main.c의 ProcessCmd 대신, 여기서 문자열만 복사 */
void Control_SetCmd(const char *cmd) {
    // 1) 안전한 복사를 위해 로컬 버퍼에 복사
    char buf[CMD_BUF_SIZE];
    strncpy(buf, cmd, CMD_BUF_SIZE - 1);
    buf[CMD_BUF_SIZE - 1] = '\0';

    // 2) 뒤쪽에 남아 있는 '\r' 또는 '\n' 제거
    int len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n')) {
        buf[--len] = '\0';
    }

    // 3) 최종 명령 문자열로 복사
    strncpy(last_cmd, buf, CMD_BUF_SIZE - 1);
    last_cmd[CMD_BUF_SIZE - 1] = '\0';
}

void Control_Update(void) {
    const uint16_t THRESH       = OBST_DIST_FRONT;
    const uint32_t AVOID_TIME   = 500;      // 회피 동작 유지 시간 (ms)

    static uint32_t avoid_start = 0;
    static uint8_t  avoid_type  = 0;
    static char     manual_cmd[CMD_BUF_SIZE] = "";
    uint32_t now = HAL_GetTick();

    // 1) 회피 동작 유지
    if (avoid_start != 0) {
        if (now - avoid_start < AVOID_TIME) {
            switch (avoid_type) {
                case 0: Stop_All(); Move_Backward(); break;
                case 1: Stop_All(); Forward_Left();  break;
                case 2: Stop_All(); Forward_Right(); break;
                case 3: Stop_All(); Move_Forward();  break;
            }
            return;
        } else {
            avoid_start = 0;
        }
    }

    // 2) 새로운 수동 명령 수신 시 바로 저장
    if (last_cmd[0] != '\0') {
        strncpy(manual_cmd, last_cmd, sizeof(manual_cmd) - 1);
        manual_cmd[sizeof(manual_cmd) - 1] = '\0';
        last_cmd[0] = '\0';
    }

    // 3) 센서에 의한 새로운 회피
    if (dist_front < THRESH) {
        avoid_type  = 0;
        avoid_start = now;
        Stop_All(); Move_Backward();
        return;
    }
    if (dist_right < THRESH) {
        avoid_type  = 1;
        avoid_start = now;
        Stop_All(); Forward_Left();
        return;
    }
    if (dist_left < THRESH) {
        avoid_type  = 2;
        avoid_start = now;
        Stop_All(); Forward_Right();
        return;
    }
    if (dist_back < THRESH) {
        avoid_type  = 3;
        avoid_start = now;
        Stop_All(); Move_Forward();
        return;
    }

    // 4) 수동 명령 있으면 바로 실행
    if (manual_cmd[0] != '\0') {
        if      (strcasecmp(manual_cmd, "w") == 0) Move_Forward();
        else if (strcasecmp(manual_cmd, "s") == 0) Move_Backward();
        else if (strcasecmp(manual_cmd, "a") == 0) Turn_Left();
        else if (strcasecmp(manual_cmd, "d") == 0) Turn_Right();
        else if (strcasecmp(manual_cmd, "q") == 0) Forward_Left();
        else if (strcasecmp(manual_cmd, "e") == 0) Forward_Right();
        else if (strcasecmp(manual_cmd, "z") == 0) Backward_Left();
        else if (strcasecmp(manual_cmd, "c") == 0) Backward_Right();
        else if (strcasecmp(manual_cmd, "x") == 0) Stop_All();
        // 한 번만 실행하고 싶다면 아래 주석 해제:
        // manual_cmd[0] = '\0';
        return;
    }

    // 5) 기본 정지
    Stop_All();
}
