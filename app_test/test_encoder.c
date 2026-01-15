#include "test_encoder.h"
#include "app_usart2.h"
#include "app_encoder.h"
#include "usart.h"
#include "delay.h"

extern volatile int64_t encoder_acc_L;
extern volatile int64_t encoder_acc_R;
extern volatile float encoder_raw_w_L;
extern volatile float encoder_raw_w_R;
extern volatile float encoder_filtered_w_L;
extern volatile float encoder_filtered_w_R;

static volatile float last_pos_L = 0.0f; // M method
static volatile float last_pos_R = 0.0f; // M method

/**
 * @brief 测试encoder, 电机角度测量
 */
void test_encoder(void){
    app_usart2_init();
    app_encoder_init();

    while (1)
    {
        // My_USART_Printf(USART2, "%d,%d\n", (int32_t)encoder_acc_L, (int32_t)encoder_acc_R);
        My_USART_Printf(USART2, "%.3f,%.3f\n", app_encoder_getpos_L(), app_encoder_getpos_R());
        Delay(50);
    }
}

/**
 * @brief 测试encoder, 电机速度测量, M法
 */
void test_encoder_Mmethod(void){
    app_usart2_init();
    app_encoder_init();

    while (1)
    {
        float pos_L = app_encoder_getpos_L();
        float pos_R = app_encoder_getpos_R();

        float a_L = pos_L - last_pos_L;
        float a_R = pos_R - last_pos_R;

        float w_L = a_L / 0.001f;
        float w_R = a_R / 0.001f;

        last_pos_L = pos_L;
        last_pos_R = pos_R;

        // 出现锯齿形的波形
        My_USART_Printf(USART2, "%.2f,%.2f,%.2f,%.2f\n", pos_L, pos_R, w_L, w_R);

        Delay(1); //  dt = 1ms
    }
}

/**
 * @brief 测试encoder, 电机速度测量, T法
 */
void test_encoder_Tmethod(void){
    app_usart2_init();
    app_encoder_init();

    while (1)
    {
        // float pos_L = app_encoder_getpos_L();
        // float pos_R = app_encoder_getpos_R();

        // My_USART_Printf(USART2, "%.2f,%.2f,%.2f,%.2f\n", pos_L, pos_R, app_encoder_getw_L(), app_encoder_getw_R());
        My_USART_Printf(USART2, "%.2f,%.2f\n", app_encoder_getw_L(), encoder_raw_w_L);

        Delay(1); //  dt = 1ms
    }
}

