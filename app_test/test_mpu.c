#include "test_mpu.h"
#include "app_usart2.h"
#include "app_mpu.h"
#include "delay.h"
#include "usart.h"
#include "task.h"

// raw
extern volatile float ax;
extern volatile float ay;
extern volatile float az;
extern volatile float gx;
extern volatile float gy;
extern volatile float gz;
extern volatile float temp;

// eular angle
extern volatile float yaw;
extern volatile float pitch;
extern volatile float roll;

/**
 * @brief 周期为10ms的usart2发送任务
 */
void test_usart2_proc(void)
{
    PERIODIC(10);
    // My_USART_Printf(USART2, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", yaw, pitch, roll, gx, gy, gz);
    My_USART_Printf(USART2, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", yaw, pitch, roll, ax, ay, az);
    // My_USART_Printf(USART2, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", ax, ay, az, gx, gy, gz, temp);
}

/**
 * @brief 测试mpu数据获取, 原始数据, 欧拉角
 */
void test_mpu(void)
{
    app_usart2_init();
    app_mpu_init();

    My_USART_Printf(USART2, "test\n");

    while (1)
    {
        // app_mpu_update();
        app_mpu_proc();
        // My_USART_Printf(USART2, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", ax, ay, az, gx, gy, gz, temp);
        test_usart2_proc();
        // Delay(10);
    }
}
