#include <stdio.h>
#include "stm32f10x.h"
#include "delay.h"

#define RCCAPB2PERIPH (RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO)
#define RCCAPB1PERIPH RCC_APB1Periph_I2C1

void togglePin(void);
int userI2CSendBytes(I2C_TypeDef *i2cX, uint8_t *addr, uint8_t *pData, uint16_t size);
int userI2CReadBytes(I2C_TypeDef *i2cX, uint8_t *addr, uint8_t *pBuffer, uint16_t size);

uint8_t addr = 0x78;
uint8_t init_commands[] = {0x00, 0x8d, 0x14, 0xaf, 0xa5};

int main(void)
{
    Delay_Init();
    RCC_APB2PeriphClockCmd(RCCAPB2PERIPH, ENABLE);
    RCC_APB1PeriphClockCmd(RCCAPB1PERIPH, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
    GPIO_InitTypeDef gpioInit_B8 = {
        .GPIO_Mode = GPIO_Mode_AF_OD,
        .GPIO_Pin = GPIO_Pin_8,
        .GPIO_Speed = GPIO_Speed_10MHz};
    GPIO_InitTypeDef gpioInit_B9 = {
        .GPIO_Mode = GPIO_Mode_AF_OD,
        .GPIO_Pin = GPIO_Pin_9,
        .GPIO_Speed = GPIO_Speed_10MHz};
    GPIO_InitTypeDef gpioInit_C13 = {// 见DS5319 p33
                                     .GPIO_Mode = GPIO_Mode_Out_OD,
                                     .GPIO_Pin = GPIO_Pin_13,
                                     .GPIO_Speed = GPIO_Speed_2MHz};
    GPIO_Init(GPIOB, &gpioInit_B8);
    GPIO_Init(GPIOB, &gpioInit_B9);
    GPIO_Init(GPIOC, &gpioInit_C13);

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE); // reset iic
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
    I2C_InitTypeDef iicInit = {
        .I2C_Mode = I2C_Mode_I2C,
        .I2C_ClockSpeed = 400000,
        .I2C_DutyCycle = I2C_DutyCycle_2};
    I2C_Init(I2C1, &iicInit);
    I2C_Cmd(I2C1, ENABLE);

    GPIO_SetBits(GPIOC, GPIO_Pin_13);
    Delay(1000);

    if ((userI2CSendBytes(I2C1, &addr, init_commands, sizeof(init_commands)) == -1))
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        while (1)
            ;
    }
    if ((userI2CSendBytes(I2C1, &addr, init_commands, sizeof(init_commands)) == -2))
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        while (1)
            ;
    }

    uint8_t buf;
    if (userI2CSendBytes(I2C1, &addr, &buf, sizeof(buf)) == -1)
    {
        while (1)
            ;
    }
    if((buf & (1 << 6)) == 0) GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    Delay(5000);

    while (1)
    {
        togglePin();
    }
}

void togglePin(void)
{
    GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) ? \
        GPIO_ResetBits(GPIOC, GPIO_Pin_13) : GPIO_SetBits(GPIOC, GPIO_Pin_13);
    Delay(1000);
}

