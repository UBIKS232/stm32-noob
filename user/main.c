#include "config.h"
#include "stm32f10x.h"
#include "app_battery.h"
#include "app_usart2.h"
#include "usart.h"
#include "delay.h"
#ifdef TEST
#include "test_battery.h"
#endif

volatile float vbat = 0.0f;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); // NVIC分组 0 1 2 3 4

    test_battery();

    while (1)
    {
    }
}
