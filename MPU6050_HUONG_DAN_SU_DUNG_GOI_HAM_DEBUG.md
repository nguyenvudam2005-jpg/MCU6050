# HƯỚNG DẪN SỬ DỤNG THƯ VIỆN MPU6050

Tài liệu này hướng dẫn cách **khởi tạo, chạy, gọi dữ liệu và Debug** thư
viện `mpu6050.c / mpu6050.h` hiện tại trên STM32 + Keil.

------------------------------------------------------------------------

# 1. Các file cần có

``` text
Core/
├── Inc/
│   └── mpu6050.h
└── Src/
    └── mpu6050.c
```

Trong Keil:

-   `mpu6050.c` phải được Add vào project.
-   `Core/Inc` phải nằm trong Include Paths.
-   `main.c` cần include `mpu6050.h`.

------------------------------------------------------------------------

# 2. Include thư viện trong file muốn sử dụng

Trong `main.c`, tại:

``` c
/* USER CODE BEGIN Includes */
```

thêm:

``` c
#include "mpu6050.h"
```

Kết quả:

``` c
/* USER CODE BEGIN Includes */

#include "mpu6050.h"

/* USER CODE END Includes */
```

Nếu sau này muốn dùng MPU6050 trong file khác, ví dụ:

``` text
control.c
```

thì đầu file đó cũng chỉ cần:

``` c
#include "mpu6050.h"
```

------------------------------------------------------------------------

# 3. Khởi tạo MPU6050

Sau khi CubeMX khởi tạo I2C:

``` c
MX_GPIO_Init();
MX_I2C1_Init();
```

tại:

``` c
/* USER CODE BEGIN 2 */
```

thêm:

``` c
if (MPU6050_Init(&hi2c1) == HAL_OK)
{
    HAL_Delay(500);
    MPU6050_Calibrate_GyroZ();
}
```

Ví dụ:

``` c
MX_GPIO_Init();
MX_I2C1_Init();

/* USER CODE BEGIN 2 */

if (MPU6050_Init(&hi2c1) == HAL_OK)
{
    HAL_Delay(500);
    MPU6050_Calibrate_GyroZ();
}

/* USER CODE END 2 */
```

## Chú ý

Trong lúc:

``` c
MPU6050_Calibrate_GyroZ();
```

phải giữ MPU6050 đứng yên.

------------------------------------------------------------------------

# 4. Chạy cập nhật MPU6050

Trong:

``` c
while (1)
```

gọi:

``` c
MPU6050_Loop();
```

Ví dụ:

``` c
while (1)
{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    MPU6050_Loop();
}

/* USER CODE END 3 */
```

`MPU6050_Loop()` tự kiểm tra chu kỳ 10 ms rồi cập nhật dữ liệu.

Không cần thêm `HAL_Delay(10)` chỉ để chạy MPU6050.

------------------------------------------------------------------------

# 5. Ba giá trị góc chính

Thư viện cung cấp trực tiếp:

``` c
MPU_Yaw
MPU_Roll
MPU_Pitch
```

Đơn vị:

``` text
độ (degree)
```

Ý nghĩa:

``` text
MPU_Yaw    : góc quay quanh trục Z
MPU_Roll   : góc nghiêng Roll
MPU_Pitch  : góc nghiêng Pitch
```

Ba biến này vừa có thể dùng trong code, vừa có thể xem trực tiếp bằng
Keil Watch.

------------------------------------------------------------------------

# 6. Gọi Yaw để sử dụng

Dùng trực tiếp:

``` c
MPU_Yaw
```

Ví dụ lưu giá trị:

``` c
float yaw;

yaw = MPU_Yaw;
```

Hoặc sử dụng trực tiếp:

``` c
if (MPU_Yaw >= 90.0f)
{
    /* Code xử lý */
}
```

------------------------------------------------------------------------

# 7. Gọi Roll để sử dụng

``` c
float roll;

roll = MPU_Roll;
```

Hoặc:

``` c
if (MPU_Roll > 10.0f)
{
    /* Code xử lý */
}
```

------------------------------------------------------------------------

# 8. Gọi Pitch để sử dụng

``` c
float pitch;

pitch = MPU_Pitch;
```

Hoặc:

``` c
if (MPU_Pitch > 10.0f)
{
    /* Code xử lý */
}
```

------------------------------------------------------------------------

# 9. Lấy cả 3 giá trị

``` c
float yaw;
float roll;
float pitch;

yaw   = MPU_Yaw;
roll  = MPU_Roll;
pitch = MPU_Pitch;
```

Nếu không cần lưu riêng thì có thể dùng trực tiếp `MPU_Yaw`, `MPU_Roll`,
`MPU_Pitch`.

------------------------------------------------------------------------

# 10. Các hàm Getter

Ngoài các biến public, thư viện vẫn hỗ trợ Getter.

## Yaw

``` c
float yaw;

yaw = MPU6050_GetYaw();
```

## Roll

``` c
float roll;

roll = MPU6050_GetRoll();
```

## Pitch

``` c
float pitch;

pitch = MPU6050_GetPitch();
```