// 0: success, -1: addr failure, -2: send failure
int userI2CSendBytes(I2C_TypeDef *i2cX, uint8_t *addr, uint8_t *pData, uint16_t size)
{
    // RM0008 24.3.3
    // send START
    while (I2C_GetFlagStatus(i2cX, I2C_FLAG_BUSY) == SET)
        ;
    I2C_GenerateSTART(i2cX, ENABLE);
    while (I2C_GetFlagStatus(i2cX, I2C_FLAG_SB) == RESET)
        ; // start bit

    // slave addr send
    I2C_ClearFlag(i2cX, I2C_FLAG_AF);
    I2C_Send7bitAddress(i2cX, *addr, I2C_Direction_Transmitter); // same as I2C_SendData?
    // I2C_SendData(i2cX, (*addr) & 0xfe);
    while (1)
    {
        if (I2C_GetFlagStatus(i2cX, I2C_FLAG_ADDR) == SET)
        {
            break;
        }
        if (I2C_GetFlagStatus(i2cX, I2C_FLAG_AF) == SET)
        {
            I2C_GenerateSTOP(i2cX, ENABLE);
            return -1;
        }
    }
    I2C_ReadRegister(i2cX, I2C_Register_SR1); // clear ADDR flg
    I2C_ReadRegister(i2cX, I2C_Register_SR2); // clear ADDR flg

    // send bytes
    for (uint16_t i = 0; i < size; i++)
    {
        while (1)
        {
            if (I2C_GetFlagStatus(i2cX, I2C_FLAG_TXE) == SET)
            {
                break;
            }
            if (I2C_GetFlagStatus(i2cX, I2C_FLAG_AF) == SET)
            {
                I2C_GenerateSTOP(i2cX, ENABLE);
                return -2;
            }
        }
        I2C_SendData(i2cX, pData[i]);
    }
    while (1)
    {
        if ((I2C_GetFlagStatus(i2cX, I2C_FLAG_TXE) == SET) && (I2C_GetFlagStatus(i2cX, I2C_FLAG_BTF) == SET))
        {
            break;
        }
        if (I2C_GetFlagStatus(i2cX, I2C_FLAG_AF) == SET)
        {
            I2C_GenerateSTOP(i2cX, ENABLE);
            return -2;
        }
    }

    // send STOP
    I2C_GenerateSTOP(i2cX, ENABLE);
    return 0;
}

int userI2CReadBytes(I2C_TypeDef *i2cX, uint8_t *addr, uint8_t *pBuffer, uint16_t size){
    // RM0008 24.3.3
    // send START
    while (I2C_GetFlagStatus(i2cX, I2C_FLAG_BUSY) == SET)
        ;
    I2C_GenerateSTART(i2cX, ENABLE);
    while (I2C_GetFlagStatus(i2cX, I2C_FLAG_SB) == RESET)
        ; // start bit

    // slave addr send
    I2C_ClearFlag(i2cX, I2C_FLAG_AF);
    I2C_Send7bitAddress(i2cX, *addr, I2C_Direction_Receiver); // same as I2C_SendData?
    // I2C_SendData(i2cX, (*addr) | 0x01);
    while (1)
    {
        if (I2C_GetFlagStatus(i2cX, I2C_FLAG_ADDR) == SET)
        {
            break;
        }
        if (I2C_GetFlagStatus(i2cX, I2C_FLAG_AF) == SET)
        {
            I2C_GenerateSTOP(i2cX, ENABLE);
            return -1;
        }
    }
    // I2C_ReadRegister(i2cX, I2C_Register_SR1); // clear ADDR flg
    // I2C_ReadRegister(i2cX, I2C_Register_SR2); // clear ADDR flg

    if(size == 1){
        I2C_ReadRegister(i2cX, I2C_Register_SR1); // clear ADDR flg
        I2C_ReadRegister(i2cX, I2C_Register_SR2); // clear ADDR flg

        I2C_AcknowledgeConfig(i2cX, DISABLE); // NACK
        I2C_GenerateSTOP(i2cX, ENABLE); // STOP, 作用于下一个数据, 对当前正在移入寄存器的总线数据不影响

        while(I2C_GetFlagStatus(i2cX, I2C_FLAG_RXNE) == RESET);

        pBuffer[0] = I2C_ReceiveData(i2cX);
    }
    else{
        I2C_ReadRegister(i2cX, I2C_Register_SR1); // clear ADDR flg
        I2C_ReadRegister(i2cX, I2C_Register_SR2); // clear ADDR flg

        I2C_AcknowledgeConfig(i2cX, ENABLE); // ACK

        for(int i = 0; i < size - 1; i++){
            while(I2C_GetFlagStatus(i2cX, I2C_FLAG_RXNE) == RESET);

            pBuffer[i] = I2C_ReceiveData(i2cX);
        }

        I2C_AcknowledgeConfig(i2cX, DISABLE); // NACK
        I2C_GenerateSTOP(i2cX, ENABLE); // STOP, 作用于下一个数据, 对当前正在移入寄存器的总线数据不影响

        while(I2C_GetFlagStatus(i2cX, I2C_FLAG_RXNE) == RESET);

        pBuffer[size - 1] = I2C_ReceiveData(i2cX);
    }

    return 0;
}
