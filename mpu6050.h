#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include "main.h"


/* ===================== DATA ===================== */

typedef struct
{
    /* Accelerometer RAW */
    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;

    /* Gyroscope RAW */
    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;

    /* Accelerometer - g */
    float Ax;
    float Ay;
    float Az;

    /* Gyroscope - deg/s */
    float Gx;
    float Gy;
    float Gz;

    /* Angle - degree */
    float Roll;
    float Pitch;
    float Yaw;

    /* Sample time - second */
    float dt;

} MPU6050_Data_t;

/* ===================== PUBLIC ANGLE ===================== */

/* Bien public:
 * - Co the dung truc tiep trong code
 * - Co the xem truc tiep trong Keil Watch
 */
extern float MPU_Yaw;
extern float MPU_Roll;
extern float MPU_Pitch;


/* ===================== FUNCTION ===================== */

/* Khởi tạo MPU6050 */
HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *hi2c);

/* Calibration Gyro Z */
void MPU6050_Calibrate_GyroZ(void);

/* Đọc và cập nhật dữ liệu IMU */
void MPU6050_Update(void);

/* Hàm chạy chu kỳ 10ms giống code hiện tại */
void MPU6050_Loop(void);


/* ===================== GET DATA ===================== */

MPU6050_Data_t *MPU6050_GetData(void);


/* ===================== GET ANGLE ===================== */

float MPU6050_GetYaw(void);
float MPU6050_GetPitch(void);
float MPU6050_GetRoll(void);


/* ===================== GET GYRO ===================== */

float MPU6050_GetGx(void);
float MPU6050_GetGy(void);
float MPU6050_GetGz(void);


/* ===================== GET ACCEL ===================== */

float MPU6050_GetAx(void);
float MPU6050_GetAy(void);
float MPU6050_GetAz(void);


/* ===================== OTHER ===================== */

float MPU6050_GetDt(void);
float MPU6050_GetGyroZOffset(void);

void MPU6050_ResetYaw(void);


#endif /* INC_MPU6050_H_ */