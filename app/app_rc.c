#include "app_rc.h"
#include "app_control.h"
#include <string.h>
#include <stdio.h>

#define BUF_MAX_LENGTH 64 // bytes

static char int_buf[BUF_MAX_LENGTH];   // 中断程序接收数据的缓冲区
static char trans_buf[BUF_MAX_LENGTH]; // 从中断函数向进程函数转运数据
static char proc_buf[BUF_MAX_LENGTH];  // 进程函数处理数据

static volatile uint8_t line_received_flag = 0; // 一行指令接收完成标志位
static volatile uint16_t int_buf_cursor = 0;    // 指向intt_buf中下一个空白位置

/**
 * @brief 初始化遥控器模块, PB10: Tx, PB11: Rx
 */
void app_rc_init(void)
{
    // PB10: Tx, PB11: Rx
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef gpiob = {0};
    gpiob.GPIO_Mode = GPIO_Mode_AF_PP;
    gpiob.GPIO_Pin = GPIO_Pin_10;
    gpiob.GPIO_Speed = GPIO_Speed_2MHz; // 921600
    GPIO_Init(GPIOB, &gpiob);           // Tx
    gpiob.GPIO_Mode = GPIO_Mode_IPU;
    gpiob.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOB, &gpiob); // Rx
    // GPIO_WriteBit(GPIOA, GPIO_Pin_2 | GPIO_Pin_3, Bit_SET);

    // USART2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    USART_InitTypeDef usart3 = {0};
    usart3.USART_BaudRate = 9600;
    usart3.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    usart3.USART_Parity = USART_Parity_No;
    usart3.USART_StopBits = USART_StopBits_1;
    usart3.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3, &usart3);

    // 开启USART3接收寄存器满中断RXNE
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    // NVIC: USART3_IRQn
    NVIC_InitTypeDef nvic = {0};
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannel = USART3_IRQn; // 中断编号
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0; // 最高
    NVIC_Init(&nvic);

    // 使能USART3
    USART_Cmd(USART3, ENABLE);
}

/**
 * @brief 遥控器模块进程切片, 处理接受的控制数据
 */
void app_rc_proc(void)
{
    if (line_received_flag == 1)
    {
        strcpy(proc_buf, trans_buf);
        line_received_flag = 0;

        if (strncasecmp(proc_buf, "move ", 5) == 0)
        {
            int turn_speed = 0; // 幅度为-100~+100
            int move_speed = 0; // 幅度为-100~+100

            if (sscanf(proc_buf, "move %d %d", &turn_speed, &move_speed) == 2)
            {
                app_conrtol_set_move_speed(move_speed * 0.01f * 0.7f);
                app_conrtol_set_turn_speed(-turn_speed * 0.01f * 15.0f);
            }
        }
    }
}

/**
 * @brief 串口3的中断响应函数
 */
void USART3_IRQHandler(void)
{
    if (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
    {
        uint8_t data = USART_ReceiveData(USART3); // 读取DR后外USART外设的硬件电路会自动清零RXNE标志位
        if (data != '\n')
        {
            int_buf[int_buf_cursor] = data;
            int_buf_cursor++;
        }
        else
        {
            int_buf[int_buf_cursor] = '\0';
            int_buf_cursor = 0;
            strcpy(trans_buf, int_buf);
            line_received_flag = 1;
        }
    }
}
