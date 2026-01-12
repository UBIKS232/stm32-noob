#ifndef __APP_PWM_H__
#define __APP_PWM_H__

#include "stm32f10x.h"

void app_pwm_init(void);
void app_pwm_proc(void);
void app_pwm_cmd(uint8_t state);
void app_pwm_set_L(float duty);
void app_pwm_set_R(float duty);

#endif // __APP_PWM_H__
