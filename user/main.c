#include "config.h"
#include "stm32f10x.h"
// apps
#include "app_battery.h"
#include "app_usart2.h"
#include "app_button.h"
#include "app_pwm.h"
#include "app_encoder.h"
#include "app_mpu.h"
#include "app_motor.h"
#include "app_control.h"
// mylibs
#include "usart.h"
#include "delay.h"
#include "task.h"
#include "i2c.h"
#include "pid.h"
// tests
#ifdef TEST
#include "test_battery.h"
#include "test_pwm.h"
#include "test_encoder.h"
#include "test_mpu.h"
#endif

// app battery
volatile float vbat = 0.0f; // battery volatage
// volatile uint8_t vbat_state = 0;
// app pwm
volatile uint8_t pwm_state = 0; // tell TB6612 to be on or not
// app encoder
volatile int64_t encoder_acc_L = 0;         // encoder L: accumulated value
volatile int64_t encoder_acc_R = 0;         // encoder R: accumulated value
volatile uint64_t encoder_t0_L = 0;         // encoder L: last time t0, in 'us'
volatile uint64_t encoder_t0_R = 0;         // encoder R: last time t0, in 'us'
volatile uint64_t encoder_t1_L = 0;         // encoder L: this time t1, in 'us'
volatile uint64_t encoder_t1_R = 0;         // encoder R: this time t1, in 'us'
volatile int8_t encoder_direction_L = 0;    // encoder L: T method direction
volatile int8_t encoder_direction_R = 0;    // encoder R: T method direction
volatile float encoder_raw_w_L = 0.0f;      // 不运行getw, 这个值就不更新
volatile float encoder_raw_w_R = 0.0f;      // 不运行getw, 这个值就不更新
volatile float encoder_filtered_w_L = 0.0f; // rad/s, 与getw函数用途重合, 但是不运行getw, 这个值就不更新
volatile float encoder_filtered_w_R = 0.0f; // rad/s, 与getw函数用途重合, 但是不运行getw, 这个值就不更新
// app mpu
volatile float ax = 0.0f; // g(9.8m/s^2)
volatile float ay = 0.0f; // g(9.8m/s^2)
volatile float az = 0.0f; // g(9.8m/s^2)
volatile float gx = 0.0f; // degree/s
volatile float gy = 0.0f; // degree/s
volatile float gz = 0.0f; // degree/s
volatile float temp = 0.0f;
volatile float yaw = 0.0f;   // 单位: degree
volatile float pitch = 0.0f; // 单位: degree
volatile float roll = 0.0f;  // 单位: degree
// app motor
PID_TypeDef motor_pid_L = {0};
PID_TypeDef motor_pid_R = {0};
// app control
volatile uint64_t control_t0 = 0;
PID_TypeDef contorl_velocity = {0};  // 速度环
PID_TypeDef contorl_theta = {0};     // theta环
PID_TypeDef contorl_theta_dot = {0}; // theta_dot(w)环
volatile float omega_ref = 0.0f;

// test pid tareget
// static float target_w = 0.0f;

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
#ifdef APP_MPU_ENABLE
    app_mpu_init();
#endif
#ifdef APP_MOTOR_ENABLE
    app_motor_init();
#endif
#ifdef APP_CONTROL_ENABLE
    app_control_init();
#endif
#endif

#ifdef TEST
    // test_battery();
    // test_pwm();
    // test_encoder();
    // test_encoder_Mmethod();
    test_encoder_Tmethod();
    // test_mpu();
#endif

    while (1)
    {
#ifndef TEST

        // target_w = (GetTick() / 1000) % 10 * 2.0f;

        // app_motor_setw_L(target_w);
        // app_motor_setw_R(target_w);

        // My_USART_Printf(USART2, "%.3f,%.3f,%.3f\n", target_w, app_encoder_getw_L(), app_encoder_getw_R());

#ifdef APP_BUTTON_ENABLE
        app_button_proc();
#endif
#ifdef APP_PWM_ENABLE
        // app_pwm_proc();
#endif
#ifdef APP_PWM_ENABLE
        // app_encoder_proc();
#endif
#ifdef APP_BATTERY_ENABLE
        app_battery_proc();
#endif
#ifdef APP_MPU_ENABLE
        app_mpu_proc();
#endif
#ifdef APP_MOTOR_ENABLE
        app_motor_proc();
#endif
#ifdef APP_CONTROL_ENABLE
        app_control_proc();
#endif
#endif
    }
}
