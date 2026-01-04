#include <stdio.h>
#include "stm32f10x.h"
#include "delay.h"

#define RCCAPB2PERIPH (RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB)
// #define RCCAPB1PERIPH RCC_APB1Periph_I2C1

void togglePC13(void);
void sda_write(uint8_t state);
void scl_write(uint8_t state);
uint8_t sda_read(void);
void iic_send_start(void);
void iic_send_stop(void);
uint8_t iic_send_byte(uint8_t msg);
uint8_t iic_receive_byte(uint8_t ack);

int user_si2c_send_bytes(uint8_t *addr, uint8_t *pData, uint16_t size);
int user_si2c_reveive_bytes(uint8_t *addr, uint8_t *pBuffer, uint16_t size);

uint8_t addr = 0x78;
uint8_t init_commands[] = {
    0x00,       // 开始命令
    0x8d, 0x14, // 使能电荷泵
    0xaf,       // 打开屏幕
    0xa5        // 全亮
};

int main(void)
{
    Delay_Init();
    RCC_APB2PeriphClockCmd(RCCAPB2PERIPH, ENABLE);

    GPIO_InitTypeDef gpioInit_B8 = {// scl
                                    .GPIO_Mode = GPIO_Mode_Out_OD,
                                    .GPIO_Pin = GPIO_Pin_8,
                                    .GPIO_Speed = GPIO_Speed_10MHz};
    GPIO_InitTypeDef gpioInit_B9 = {// sda
                                    .GPIO_Mode = GPIO_Mode_Out_OD,
                                    .GPIO_Pin = GPIO_Pin_9,
                                    .GPIO_Speed = GPIO_Speed_10MHz};
    GPIO_InitTypeDef gpioInit_C13 = {// 见DS5319 p33
                                     .GPIO_Mode = GPIO_Mode_Out_OD,
                                     .GPIO_Pin = GPIO_Pin_13,
                                     .GPIO_Speed = GPIO_Speed_2MHz};
    GPIO_Init(GPIOB, &gpioInit_B8);
    GPIO_Init(GPIOB, &gpioInit_B9);
    GPIO_Init(GPIOC, &gpioInit_C13);

    GPIO_SetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9);

    user_si2c_send_bytes(&addr, init_commands, sizeof(init_commands));

    while (1)
    {
        togglePC13();
    }
}

void togglePC13(void)
{
    GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) ? GPIO_ResetBits(GPIOC, GPIO_Pin_13) : GPIO_SetBits(GPIOC, GPIO_Pin_13);
    Delay(1000);
}

void sda_write(uint8_t state)
{
    if (state == 1)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_9);
    }
    else
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_9);
    }
}

void scl_write(uint8_t state)
{
    if (state == 1)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_8);
    }
    else
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    }
}

uint8_t sda_read(void)
{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) ? 1 : 0; // 读取输入电平
}

void iic_send_start(void)
{
    scl_write(1);
    sda_write(1);
    DelayUs(1);   // 确保稳定高电平
    sda_write(0); // SCL 高时 SDA 下降沿 → START
    DelayUs(1);
    scl_write(0); // 拉低 SCL，进入数据传输阶段
}

void iic_send_stop(void)
{
    scl_write(0);
    sda_write(0);
    DelayUs(1);

    scl_write(1);
    DelayUs(1);

    sda_write(1);
    DelayUs(1);
}

// return value: 0 - ack, 1 - nack
uint8_t iic_send_byte(uint8_t msg)
{
    for (int8_t i = 7; i >= 0; i--)
    {
        scl_write(0);
        (msg & (1 << i)) ? sda_write(1) : sda_write(0);
        DelayUs(1);

        scl_write(1);
        DelayUs(1);
    }

    scl_write(0);
    sda_write(1);
    DelayUs(1);

    scl_write(1);
    DelayUs(1);

    return sda_read();
}

// ack: 0 - ack, 1 - nack
uint8_t iic_receive_byte(uint8_t ack)
{
    uint8_t buf = 0;

    for (int8_t i = 7; i >= 0; i--)
    {
        scl_write(0);
        sda_write(1); // 释放sda
        DelayUs(1);

        scl_write(1);
        DelayUs(1);

        // sda_read() ? ((buf | 1) << 1) : (buf << 1);
        if (sda_read())
            buf |= (1 << i);
    }

    scl_write(0);
    sda_write(ack);
    DelayUs(1);

    scl_write(1);
    DelayUs(1);

    return buf;
}

int user_si2c_send_bytes(uint8_t *addr, uint8_t *pData, uint16_t size)
{
    iic_send_start();
    if (!iic_send_byte(*addr & 0xfe))
    {
        iic_send_stop();
        return -1;
    }

    for (uint16_t i = 0; i < size; i++)
    {
        if (!iic_send_byte(pData[i]))
        {
            iic_send_stop();
            return -1;
        }
    }

    iic_send_stop();

    return 0;
}

int user_si2c_reveive_bytes(uint8_t *addr, uint8_t *pBuffer, uint16_t size)
{
    iic_send_start();
    if (!iic_send_byte(*addr | 0x01))
    {
        iic_send_stop();
        return -1;
    }

    for (uint16_t i = 0; i < size - 1; i++)
    {
        pBuffer[i] = iic_receive_byte(0);
    }

    pBuffer[size - 1] = iic_receive_byte(1);

    iic_send_stop();

    return 0;
}
