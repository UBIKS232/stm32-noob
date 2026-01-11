#include "app_battery.h"
#include "stm32f10x.h"

extern volatile float vbat;

/**
 * @brief 初始化ADC1, 注入序列CH8(PB0), 使能ADC1的JEOC标志位置位中断, 使用TIM2_TRGO触发采样, 在ADC1_2_IRQHandler内读取结果
 */
void init_adc1()
{
    // TIM2初始化
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef tim2 = {0};
    tim2.TIM_CounterMode = TIM_CounterMode_Up;
    tim2.TIM_ClockDivision = TIM_CKD_DIV1;
    tim2.TIM_Period = (720 - 1);
    tim2.TIM_Prescaler = (1000 - 1);
    tim2.TIM_RepetitionCounter = 0;
    // 72Mhz / 1 / 1000 / 720 = 100Hz
    TIM_TimeBaseInit(TIM2, &tim2);
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
    TIM_Cmd(TIM2, ENABLE);

    // ADC1初始化
    // PB0
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef gpiob0 = {0};
    gpiob0.GPIO_Mode = GPIO_Mode_AIN;
    gpiob0.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOB, &gpiob0);
    // ADC1
    ADC_InitTypeDef adc1 = {0};
    adc1.ADC_ContinuousConvMode = DISABLE;
    adc1.ADC_DataAlign = ADC_DataAlign_Right;
    adc1.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc1.ADC_Mode = ADC_Mode_Independent;
    adc1.ADC_NbrOfChannel = 1;
    adc1.ADC_ScanConvMode = DISABLE;
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) != SET)
        ;
    ADC_Init(ADC1, &adc1);
    // ADC1注入序列配置
    ADC_InjectedSequencerLengthConfig(ADC1, 1);
    // PB0(ADC_Channel_8) -> JDR1(ADC_InjectedChannel_1)
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_7Cycles5);
    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T2_TRGO);
    ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);

    // 使能ADC1的JEOC标志位置位中断
    ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);
    // 配置NVIC
    NVIC_InitTypeDef nvic_adc1 = {0};
    nvic_adc1.NVIC_IRQChannel = ADC1_2_IRQn;
    nvic_adc1.NVIC_IRQChannelCmd = ENABLE;
    nvic_adc1.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_adc1.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&nvic_adc1);

    // 使能ADC1
    ADC_Cmd(ADC1, ENABLE);
}

/**
 * @brief ADC1中断响应函数, 读取ADC1在JEOC标志位置位中断发生后JDR1内的值
 */
void ADC1_2_IRQHandler(void){
    if(ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC) == SET){
        ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);
        uint16_t jdr1 = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1);
        vbat = jdr1 * 2.0513e-3; // 3.3 * (8.4f / 3.3f) / 4095.0f = 2.0513e-3
    }
}
