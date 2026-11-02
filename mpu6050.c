#include "mpu6050.h"
#include <math.h>


/* ===================== CONFIG ===================== */

#define MPU6050_ADDR       (0x68 << 1)
#define RAD_TO_DEG         57.2957795f

#define IMU_ALPHA          0.90f
#define GYRO_DEADBAND      0.5f

#define MPU6050_PERIOD_MS  10


/* ===================== PRIVATE ===================== */

static I2C_HandleTypeDef *MPU_I2C = NULL;

static uint8_t MPU_Data[14];
static uint8_t WHO_AM_I = 0;

static float Gyro_Z_Offset = 0.0f;

static float Roll_Raw  = 0.0f;
static float Pitch_Raw = 0.0f;

static uint32_t IMU_LastTime = 0;


/* Dữ liệu MPU6050 */
static MPU6050_Data_t MPU6050_Data;

/* ===================== PUBLIC ANGLE ===================== */

float MPU_Yaw   = 0.0f;
float MPU_Roll  = 0.0f;
float MPU_Pitch = 0.0f;


/* =====================================================
 *                    MPU6050 INIT
 * ===================================================== */

HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t data;
    HAL_StatusTypeDef status;

    MPU_I2C = hi2c;


    /* Kiểm tra WHO_AM_I */
    status = HAL_I2C_Mem_Read(MPU_I2C,
                              MPU6050_ADDR,
                              0x75,
                              I2C_MEMADD_SIZE_8BIT,
                              &WHO_AM_I,
                              1,
                              100);

    if (status != HAL_OK)
    {
        return status;
    }

    if (WHO_AM_I != 0x68)
    {
        return HAL_ERROR;
    }


    /* Đánh thức MPU6050 */
    data = 0x00;

    status = HAL_I2C_Mem_Write(MPU_I2C,
                               MPU6050_ADDR,
                               0x6B,
                               I2C_MEMADD_SIZE_8BIT,
                               &data,
                               1,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }


    /* Accelerometer +-2g */
    data = 0x00;

    status = HAL_I2C_Mem_Write(MPU_I2C,
                               MPU6050_ADDR,
                               0x1C,
                               I2C_MEMADD_SIZE_8BIT,
                               &data,
                               1,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }


    /* Gyroscope +-250 deg/s */
    data = 0x00;

    status = HAL_I2C_Mem_Write(MPU_I2C,
                               MPU6050_ADDR,
                               0x1B,
                               I2C_MEMADD_SIZE_8BIT,
                               &data,
                               1,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }


    /* Reset dữ liệu ban đầu */
    MPU6050_Data.Roll  = 0.0f;
    MPU6050_Data.Pitch = 0.0f;
    MPU6050_Data.Yaw   = 0.0f;

    MPU6050_Data.Gx = 0.0f;
    MPU6050_Data.Gy = 0.0f;
    MPU6050_Data.Gz = 0.0f;

    MPU6050_Data.Ax = 0.0f;
    MPU6050_Data.Ay = 0.0f;
    MPU6050_Data.Az = 0.0f;

    MPU6050_Data.dt = 0.0f;

    IMU_LastTime = HAL_GetTick();

    return HAL_OK;
}


/* =====================================================
 *                CALIBRATE GYRO Z
 * ===================================================== */

void MPU6050_Calibrate_GyroZ(void)
{
    int32_t sum = 0;
    int16_t gz_raw;
    uint8_t data[2];


    if (MPU_I2C == NULL)
    {
        return;
    }


    for (int i = 0; i < 500; i++)
    {
        HAL_I2C_Mem_Read(MPU_I2C,
                         MPU6050_ADDR,
                         0x47,
                         I2C_MEMADD_SIZE_8BIT,
                         data,
                         2,
                         100);


        gz_raw = (int16_t)((data[0] << 8) |
                           data[1]);


        sum += gz_raw;


        HAL_Delay(2);
    }


    Gyro_Z_Offset = (float)sum / 500.0f;


    /* Sau calibration bắt đầu Yaw từ 0 */
    MPU6050_Data.Yaw = 0.0f;

    IMU_LastTime = HAL_GetTick();
}


/* =====================================================
 *                  UPDATE MPU6050
 * ===================================================== */

