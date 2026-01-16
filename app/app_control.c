#include "app_control.h"
#include "app_motor.h"
#include "pid.h"
#include "task.h"
#include "delay.h"
#include "qmath.h"

#define D2R 0.0174533f // PI / 180
#define STD_G 9.8      // 重力加速度
#define STD_L 0.062f   // 平衡车轮胎中心到电池中心的距离
#define STD_R 0.032f   // 平衡车轮胎半径

extern PID_TypeDef contorl_theta;
extern PID_TypeDef contorl_theta_dot;
extern volatile float gx;
extern volatile float yaw;
extern volatile float pitch;
extern volatile float roll;

extern volatile float omega_ref;
static uint64_t control_t0 = 0;

/**
 * @brief 倒立摆, 串级pid初始化
 */
void app_control_init(void)
{
    PID_InitTypeDef pidttd = {0};
    pidttd.DefaultOutput = 0;
    pidttd.Setpoint = 0;

    // theta
    pidttd.Kp = 4.0f;
    pidttd.Ki = 0.0f;
    pidttd.Kd = 0.0f;
    pidttd.OutputLowerLimit = -12.57f; // -4 * pi rad/s
    pidttd.OutputUpperLimit = 12.57f;  // 4 * pi rad/s
    PID_Init(&contorl_theta, &pidttd);

    // theta dot
    pidttd.Kp = 10.0f;
    pidttd.Ki = 10.0f;
    pidttd.Kd = 0.0f;
    pidttd.OutputLowerLimit = -125.7f; // -40 * pi rad/s^2
    pidttd.OutputUpperLimit = 125.7f;  // 40 * pi rad/s^2
    PID_Init(&contorl_theta_dot, &pidttd);
}

/**
 * @brief 倒立摆, 串级pid任务切片
 */
void app_control_proc(void)
{
    PERIODIC(5); // MPU6050的读取速度为200Hz

    // 设定外环thetaPID的SP=0
    PID_ChangeSetpoint(&contorl_theta, 0.0f);

    // 获取MPU数据, 获取当前运行时间(us)
    float theta = pitch * D2R;
    float theta_dot = gx * D2R;
    uint64_t now = GetUs();
    float dt = (now - control_t0) * 1.0e-6;

    // 计算外环thetaPID的输出theta_dot_ref
    float theta_dot_ref = PID_Compute1(&contorl_theta, theta, now);

    // 更改内环theta_dotPID的设定值
    PID_ChangeSetpoint(&contorl_theta_dot, theta_dot_ref);

    // 计算内环theta_dotPID的输出theta_dotdot_ref
    float theta_dotdot_ref = PID_Compute1(&contorl_theta_dot, theta_dot, now);

    // 倒立摆逆运动解算, 与倒立摆这个非线性系统的作用相抵消
    float x_dotdot_ref = (STD_G * qsin(theta) - theta_dotdot_ref * STD_L) / qcos(theta);

    // 计算轮胎转速, rad/s
    omega_ref += (x_dotdot_ref * 1.0f) / STD_R * dt;

    // 设置轮胎转速
    app_motor_setw_L(omega_ref);
    app_motor_setw_R(omega_ref);

    // 更新controlapp的上一次时间
    control_t0 = now;
}
