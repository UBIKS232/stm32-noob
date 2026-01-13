#include "app_encoder.h"
#include "delay.h"

#define RATIO 0.8018f // 360 / ((30613 / 1500) * 22)
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

extern volatile int64_t encoder_acc_L;
extern volatile int64_t encoder_acc_R;
extern volatile int8_t encoder_direction_L;
extern volatile int8_t encoder_direction_R;
extern volatile uint64_t encoder_t0_L;
extern volatile uint64_t encoder_t0_R;
extern volatile uint64_t encoder_t1_L;
extern volatile uint64_t encoder_t1_R;

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
 * @brief 读取左轮胎实际旋转的角度
 * @retval float theta: 左轮胎实际旋转的角度
 */
float app_encoder_getpos_L(void)
{
    return (encoder_acc_L * RATIO);
}

/**
 * @brief 读取右轮胎实际旋转的角度
 * @retval float theta: 右轮胎实际旋转的角度
 */
float app_encoder_getpos_R(void)
{
    return (encoder_acc_R * RATIO);
}

/**
 * @brief T法测量左轮胎转速
 * @retval float w: 左轮胎角速度
 */
float app_encoder_getw_L(void)
{
    __disable_irq();

    int8_t cpy_encoder_direction_L = encoder_direction_L;
    uint64_t cpy_encoder_t0_L = encoder_t0_L;
    uint64_t cpy_encoder_t1_L = encoder_t1_L;

    __enable_irq();

    uint64_t now = GetUs();
    uint64_t dt = MAX(cpy_encoder_t1_L - cpy_encoder_t0_L, now - cpy_encoder_t1_L);

    return (RATIO * cpy_encoder_direction_L / (dt * 1.0e-6f));

    // 下面这样写会导致严重的毛刺!! GetUs()被反复调用和打断, 而t0, t1又在不断更新, 时间值会更容易出现异常.
    // return ((encoder_direction_L / (MAX(encoder_t1_L - encoder_t0_L, GetUs() - encoder_t1_L) * 1.0e-6)) * RATIO);
}

/**
 * @brief T法测量右轮胎转速
 * @retval float w: 右轮胎角速度
 */
float app_encoder_getw_R(void)
{
    __disable_irq();

    int8_t cpy_encoder_direction_R = encoder_direction_R;
    uint64_t cpy_encoder_t0_R = encoder_t0_R;
    uint64_t cpy_encoder_t1_R = encoder_t1_R;

    __enable_irq();

    uint64_t now = GetUs();
    uint64_t dt = MAX(cpy_encoder_t1_R - cpy_encoder_t0_R, now - cpy_encoder_t1_R);

    return (RATIO * cpy_encoder_direction_R / (dt * 1.0e-6f));
}

/**
 * @brief EXTI line 14 中断响应函数, 左电机A相, 更新编码器累加值, 累加方向, 获取编码器连续两次更新时刻
 */
void EXTI15_10_IRQHandler(void)
{
    // 多个中断的响应函数, 因此需要判断
    if (EXTI_GetFlagStatus(EXTI_Line14) == SET)
    {
        EXTI_ClearFlag(EXTI_Line14); // 防止反复进入中断

        // 获取电平
        uint8_t phase_a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14);
        uint8_t phase_b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15);

        // 获取更新时刻
        encoder_t0_L = encoder_t1_L;
        encoder_t1_L = GetUs();

        if (((phase_a == Bit_SET) && (phase_b == Bit_SET)) ||
            ((phase_a == Bit_RESET) && (phase_b == Bit_RESET)))
        {
            encoder_acc_L++;
            if (encoder_direction_L < 0)
                encoder_direction_L = 0; // 判断过零
            else
                encoder_direction_L = 1;
        }
        else
        {
            // encoder_acc_L++;
            encoder_acc_L--;
            if (encoder_direction_L > 0)
                encoder_direction_L = 0; // 判断过零
            else
                encoder_direction_L = -1;
        }
    }
}

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
 * @brief EXTI line 3 中断响应函数, 右电机A相, 更新编码器累加值, 累加方向, 获取编码器连续两次更新时刻
 */
void EXTI3_IRQHandler(void)
{
    EXTI_ClearFlag(EXTI_Line3); // 防止反复进入中断

    // 获取电平
    uint8_t phase_a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3);
    uint8_t phase_b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);

    // 获取更新时刻
    encoder_t0_R = encoder_t1_R;
    encoder_t1_R = GetUs();

    if (((phase_a == Bit_SET) && (phase_b == Bit_SET)) ||
        ((phase_a == Bit_RESET) && (phase_b == Bit_RESET)))
    {
        encoder_acc_R--;
        if (encoder_direction_R > 0)
            encoder_direction_R = 0; // 过零
        else
            encoder_direction_R = -1;
    }
    else
    {
        encoder_acc_R++;
        if (encoder_direction_R < 0)
            encoder_direction_R = 0; // 过零
        else
            encoder_direction_R = 1;
    }
}
