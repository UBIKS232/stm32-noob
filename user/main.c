#include "config.h"
#include "stm32f10x.h"
#include "app_battery.h"
#include "app_usart2.h"
#include "app_button.h"
#include "app_pwm.h"
#include "usart.h"
#include "delay.h"
#ifdef TEST
#include "test_battery.h"
#include "test_pwm.h"
#include "test_encoder.h"
#endif

volatile float vbat = 0.0f; // battery volatage
// volatile uint8_t vbat_state = 0;
volatile uint8_t pwm_state = 0; // tell TB6612 to be on or not
volatile int64_t encoder_acc_L = 0; // encoder L: accumulated value
volatile int64_t encoder_acc_R = 0; // encoder R: accumulated value
volatile uint64_t encoder_t0_L = 0; // encoder L: last time t0, in 'us'
volatile uint64_t encoder_t0_R = 0; // encoder R: last time t0, in 'us'
volatile uint64_t encoder_t1_L = 0; // encoder L: this time t1, in 'us'
volatile uint64_t encoder_t1_R = 0; // encoder R: this time t1, in 'us'
volatile int8_t encoder_direction_L = 0; // encoder L: T method direction
volatile int8_t encoder_direction_R = 0; // encoder R: T method direction

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); // NVIC分组 0 1 2 3 4

#ifndef TEST
// init
#ifdef APP_USART2_ENABLE
    app_usart2_init();
#endif
#ifdef APP_BATTERY_ENABLE
    app_battery_init();
#endif
#ifdef APP_BUTTON_ENABLE
    app_button_init();
#endif
#ifdef APP_PWM_ENABLE
    app_pwm_init();
#endif
#ifdef APP_ENCODER_ENABLE
    app_encoder_init();
#endif
#endif

#ifdef TEST
    // test_battery();
    // test_pwm();
    // test_encoder();
    // test_encoder_Mmethod();
    test_encoder_Tmethod();
#endif

    while (1)
    {
#ifndef TEST
#ifdef APP_BATTERY_ENABLE
        app_battery_proc();
#endif
#ifdef APP_BUTTON_ENABLE
        app_button_proc();
#endif
#ifdef APP_PWM_ENABLE
        // app_pwm_proc();
#endif
#ifdef APP_PWM_ENABLE
        // app_encoder_proc();
#endif
#endif
    }
}
