#include "app_button.h"
#include "app_pwm.h"

static Button_TypeDef button_state;
extern uint8_t pwm_state;

static void button_pressed_handler(uint8_t clicks);

/**
 * @brief 初始化按钮模块, 使用ttsy的my_lib/button库
 */
void app_button_init(void)
{
    Button_InitTypeDef button = {0};
    button.GPIO_Pin = GPIO_Pin_11;
    button.GPIOx = GPIOA;
    My_Button_Init(&button_state, &button);

    My_Button_SetClickCb(&button_state, button_pressed_handler);
}

/**
 * @brief button任务的切片
 */
void app_button_proc(void)
{
    My_Button_Proc(&button_state);
}

/**
 * @brief 按钮点击回调函数
 * @param uint8_t clicks: 统计点击次数
 */
static void button_pressed_handler(uint8_t clicks)
{
    if (clicks == 1)
    {
        if (pwm_state == 1)
        {
            pwm_state = 0;
        }
        else
        {
            pwm_state = 1;
        }
        app_pwm_cmd(pwm_state);
    }
}
