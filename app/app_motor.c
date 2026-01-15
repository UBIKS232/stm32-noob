#include "app_motor.h"
#include "app_encoder.h"
#include "app_battery.h"
#include "app_pwm.h"
#include "pid.h"
#include "task.h"

extern float vbat;
extern PID_TypeDef motor_pid_L;
extern PID_TypeDef motor_pid_R;

/**
 * @brief 初始化电机的pid参数, 左右分别进行初始化, 使用mylib/pid.h
 */
void app_motor_init(void)
{
    PID_InitTypeDef pidlr = {0};
    pidlr.DefaultOutput = 0;
    pidlr.Kd = 0.0f;
    pidlr.Kp = 0.55f;
    pidlr.Ki = 7.0f;
    pidlr.OutputLowerLimit = -8.4f; // 电池电压反向输出最大值, 测试时安全起见, 设置为-4.0f
    pidlr.OutputUpperLimit = 8.4f;  // 电池电压正向输出最大值, 测试时安全起见, 设置为4.0f
    pidlr.Setpoint = 0;
    PID_Init(&motor_pid_L, &pidlr);
    PID_Init(&motor_pid_R, &pidlr);
}

/**
 * @brief 初始化电机的pid计算切片
 */
void app_motor_proc(void)
{
    PERIODIC(1);

    float w_L = app_encoder_getw_L();
    float w_R = app_encoder_getw_R();

    uint64_t now = GetUs();

    float Um_L = PID_Compute1(&motor_pid_L, w_L, now);
    float Um_R = PID_Compute1(&motor_pid_R, w_R, now);

    float duty_L = Um_L / vbat * 100.f;
    float duty_R = Um_R / vbat * 100.f;

    app_pwm_set_L(duty_L);
    app_pwm_set_R(duty_R);
}
/**
 * @brief 设置左电机转速w
 * @param float w: 左电机转速(rad/s)
 */
void app_motor_setw_L(float w)
{
    PID_ChangeSetpoint(&motor_pid_L, w);
}

/**
 * @brief 设置右电机转速w
 * @param float w: 右电机转速(rad/s)
 */
void app_motor_setw_R(float w)
{
    PID_ChangeSetpoint(&motor_pid_R, w);
}
