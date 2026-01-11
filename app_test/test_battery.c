#include "test_battery.h"
#include "app_battery.h"
#include "app_usart2.h"
#include "usart.h"
#include "delay.h"

extern volatile float vbat;

/**
 * @brief 测试电池电压读取与串口回传, ADC1&USART2
 */
void test_battery(void)
{
    init_usart2();
    init_adc1();

    My_USART_Printf(USART2, "Test: ADC1, USART2, Delay.\n");

    while (1)
    {
        // My_USART_Printf(USART2, "11111111111111\n");
        Delay(10);
        // My_USART_Printf(USART2, "22222222222222\n");
        My_USART_Printf(USART2, "%.4f\n", vbat);
    }
}
