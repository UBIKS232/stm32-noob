#include "test_encoder.h"
#include "app_usart2.h"
#include "app_encoder.h"
#include "usart.h"
#include "delay.h"

extern volatile int64_t encoder_L;
extern volatile int64_t encoder_R;

/**
 * @brief 测试encoder, 电机角度测量
 */
void test_encoder(void){
    app_usart2_init();
    app_encoder_init();

    while (1)
    {
        My_USART_Printf(USART2, "%d,%d\n", (int32_t)encoder_L, (int32_t)encoder_R);
        Delay(50);
    }
}
