#include "test_pwm.h"
#include "app_button.h"
#include "app_pwm.h"
#include "delay.h"

/**
 * @brief 测试PWM, 电机控制
 */
void test_pwm(void)
{
    // 使能电机
    app_pwm_init();
    app_pwm_cmd(1);

    // 电机分别正转2s, 30%, 60% 90%
    app_pwm_set_l(30);
    app_pwm_set_r(30);
    Delay(2000);

    // app_pwm_set_l(60);
    // // app_pwm_set_r(60);
    // Delay(2000);

    // app_pwm_set_l(90);
    // // app_pwm_set_r(90);
    // Delay(2000);

    // 关闭电机
    app_pwm_cmd(0);

    while (1)
    {
    }
}
