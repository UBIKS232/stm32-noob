#ifndef __APP_CONTROL_H__
#define __APP_CONTROL_H__

#include "stm32f10x.h"

void app_control_init(void);
void app_control_proc(void);
void app_conrtol_reset(void);
void app_conrtol_set_move_speed(float speed);
void app_conrtol_set_turn_speed(float turn);

#endif // __APP_CONTROL_H__