Tóm lại có hai cách lấy góc:

``` c
float yaw1 = MPU_Yaw;
```

hoặc:

``` c
float yaw2 = MPU6050_GetYaw();
```

Với thư viện hiện tại, hai cách đều lấy dữ liệu Yaw mà thư viện đang cập
nhật.

------------------------------------------------------------------------

# 11. Lấy toàn bộ dữ liệu bằng struct

Nếu cần nhiều thông số cùng lúc:

``` c
MPU6050_Data_t *imu;

imu = MPU6050_GetData();
```

Sau đó:

``` c
imu->Yaw
imu->Roll
imu->Pitch
```

Gyroscope:

``` c
imu->Gx
imu->Gy
imu->Gz
```

Accelerometer:

``` c
imu->Ax
imu->Ay
imu->Az
```

Thời gian mẫu:

``` c
imu->dt
```

RAW:

``` c
imu->Accel_X_RAW
imu->Accel_Y_RAW
imu->Accel_Z_RAW

imu->Gyro_X_RAW
imu->Gyro_Y_RAW
imu->Gyro_Z_RAW
```

Ví dụ:

``` c
MPU6050_Data_t *imu;

imu = MPU6050_GetData();

float yaw = imu->Yaw;
float gz  = imu->Gz;
float ax  = imu->Ax;
```

------------------------------------------------------------------------

# 12. Các hàm lấy Gyroscope

``` c
float gx = MPU6050_GetGx();
float gy = MPU6050_GetGy();
float gz = MPU6050_GetGz();
```

Đơn vị:

``` text
deg/s
```

------------------------------------------------------------------------

# 13. Các hàm lấy Accelerometer

``` c
float ax = MPU6050_GetAx();
float ay = MPU6050_GetAy();
float az = MPU6050_GetAz();
```

Đơn vị:

``` text
g
```

------------------------------------------------------------------------

# 14. Lấy thời gian mẫu

``` c
float dt;

dt = MPU6050_GetDt();
```

Đơn vị:

``` text
second
```

Với chu kỳ hiện tại thường xấp xỉ:

``` text
0.010 s
```

------------------------------------------------------------------------

# 15. Lấy Gyro Z Offset

``` c
float offset;

offset = MPU6050_GetGyroZOffset();
```

Offset được tạo khi chạy:

``` c
MPU6050_Calibrate_GyroZ();
```

------------------------------------------------------------------------

# 16. Reset Yaw

Khi muốn lấy hướng hiện tại làm mốc `0°`:

``` c
MPU6050_ResetYaw();
```

Không nên reset bằng:

``` c
MPU_Yaw = 0.0f;
```

vì thư viện còn có giá trị Yaw nội bộ.

Luôn dùng:

``` c
MPU6050_ResetYaw();
```

------------------------------------------------------------------------

# 17. Debug Yaw, Roll, Pitch trong Keil

Nếu hiện tại chỉ muốn kiểm tra cảm biến, chưa dùng các góc để điều khiển
gì, `while(1)` chỉ cần:

``` c
while (1)
{
    MPU6050_Loop();
}
```

Sau đó Build project:

``` text
F7
```

Đảm bảo:

``` text
0 Error(s)
```

Vào Debug và cho chương trình chạy.

Mở:

``` text
View
→ Watch Windows
→ Watch 1
```

Trong cột `Expression`, nhập:

``` text
MPU_Yaw
MPU_Roll
MPU_Pitch
```

Ví dụ Watch có thể hiển thị:

``` text
Expression        Value

MPU_Yaw           89.95
MPU_Roll           0.42
MPU_Pitch         -0.31
```

Đây chính là các biến mà chương trình có thể sử dụng sau này.

------------------------------------------------------------------------

# 18. Không nhập biến vào Memory Window để Debug kiểu này

Không nhập:

``` text
MPU_Yaw
```

vào:

``` text
Memory 1
```

để xem như Watch.

`Memory Window` chủ yếu dùng để xem vùng nhớ theo địa chỉ.

Để xem giá trị biến:

``` text
MPU_Yaw
MPU_Roll
MPU_Pitch
```

hãy dùng:

``` text
Watch 1
```

------------------------------------------------------------------------

# 19. Debug các thông số khác

Nếu muốn xem thêm bằng Watch, có thể thử Getter/struct tùy ngữ cảnh,
nhưng cách đơn giản nhất cho 3 góc vẫn là:

``` text
MPU_Yaw
MPU_Roll
MPU_Pitch
```

Ba biến public được tạo chính để dễ sử dụng và dễ quan sát.

------------------------------------------------------------------------

# 20. Ví dụ main.c khi chỉ Debug MPU6050

``` c
#include "main.h"

/* USER CODE BEGIN Includes */

#include "mpu6050.h"

/* USER CODE END Includes */


/* ... */


int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();


    /* USER CODE BEGIN 2 */

    if (MPU6050_Init(&hi2c1) == HAL_OK)
    {
        HAL_Delay(500);

        MPU6050_Calibrate_GyroZ();
    }

    /* USER CODE END 2 */


    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

        MPU6050_Loop();
    }

    /* USER CODE END 3 */
}
```

