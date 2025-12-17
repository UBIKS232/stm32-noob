#include <stdio.h>
#include "stm32f10x.h"
#include "delay.h"

#define RCCAPB2PERIPH (RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO)

void userSerialTransmit(USART_TypeDef *usartTypedef_t, uint8_t *pData, uint16_t size);
int fputc(int ch, FILE *file);

int main(void)
{
    RCC_APB2PeriphClockCmd(RCCAPB2PERIPH, ENABLE);
    Delay_Init();

    // GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE); // managed by AFPIO, enable PB6, PB7 as pins of USART1
    GPIO_InitTypeDef gpioInit_A10 = {
        // .GPIO_Mode = GPIO_Mode_AF_PP, // 配反了...
        .GPIO_Mode = GPIO_Mode_IPU,
        .GPIO_Pin = GPIO_Pin_10,
        .GPIO_Speed = GPIO_Speed_10MHz};
    GPIO_InitTypeDef gpioInit_A9 = {
        // .GPIO_Mode = GPIO_Mode_IPU,
        .GPIO_Mode = GPIO_Mode_AF_PP,
        .GPIO_Pin = GPIO_Pin_9,
        .GPIO_Speed = GPIO_Speed_10MHz};
    GPIO_InitTypeDef gpioInit_C13 = {
        .GPIO_Mode = GPIO_Mode_Out_PP,
        .GPIO_Pin = GPIO_Pin_13,
        .GPIO_Speed = GPIO_Speed_2MHz};
    GPIO_Init(GPIOA, &gpioInit_A10);
    GPIO_Init(GPIOA, &gpioInit_A9);
    GPIO_Init(GPIOC, &gpioInit_C13);

    USART_InitTypeDef usartInit = {
        .USART_BaudRate = 115200,
        // .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
        .USART_Mode = USART_Mode_Tx | USART_Mode_Rx,
        .USART_Parity = USART_Parity_Even,
        .USART_StopBits = USART_StopBits_2,
        .USART_WordLength = USART_WordLength_9b}; // 9b - 1b(parity) = 8b(data)
    USART_Init(USART1, &usartInit);
    USART_Cmd(USART1, ENABLE);

    uint8_t userData[2] = {0x38, 0x67};

    while (1)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
        Delay(1000);
        printf("Hello serial transmit.\n");
        userSerialTransmit(USART1, userData, sizeof(userData));
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        Delay(1000);
    }
}

void userSerialTransmit(USART_TypeDef *usartTypedef_t, uint8_t *pData, uint16_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        while (USART_GetFlagStatus(usartTypedef_t, USART_FLAG_TXE) == RESET)
            ;
        // if (USART_GetFlagStatus(usartTypedef_t, USART_FLAG_TXE) == SET); // 不对
        // USART_SendData(usartTypedef_t, *(pData + i));
        USART_SendData(usartTypedef_t, pData[i]);
    }
    while (USART_GetFlagStatus(usartTypedef_t, USART_FLAG_TC) == RESET)
        ;
}

int fputc(int ch, FILE *file)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        ;
    USART_SendData(USART1, (uint8_t)ch);
    return ch;
}
