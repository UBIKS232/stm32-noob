#ifndef __APP_ENCODER_H__
#define __APP_ENCODER_H__

#include "stm32f10x.h"

void app_encoder_init(void);
void app_encoder_proc(void);
float app_encoder_getpos_L(void);
float app_encoder_getpos_R(void);
float app_encoder_getw_L(void);
float app_encoder_getw_R(void);

#endif // __APP_ENCODER_H__

