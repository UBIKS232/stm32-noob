#include <stdio.h>
#include "stm32f10x.h"
#include "delay.h"

#define ADC_SINGLE_CHANNEL 1

void init_adc1(void);
void init_usart1(void);
void init_pc13(void);

uint16_t userADCGetDataSoft(void);
void userSerialTransmit(USART_TypeDef *usartTypedef_t, uint8_t *pData, uint16_t size);
void userSerialReceive(USART_TypeDef *usartTypedef_t, uint8_t *pData);
int fputc(int ch, FILE *file);

int main(void)
{
    Delay_Init();
    init_pc13();
    // init_usart1();
    init_adc1();

    while (1)
    {
        if((userADCGetDataSoft() * (3.3f / 4095)) > 1.5){
            GPIO_SetBits(GPIOC, GPIO_Pin_13);
        }
        else{
            GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        }
        DelayUs(10);
    }
}

void init_pc13()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef gpioc = {
        .GPIO_Mode = GPIO_Mode_Out_OD,
        .GPIO_Pin = GPIO_Pin_13,
        .GPIO_Speed = GPIO_Speed_2MHz};
    GPIO_Init(GPIOC, &gpioc);
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void init_adc1()
{
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

    // RCC_ClocksTypeDef rcc_clk;
    // RCC_GetClocksFreq(&rcc_clk);
    // int tcal = (int)((1 / (float)rcc_clk.ADCCLK_Frequency) * 1e6 * 4);
    // int tstab = (int)((1 / (float)rcc_clk.ADCCLK_Frequency) * 1e6 * 4);
    // DelayUs(tcal);

    TIM_TimeBaseInitTypeDef tim3 = {
        .TIM_ClockDivision = TIM_CKD_DIV4, // 1, 2, 4
        .TIM_CounterMode = TIM_CounterMode_Up,
        .TIM_Period = 3,
        .TIM_Prescaler = 6000,
        // .TIM_RepetitionCounter =
    };
    TIM_TimeBaseInit(TIM3, &tim3);
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
    TIM_Cmd(TIM3, ENABLE);


    GPIO_InitTypeDef gpioa = {
        .GPIO_Mode = GPIO_Mode_AIN,
        // .GPIO_Speed = GPIO_Speed_50MHz, // not needed
        .GPIO_Pin = GPIO_Pin_0};
    GPIO_Init(GPIOA, &gpioa);

    ADC_InitTypeDef adc1 = {
        .ADC_ContinuousConvMode = DISABLE,
        .ADC_DataAlign = ADC_DataAlign_Right,
        // .ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO,
        .ADC_ExternalTrigConv = ADC_ExternalTrigConv_None, // none 为软件触发
        .ADC_Mode = ADC_Mode_Independent, // 双ADC模式, 选独立模式
        .ADC_NbrOfChannel = ADC_SINGLE_CHANNEL,
        .ADC_ScanConvMode = DISABLE};
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) != SET)
        ;
    ADC_Init(ADC1, &adc1);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_13Cycles5);
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
}

#define RCCAPB2PERIPH (RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO)

void init_usart1(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

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
    GPIO_Init(GPIOA, &gpioInit_A10);
    GPIO_Init(GPIOA, &gpioInit_A9);

    USART_InitTypeDef usartInit = {
        .USART_BaudRate = 115200,
        // .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
        .USART_Mode = USART_Mode_Tx | USART_Mode_Rx,
        .USART_Parity = USART_Parity_No,
        .USART_StopBits = USART_StopBits_1,
        .USART_WordLength = USART_WordLength_8b}; // 9b - 1b(parity) = 8b(data)
    USART_Init(USART1, &usartInit);
    USART_Cmd(USART1, ENABLE);
}

uint16_t userADCGetDataSoft(void){
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) != SET);
    uint16_t adc_value = ADC_GetConversionValue(ADC1);
    return adc_value;
}

void userSerialTransmit(USART_TypeDef *usartTypedef_t, uint8_t *pData, uint16_t size)
{
    if (size == 0)
        return;

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

void userSerialReceive(USART_TypeDef *usartTypedef_t, uint8_t *pData)
{
    while (USART_GetFlagStatus(usartTypedef_t, USART_FLAG_RXNE) == RESET)
        ;
    *pData = USART_ReceiveData(usartTypedef_t);
}

int fputc(int ch, FILE *file)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        ;
    USART_SendData(USART1, (uint8_t)ch);
    return ch;
}