Sau đó vào Watch và nhập:

``` text
MPU_Yaw
MPU_Roll
MPU_Pitch
```

Không cần tạo:

``` c
Yaw_Debug
Roll_Debug
Pitch_Debug
```

------------------------------------------------------------------------

# 21. Ví dụ main.c khi bắt đầu sử dụng Yaw

Sau này khi cần dùng góc:

``` c
while (1)
{
    MPU6050_Loop();

    if (MPU_Yaw >= 90.0f)
    {
        /* Code cần thực hiện */
    }
}
```

Và đồng thời trong Keil Watch vẫn có thể xem:

``` text
MPU_Yaw
MPU_Roll
MPU_Pitch
```

Tức là cùng một giá trị vừa được chương trình sử dụng, vừa được Debug.

------------------------------------------------------------------------

# 22. Dùng trong file khác

Ví dụ có:

``` text
control.c
```

Đầu file:

``` c
#include "mpu6050.h"
```

Sau đó:

``` c
void Control(void)
{
    if (MPU_Yaw >= 90.0f)
    {
        /* xử lý */
    }
}
```

Không cần khai báo lại:

``` c
extern float MPU_Yaw;
```

vì `mpu6050.h` đã làm việc đó.

------------------------------------------------------------------------

# 23. Danh sách hàm của thư viện

  Hàm / biến                    Chức năng
  ----------------------------- -----------------------------
  `MPU6050_Init(&hi2c1)`        Khởi tạo MPU6050
  `MPU6050_Calibrate_GyroZ()`   Calibration Gyro Z
  `MPU6050_Loop()`              Tự cập nhật MPU theo chu kỳ
  `MPU6050_Update()`            Cập nhật MPU trực tiếp
  `MPU_Yaw`                     Giá trị Yaw public
  `MPU_Roll`                    Giá trị Roll public
  `MPU_Pitch`                   Giá trị Pitch public
  `MPU6050_GetYaw()`            Lấy Yaw qua Getter
  `MPU6050_GetRoll()`           Lấy Roll qua Getter
  `MPU6050_GetPitch()`          Lấy Pitch qua Getter
  `MPU6050_GetGx()`             Lấy Gyro X
  `MPU6050_GetGy()`             Lấy Gyro Y
  `MPU6050_GetGz()`             Lấy Gyro Z
  `MPU6050_GetAx()`             Lấy Accel X
  `MPU6050_GetAy()`             Lấy Accel Y
  `MPU6050_GetAz()`             Lấy Accel Z
  `MPU6050_GetDt()`             Lấy thời gian mẫu
  `MPU6050_GetGyroZOffset()`    Lấy offset Gyro Z
  `MPU6050_GetData()`           Lấy toàn bộ struct dữ liệu
  `MPU6050_ResetYaw()`          Reset Yaw về 0°

------------------------------------------------------------------------

# 24. Quy trình sử dụng chuẩn

``` text
1. #include "mpu6050.h"
          │
          ▼
2. MPU6050_Init(&hi2c1)
          │
          ▼
3. Giữ MPU đứng yên
          │
          ▼
4. MPU6050_Calibrate_GyroZ()
          │
          ▼
5. while(1)
   {
       MPU6050_Loop();
   }
          │
          ▼
6. Dữ liệu được cập nhật
          │
     ┌────┼────┐
     ▼    ▼    ▼
   Yaw   Roll Pitch
     │    │    │
     └────┼────┘
          │
      ┌───┴────┐
      ▼        ▼
   CODE      DEBUG
```

------------------------------------------------------------------------

# 25. Ghi nhớ nhanh

## Muốn MPU hoạt động

``` c
MPU6050_Loop();
```

## Muốn lấy Yaw

``` c
MPU_Yaw
```

## Muốn lấy Roll

``` c
MPU_Roll
```

## Muốn lấy Pitch

``` c
MPU_Pitch
```

## Muốn Reset Yaw

``` c
MPU6050_ResetYaw();
```

## Muốn xem Debug

``` text
Keil
→ Debug
→ View
→ Watch Windows
→ Watch 1
```

Nhập:

``` text
MPU_Yaw
MPU_Roll
MPU_Pitch
```

------------------------------------------------------------------------

# 26. Tóm tắt cuối

Khi **chỉ Debug cảm biến**:

``` c
while (1)
{
    MPU6050_Loop();
}
```

Watch:

``` text
MPU_Yaw
MPU_Roll
MPU_Pitch
```

Khi **sau này cần sử dụng**:

``` c
if (MPU_Yaw >= 90.0f)
{
    /* xử lý */
}
```

Hoặc:

``` c
float yaw = MPU6050_GetYaw();
```

Hoặc lấy cả bộ:

``` c
MPU6050_Data_t *imu = MPU6050_GetData();
```

Đây là ba cách truy cập dữ liệu đã được thư viện cung cấp; với việc vừa
code vừa Debug 3 góc chính, ưu tiên dùng:

``` c
MPU_Yaw
MPU_Roll
MPU_Pitch
```