void MPU6050_Update(void)
{
    uint32_t now;


    if (MPU_I2C == NULL)
    {
        return;
    }


    /* ================= TIME ================= */

    now = HAL_GetTick();

    MPU6050_Data.dt =
            (now - IMU_LastTime) / 1000.0f;

    IMU_LastTime = now;


    /* ================= READ MPU6050 ================= */

    if (HAL_I2C_Mem_Read(MPU_I2C,
                         MPU6050_ADDR,
                         0x3B,
                         I2C_MEMADD_SIZE_8BIT,
                         MPU_Data,
                         14,
                         100) != HAL_OK)
    {
        return;
    }


    /* ================= ACCEL RAW ================= */

    MPU6050_Data.Accel_X_RAW =
            (int16_t)((MPU_Data[0] << 8) |
                       MPU_Data[1]);

    MPU6050_Data.Accel_Y_RAW =
            (int16_t)((MPU_Data[2] << 8) |
                       MPU_Data[3]);

    MPU6050_Data.Accel_Z_RAW =
            (int16_t)((MPU_Data[4] << 8) |
                       MPU_Data[5]);


    /* ================= GYRO RAW ================= */

    MPU6050_Data.Gyro_X_RAW =
            (int16_t)((MPU_Data[8] << 8) |
                       MPU_Data[9]);

    MPU6050_Data.Gyro_Y_RAW =
            (int16_t)((MPU_Data[10] << 8) |
                       MPU_Data[11]);

    MPU6050_Data.Gyro_Z_RAW =
            (int16_t)((MPU_Data[12] << 8) |
                       MPU_Data[13]);


    /* ================= RAW -> g ================= */

    MPU6050_Data.Ax =
            (float)MPU6050_Data.Accel_X_RAW / 16384.0f;

    MPU6050_Data.Ay =
            (float)MPU6050_Data.Accel_Y_RAW / 16384.0f;

    MPU6050_Data.Az =
            (float)MPU6050_Data.Accel_Z_RAW / 16384.0f;


    /* ================= RAW -> deg/s ================= */

    MPU6050_Data.Gx =
            (float)MPU6050_Data.Gyro_X_RAW / 131.0f;

    MPU6050_Data.Gy =
            (float)MPU6050_Data.Gyro_Y_RAW / 131.0f;

    MPU6050_Data.Gz =
            ((float)MPU6050_Data.Gyro_Z_RAW -
             Gyro_Z_Offset) / 131.0f;


    /* ================= ROLL RAW ================= */

    Roll_Raw =
            atan2f(MPU6050_Data.Ay,
                   MPU6050_Data.Az)
            * RAD_TO_DEG;


    /* ================= PITCH RAW ================= */

    Pitch_Raw =
            atan2f(
                -MPU6050_Data.Ax,
                sqrtf(
                    (MPU6050_Data.Ay * MPU6050_Data.Ay) +
                    (MPU6050_Data.Az * MPU6050_Data.Az)
                )
            ) * RAD_TO_DEG;


    /* ================= LOW PASS FILTER ================= */

    MPU6050_Data.Roll =
            IMU_ALPHA * MPU6050_Data.Roll +
            (1.0f - IMU_ALPHA) * Roll_Raw;


    MPU6050_Data.Pitch =
            IMU_ALPHA * MPU6050_Data.Pitch +
            (1.0f - IMU_ALPHA) * Pitch_Raw;


    /* ================= YAW DEADBAND ================= */

    if ((MPU6050_Data.Gz > -GYRO_DEADBAND) &&
        (MPU6050_Data.Gz <  GYRO_DEADBAND))
    {
        MPU6050_Data.Gz = 0.0f;
    }


    /* ================= YAW ================= */

    MPU6050_Data.Yaw =
            MPU6050_Data.Yaw +
            (MPU6050_Data.Gz * MPU6050_Data.dt);
    /* ================= PUBLIC ANGLE ================= */

        MPU_Yaw   = MPU6050_Data.Yaw;
        MPU_Roll  = MPU6050_Data.Roll;
        MPU_Pitch = MPU6050_Data.Pitch;
}

/* =====================================================
 *                    LOOP 10 ms
 * ===================================================== */

void MPU6050_Loop(void)
{
    if ((HAL_GetTick() - IMU_LastTime) >= MPU6050_PERIOD_MS)
    {
        MPU6050_Update();
    }
}


/* =====================================================
 *                      GET DATA
 * ===================================================== */

MPU6050_Data_t *MPU6050_GetData(void)
{
    return &MPU6050_Data;
}


/* =====================================================
 *                     GET ANGLE
 * ===================================================== */

float MPU6050_GetYaw(void)
{
    return MPU6050_Data.Yaw;
}


float MPU6050_GetPitch(void)
{
    return MPU6050_Data.Pitch;
}


float MPU6050_GetRoll(void)
{
    return MPU6050_Data.Roll;
}


/* =====================================================
 *                     GET GYRO
 * ===================================================== */

float MPU6050_GetGx(void)
{
    return MPU6050_Data.Gx;
}


float MPU6050_GetGy(void)
{
    return MPU6050_Data.Gy;
}


float MPU6050_GetGz(void)
{
    return MPU6050_Data.Gz;
}


/* =====================================================
 *                     GET ACCEL
 * ===================================================== */

float MPU6050_GetAx(void)
{
    return MPU6050_Data.Ax;
}


float MPU6050_GetAy(void)
{
    return MPU6050_Data.Ay;
}


float MPU6050_GetAz(void)
{
    return MPU6050_Data.Az;
}


/* =====================================================
 *                      OTHER
 * ===================================================== */

float MPU6050_GetDt(void)
{
    return MPU6050_Data.dt;
}


float MPU6050_GetGyroZOffset(void)
{
    return Gyro_Z_Offset;
}


void MPU6050_ResetYaw(void)
{
    MPU6050_Data.Yaw = 0.0f;
    MPU_Yaw = 0.0f;
    
    IMU_LastTime = HAL_GetTick();
}