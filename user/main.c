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
#endif

volatile float vbat = 0.0f;
// volatile uint8_t vbat_state = 0;
volatile uint8_t pwm_state = 0;

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
#endif

#ifdef TEST
    // test_battery();
    test_pwm();
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
#endif
    }
}
