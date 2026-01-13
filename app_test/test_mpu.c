#include "test_mpu.h"
#include "app_usart2.h"
#include "app_mpu.h"
#include "delay.h"
#include "usart.h"

extern volatile float ax;
extern volatile float ay;
extern volatile float az;
extern volatile float gx;
extern volatile float gy;
extern volatile float gz;
extern volatile float temp;

/**
 * @brief 测试mpu数据获取
 */
void test_mpu(void)
{
    app_usart2_init();
    app_mpu_init();

    My_USART_Printf(USART2, "test\n");

    while (1)
    {
        app_mpu_proc();
        My_USART_Printf(USART2, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", ax, ay, az, gx, gy, gz, temp);
        Delay(10);
    }
}
