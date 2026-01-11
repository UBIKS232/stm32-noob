#include "app_usart2.h"
#include "stm32f10x.h"

/**
 * @brief 初始化USART2作为串口调试的接口, PA2: Tx, PA3: Rx, baudrate: 921600, 停止位: 1, 数据位: 8, 无校验, 调用my_lib中的方法实现消息收发
 */
void init_usart2(void)
{
    // PA2: Tx, PA3: Rx
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpioa = {0};
    gpioa.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioa.GPIO_Pin = GPIO_Pin_2;
    gpioa.GPIO_Speed = GPIO_Speed_2MHz; // 921600
    GPIO_Init(GPIOA, &gpioa);           // Tx
    gpioa.GPIO_Mode = GPIO_Mode_IPU;
    gpioa.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &gpioa); // Rx

    // USART2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    USART_InitTypeDef usart2 = {0};
    usart2.USART_BaudRate = 921600;
    usart2.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    usart2.USART_Parity = USART_Parity_No;
    usart2.USART_StopBits = USART_StopBits_1;
    usart2.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &usart2);

    // 使能USART2
    USART_Cmd(USART2, ENABLE);
}
