#include "app_battery.h"

#define VBAT_DELTA 0.1
#define VBAT_FULL 7.9 // 100%
#define VBAT_3IN4 7.4 // 75%
#define VBAT_1IN2 7   // 50%
#define VBAT_1IN5 6.5 // 20%

extern volatile float vbat;
// extern volatile uint8_t vbat_state;
static uint32_t app_battery_proc_last_time = 0;
static uint8_t app_battery_led_last_state = 0;

static void init_batled(void);
static void init_adc1(void);

/**
 * @brief 电池监测任务初始化: ADC1Injected, TIM2TRGO, LED
 */
void app_battery_init(void)
{
    init_adc1();
    init_batled();
}

/**
 * @brief 电池监测任务切片
 */
void app_battery_proc(void)
{
    if (vbat >= VBAT_FULL)
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_4, Bit_SET);
        // GPIO_SetBits(GPIOA, GPIO_Pin_6);
        // GPIO_SetBits(GPIOA, GPIO_Pin_5);
        // GPIO_SetBits(GPIOA, GPIO_Pin_4);
    }
    else if (vbat >= VBAT_3IN4)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_6);
        GPIO_SetBits(GPIOA, GPIO_Pin_5);
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
    }
    else if (vbat >= VBAT_1IN2)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_6);
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
    }
    else if (vbat >= VBAT_1IN5)
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_4, Bit_RESET);
        // GPIO_ResetBits(GPIOA, GPIO_Pin_6);
        // GPIO_ResetBits(GPIOA, GPIO_Pin_5);
        // GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    }
    else
    {
        // 似乎可以用PERIODIC(T)来写?
        uint32_t now = GetTick();
        if (now - app_battery_proc_last_time > 200)
        {
            switch (app_battery_led_last_state)
            {
            case 1:
                GPIO_WriteBit(GPIOA, GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_4, Bit_RESET);
                app_battery_led_last_state = 0;
                break;
            case 0:
                GPIO_WriteBit(GPIOA, GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_4, Bit_SET);
                app_battery_led_last_state = 1;
                break;
            }
            // app_battery_proc_last_time = GetTick();
            app_battery_proc_last_time = now;
        }
    }
}

/**
 * @brief 初始化ADC1, 注入序列CH8(PB0), 使能ADC1的JEOC标志位置位中断, 使用TIM2_TRGO触发采样, 在ADC1_2_IRQHandler内读取结果
 */
static void init_adc1(void)
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
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB, ENABLE);
    // PB0
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
    NVIC_InitTypeDef nvic = {0};
    nvic.NVIC_IRQChannel = ADC1_2_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&nvic);

    // 使能ADC1
    ADC_Cmd(ADC1, ENABLE);
}

/**
 * @brief 电量指示LED初始化: PA4, PA5, PA6
 */
static void init_batled(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gpioa = {0};
    gpioa.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioa.GPIO_Speed = GPIO_Speed_2MHz;

    gpioa.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_Init(GPIOA, &gpioa);
}

/**
 * @brief ADC1中断响应函数, 读取ADC1在JEOC标志位置位中断发生后JDR1内的值
 */
void ADC1_2_IRQHandler(void)
{
    if (ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC) == SET)
    {
        ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);
        uint16_t jdr1 = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1);
        vbat = jdr1 * 2.0513e-3; // 3.3 * (8.4f / 3.3f) / 4095.0f = 2.0513e-3
    }
}
