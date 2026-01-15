#ifndef __APP_MOTOR_H__
#define __APP_MOTOR_H__

#include "stm32f10x.h"

void app_motor_init(void);
void app_motor_proc(void);
void app_motor_setw_L(float w);
void app_motor_setw_R(float w);

#endif // __APP_MOTOR_H__
