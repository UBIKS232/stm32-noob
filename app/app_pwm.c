#include "app_pwm.h"

#define FABS(x) (((x) > 0) ? (x) : (-(x)))

static void init_motor(void);
static void init_tim_pwm(void);

/**
 * @brief 初始化TB6612, PWM, STBY(PA1)
 */
void app_pwm_init(void)
{
    // STBY ->  PA1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpioa = {0};
    gpioa.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioa.GPIO_Pin = GPIO_Pin_1;
    gpioa.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpioa);
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);

    // 电机控制
    init_motor();
    init_tim_pwm();
}

/**
 * @brief 电机PWM任务切片
 */
void app_pwm_proc(void)
{
}

/**
 * @brief 控制TB6612休眠或者活动
 * @param uint8_t state: 1, 活动, 0 休眠
 */
void app_pwm_cmd(uint8_t state)
{
    if (state == 0)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    }
    else
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_1);
    }
}

/**
 * @brief 左电机控制
 * @param float duty: 占空比, 取值 -100.0 ~ +100.0
 */
void app_pwm_set_L(float duty)
{
    float sign; // 符号, pos: +1, neg: -1

    // 数据准备
    if(duty >= 0) sign = 1;
    else sign = -1;
    duty = FABS(duty);

    if(sign < 0)
    {
        // 反转
        GPIO_SetBits(GPIOA, GPIO_Pin_9);
        GPIO_ResetBits(GPIOA, GPIO_Pin_10);
    }
    else{
        // 正转
        GPIO_SetBits(GPIOA, GPIO_Pin_10);
        GPIO_ResetBits(GPIOA, GPIO_Pin_9);
    }

    // set CCRx
    uint16_t ccr = duty / 100.0f * (1000 - 1);
    TIM_SetCompare1(TIM1, ccr);
}

/**
 * @brief 右电机控制
 * @param float duty: 占空比, 取值 -100.0 ~ +100.0
 */
void app_pwm_set_R(float duty)
{    float sign; // 符号, pos: +1, neg: -1

    // 数据准备
    if(duty >= 0) sign = 1;
    else sign = -1;
    duty = FABS(duty);

    if(sign > 0)
    {
        // 正转
        GPIO_SetBits(GPIOB, GPIO_Pin_5);
        GPIO_ResetBits(GPIOB, GPIO_Pin_7);
    }
    else{
        // 反转
        GPIO_SetBits(GPIOB, GPIO_Pin_7);
        GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    }

    // set CCRx
    uint16_t ccr = duty / 100.0f * (1000 - 1);
    TIM_SetCompare1(TIM4, ccr);
}

/**
 * @brief   电机正反转控制初始化,
 *          IN1高IN2低: 正转, IN1低IN2高: 反转,
 *          PA9 -> AIN1, PA10 -> AIN2,
 *          PB5 -> BIN1, PB7 -> BIN2
 */
static void init_motor(void)
{
    // left motor
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpioab = {0};
    gpioab.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioab.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    gpioab.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpioab);

    // right motor
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpioab.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &gpioab);
}

/**
 * @brief   TIM1/4CH1 PWM信号发生初始化,
 *          TIM1 CH1: PA8, TIM4 CH1: PB6
 */
static void init_tim_pwm(void)
{
    // PA8
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpioab = {0};
    gpioab.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioab.GPIO_Pin = GPIO_Pin_8;
    gpioab.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpioab);

    // PB6
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpioab.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOB, &gpioab);

    // TIM1: 72MHz / 1000 = 72kHz
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    TIM_TimeBaseInitTypeDef tim14 = {0};
    tim14.TIM_CounterMode = TIM_CounterMode_Up;
    tim14.TIM_ClockDivision = TIM_CKD_DIV1;
    tim14.TIM_Period = 1000 - 1; // ARR
    tim14.TIM_Prescaler = 1 - 1; // PSC
    tim14.TIM_RepetitionCounter = 1 - 1;
    TIM_TimeBaseInit(TIM1, &tim14);
    // TIM4: 72MHz / 1000 = 72kHz
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    TIM_TimeBaseInit(TIM4, &tim14);

    // PWM channel: output compare, CCRx
    TIM_OCInitTypeDef tim14oc = {0};
    tim14oc.TIM_OCMode = TIM_OCMode_PWM1;
    tim14oc.TIM_OCPolarity = TIM_OCPolarity_High;
    tim14oc.TIM_OutputState = ENABLE;
    tim14oc.TIM_Pulse = 0;
    TIM_OC1Init(TIM1, &tim14oc);      // ch1 -> tim1
    TIM_CtrlPWMOutputs(TIM1, ENABLE); // MOE
    TIM_OC1Init(TIM4, &tim14oc);      // ch1 -> tim4
    TIM_CtrlPWMOutputs(TIM4, ENABLE); // MOE

    // 使能TIM1/4
    TIM_Cmd(TIM1, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
}
