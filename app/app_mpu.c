#include "app_mpu.h"
#include "i2c.h"
#include "delay.h"
#include "config.h"

// mpu base addr
#define MPU_ADDR 0xD0
// cfg reg addr
#define MPU_REG_PWR_MGMT_1 0x6B   // PWR_MGMT_1
#define MPU_CMD_RESET 0x80        // PWR_MGMT_1: DEVICE_RESET -> 1
#define MPU_CMD_WAKEUP 0x00       // PWR_MGMT_1: DEVICE_RESET -> 0
#define MPU_REG_GYRO_CONFIG 0x1B  // GYRO_CONFIG
#define MPU_CMD_FSSEL 0x18        // GYRO_CONFIG: FS_SEL -> 3, +-2000
#define MPU_REG_ACCEL_CONFIG 0x1C // ACCEL_CONFIG
#define MPU_CMD_AFSSEL 0x00       // ACCEL_CONFIG: AFS_SEL -> 0, +-2g
// accel data reg addr
#define MPU_REG_ACCEL_XOUT_H 0x3B
#define MPU_REG_ACCEL_XOUT_L 0x3C
#define MPU_REG_ACCEL_YOUT_H 0x3D
#define MPU_REG_ACCEL_YOUT_L 0x3E
#define MPU_REG_ACCEL_ZOUT_H 0x3F
#define MPU_REG_ACCEL_ZOUT_L 0x40
// gyro data reg addr
#define MPU_REG_GYRO_XOUT_H 0x43
#define MPU_REG_GYRO_XOUT_L 0x44
#define MPU_REG_GYRO_YOUT_H 0x45
#define MPU_REG_GYRO_YOUT_L 0x46
#define MPU_REG_GYRO_ZOUT_H 0x47
#define MPU_REG_GYRO_ZOUT_L 0x48
// temp data reg addr
#define MPU_REG_TEMP_OUT_H 0x41
#define MPU_REG_TEMP_OUT_L 0x42
// int16_t range -2^15 ~ 2^15 -> float range -2000.0~2000.0
#define G_I2F 6.1035e-2
// int16_t range -2^15 ~ 2^15 -> float range -2.0~2.0
#define A_I2F 6.1035e-5

extern volatile float ax;
extern volatile float ay;
extern volatile float az;
extern volatile float gx;
extern volatile float gy;
extern volatile float gz;
extern volatile float temp;

static void init_i2c1(void);
static void reg_wr(uint8_t addr, uint8_t data);
static uint8_t reg_rd(uint8_t addr);

/**
 * @brief 初始化MPU6050, I2C
 */
void app_mpu_init(void)
{
    // init i2c1
    init_i2c1();

    // init mpu6050
    // 复位MPU6050
    reg_wr(MPU_REG_PWR_MGMT_1, MPU_CMD_RESET);
    Delay(100);

    // 唤醒mpu6050
    reg_wr(MPU_REG_PWR_MGMT_1, MPU_CMD_WAKEUP);

    // 配置陀螺仪, 加速度计量程
    reg_wr(MPU_REG_GYRO_CONFIG, MPU_CMD_FSSEL);
    reg_wr(MPU_REG_ACCEL_CONFIG, MPU_CMD_AFSSEL);
}

/**
 * @brief MPU6050任务切片
 */
void app_mpu_proc(void) {
    app_mpu_update();
}

/**
 * @brief 更新MPU6050测量得到的数据
 */
void app_mpu_update(void)
{
    // raw data
    // accel raw
    int16_t ax_raw = (int16_t)((reg_rd(MPU_REG_ACCEL_XOUT_H) << 8) | reg_rd(MPU_REG_ACCEL_XOUT_L));
    int16_t ay_raw = (int16_t)((reg_rd(MPU_REG_ACCEL_YOUT_H) << 8) | reg_rd(MPU_REG_ACCEL_YOUT_L));
    int16_t az_raw = (int16_t)((reg_rd(MPU_REG_ACCEL_ZOUT_H) << 8) | reg_rd(MPU_REG_ACCEL_ZOUT_L));

    // gyro raw
    int16_t gx_raw = (int16_t)((reg_rd(MPU_REG_GYRO_XOUT_H) << 8) | reg_rd(MPU_REG_GYRO_XOUT_L));
    int16_t gy_raw = (int16_t)((reg_rd(MPU_REG_GYRO_YOUT_H) << 8) | reg_rd(MPU_REG_GYRO_YOUT_L));
    int16_t gz_raw = (int16_t)((reg_rd(MPU_REG_GYRO_ZOUT_H) << 8) | reg_rd(MPU_REG_GYRO_ZOUT_L));

    // temp raw
    int16_t temp_raw = (int16_t)((reg_rd(MPU_REG_TEMP_OUT_H) << 8) | reg_rd(MPU_REG_TEMP_OUT_L));

    // raw -> true data
    ax = ax_raw * A_I2F;
    ay = ay_raw * A_I2F;
    az = az_raw * A_I2F;
    gx = gx_raw * G_I2F;
    gy = gy_raw * G_I2F;
    gz = gz_raw * G_I2F;
// 平衡车上是MPU6500, 与MPU6050在温度计算上有差异, 其他功能类似但更优秀
#ifdef USE_MPU6050
    temp = temp_raw / 340.0 + 36.53;
#endif
#ifdef USE_MPU6500
    temp = temp_raw / 333.87 + 21.0;
#endif
}

/**
 * @brief 初始化i2c1, scl -> PB8, sda -> PB9
 */
static void init_i2c1(void)
{
    // 开启PB8, PB9的复用功能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);

    // PB8, PB9
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef gpiob = {0};
    gpiob.GPIO_Mode = GPIO_Mode_AF_OD;
    gpiob.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    gpiob.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &gpiob);

    // i2c1
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    I2C_InitTypeDef iic1 = {0};
    iic1.I2C_ClockSpeed = 400000;         // 通信速度, 依据MPU6050来填写
    iic1.I2C_DutyCycle = I2C_DutyCycle_2; // 占空比
    iic1.I2C_Mode = I2C_Mode_I2C;
    I2C_Init(I2C1, &iic1);
    I2C_Cmd(I2C1, ENABLE);
}

/**
 * @brief 写MPU寄存器的值
 * @param uint8_t addr
 * @param uint8_t data
 */
static void reg_wr(uint8_t addr, uint8_t data)
{
    uint8_t buf[] = {addr, data};
    My_I2C_SendBytes(I2C1, MPU_ADDR, buf, 2);
}

/**
 * @brief 读MPU寄存器的值
 * @param uint8_t addr
 * @retval uint8_t data
 */
static uint8_t reg_rd(uint8_t addr)
{
    uint8_t buf = 0;

    My_I2C_SendBytes(I2C1, MPU_ADDR, &addr, 1); // 发送要读的寄存器的地址

    My_I2C_ReceiveBytes(I2C1, MPU_ADDR, &buf, 1);

    return buf;
}
