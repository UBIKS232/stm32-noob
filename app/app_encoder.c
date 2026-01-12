#include "app_encoder.h"

extern volatile int64_t encoder_L;
extern volatile int64_t encoder_R;

static void init_encoder_L(void);
static void init_encoder_R(void);

/**
 * @brief 初始化encoder: A路/B路GPIO, EXTI
 */
void app_encoder_init(void)
{
    init_encoder_L();
    init_encoder_R();
}

/**
 * @brief encoder任务切片
 */
void app_encoder_proc(void) {}

/**
 * @brief 初始化左编码器, A: PB14, B: PB15
 */
static void init_encoder_L(void)
{
    // A: PB14, B: PB15
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef gpiob = {0};
    gpiob.GPIO_Mode = GPIO_Mode_IPU;
    gpiob.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &gpiob);

    // EXTI
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14); // gpio pin for exti line
    EXTI_InitTypeDef exti = {0};
    exti.EXTI_Line = EXTI_Line14;
    exti.EXTI_LineCmd = ENABLE;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Init(&exti);

    // 使能EXTI中断
    NVIC_InitTypeDef nvic = {0};
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannel = EXTI15_10_IRQn; // 中断编号
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0; // 最高
    NVIC_Init(&nvic);
}

/**
 * @brief 初始化右编码器, A: PB3, B: PB4
 */
static void init_encoder_R(void)
{
    // A: PB3, B: PB4
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // 禁用JTAG
    GPIO_InitTypeDef gpiob = {0};
    gpiob.GPIO_Mode = GPIO_Mode_IPU;
    gpiob.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_3;
    GPIO_Init(GPIOB, &gpiob);

    // EXTI
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3); // gpio pin for exti line
    EXTI_InitTypeDef exti = {0};
    exti.EXTI_Line = EXTI_Line3;
    exti.EXTI_LineCmd = ENABLE;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Init(&exti);

    // 使能EXTI中断
    NVIC_InitTypeDef nvic = {0};
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannel = EXTI3_IRQn; // 中断编号
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0; // 最高
    NVIC_Init(&nvic);
}

/**
 * @brief EXTI line 3 中断响应函数, 右电机A相
 */
void EXTI3_IRQHandler(void)
{
    EXTI_ClearFlag(EXTI_Line3); // 防止反复进入中断

    uint8_t phase_a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3);
    uint8_t phase_b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);

    if (phase_a == Bit_SET)
    {
        // upedge
        if (phase_b == Bit_SET)
        {
            encoder_R--;
        }
        else
        {
            encoder_R++;
        }
    }
    else
    {
        // downedge
        if (phase_b == Bit_SET)
        {
            encoder_R++;
        }
        else
        {
            encoder_R--;
        }
    }
}

/**
 * @brief EXTI line 14 中断响应函数, 左电机A相
 */
void EXTI15_10_IRQHandler(void)
{
    // 多个中断的响应函数, 因此需要判断
    if (EXTI_GetFlagStatus(EXTI_Line14) == SET)
    {
        EXTI_ClearFlag(EXTI_Line14); // 防止反复进入中断

        uint8_t phase_a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14);
        uint8_t phase_b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15);

        if (phase_a == Bit_SET)
        {
            // upedge
            if (phase_b == Bit_SET)
            {
                encoder_L--;
            }
            else
            {
                encoder_L++;
            }
        }
        else
        {
            // downedge
            if (phase_b == Bit_SET)
            {
                encoder_L++;
            }
            else
            {
                encoder_L--;
            }
        }
    }
}
