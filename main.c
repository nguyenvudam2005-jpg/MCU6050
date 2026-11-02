/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <math.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

#define MPU6050_ADDR (0x68 << 1)
#define RAD_TO_DEG 57.2957795f

#define IMU_ALPHA 0.90f

#define ROLL_DEADBAND   1.0f
#define PITCH_DEADBAND  1.0f
#define GYRO_DEADBAND   0.5f

uint8_t WHO_AM_I = 0;
uint8_t MPU_Data[14];

/* Accelerometer RAW */
int16_t Accel_X_RAW = 0;
int16_t Accel_Y_RAW = 0;
int16_t Accel_Z_RAW = 0;

/* Gyroscope RAW */
int16_t Gyro_X_RAW = 0;
int16_t Gyro_Y_RAW = 0;
int16_t Gyro_Z_RAW = 0;
float Gyro_Z_Offset = 0.0f;

/* Accelerometer đơn vị g */
float Ax = 0.0f;
float Ay = 0.0f;
float Az = 0.0f;

/* Gyroscope đơn vị d°/s */
float Gx = 0.0f;
float Gy = 0.0f;
float Gz = 0.0f;

/* Gọi IMU */
float Roll  = 0.0f;
float Pitch = 0.0f;
float Yaw   = 0.0f;

float Roll_Raw  = 0.0f;
float Pitch_Raw = 0.0f;

// Varibale
uint32_t IMU_LastTime = 0;
float IMU_dt = 0.0f;

HAL_StatusTypeDef MPU_Status;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void MPU6050_Init(void)
{
    uint8_t data;

    // Check MPU6050
   MPU_Status = HAL_I2C_Mem_Read(&hi2c1,
                     MPU6050_ADDR,
                     0x75,
                     I2C_MEMADD_SIZE_8BIT,
                     &WHO_AM_I,
                     1,
                     100);

    if (WHO_AM_I == 0x68)
    {
        // Make up MPU6050
        data = 0x00;

        HAL_I2C_Mem_Write(&hi2c1,
                          MPU6050_ADDR,
                          0x6B,
                          I2C_MEMADD_SIZE_8BIT,
                          &data,
                          1,
                          100);
			
				// Accelerometer +-2g
				data = 0x00;

				HAL_I2C_Mem_Write(&hi2c1,
                  MPU6050_ADDR,
                  0x1C,
                  I2C_MEMADD_SIZE_8BIT,
                  &data,
                  1,
                  100);

        // Gyro +-250 d?/s
        data = 0x00;

        HAL_I2C_Mem_Write(&hi2c1,
                          MPU6050_ADDR,
                          0x1B,
                          I2C_MEMADD_SIZE_8BIT,
                          &data,
                          1,
                          100);
    }
}


void MPU6050_Read_IMU(void)
{
	
	uint32_t now = HAL_GetTick();
	
	IMU_dt = (now - IMU_LastTime) / 1000.0f;
	
	IMU_LastTime = now;
	
    /* ================= Read MPU6050 ================= */

    HAL_I2C_Mem_Read(&hi2c1,
                     MPU6050_ADDR,
                     0x3B,
                     I2C_MEMADD_SIZE_8BIT,
                     MPU_Data,
                     14,
                     100);


    /* ================= ACCEL RAW ================= */

    Accel_X_RAW = (int16_t)((MPU_Data[0] << 8) |
                             MPU_Data[1]);

    Accel_Y_RAW = (int16_t)((MPU_Data[2] << 8) |
                             MPU_Data[3]);

    Accel_Z_RAW = (int16_t)((MPU_Data[4] << 8) |
                             MPU_Data[5]);


    /* ================= GYRO RAW ================= */

    Gyro_X_RAW = (int16_t)((MPU_Data[8] << 8) |
                            MPU_Data[9]);

    Gyro_Y_RAW = (int16_t)((MPU_Data[10] << 8) |
                            MPU_Data[11]);

    Gyro_Z_RAW = (int16_t)((MPU_Data[12] << 8) |
                            MPU_Data[13]);


    /* ================= RAW -> g ================= */

    Ax = (float)Accel_X_RAW / 16384.0f;
    Ay = (float)Accel_Y_RAW / 16384.0f;
    Az = (float)Accel_Z_RAW / 16384.0f;


    /* ================= RAW -> deg/s ================= */

    Gx = (float)Gyro_X_RAW / 131.0f;
    Gy = (float)Gyro_Y_RAW / 131.0f;
    Gz = ((float)Gyro_Z_RAW - Gyro_Z_Offset) / 131.0f;


		/* G�c RAW */
		Roll_Raw = atan2f(Ay, Az) * RAD_TO_DEG;

		Pitch_Raw = atan2f(-Ax, sqrtf((Ay * Ay) + (Az * Az))) * RAD_TO_DEG;


		/* Low Pass Filter */
		Roll = IMU_ALPHA * Roll + (1.0f - IMU_ALPHA) * Roll_Raw;

		Pitch = IMU_ALPHA * Pitch + (1.0f - IMU_ALPHA) * Pitch_Raw;


    /* ================= YAW ================= */

    if (Gz > -GYRO_DEADBAND && Gz < GYRO_DEADBAND)
		{
    Gz = 0.0f;
		}

		Yaw = Yaw + (Gz * IMU_dt);
		}

		void MPU6050_Calibrate_GyroZ(void)
{
    int32_t sum = 0;
    int16_t gz_raw;
    uint8_t data[2];

    for (int i = 0; i < 500; i++)
    {
        HAL_I2C_Mem_Read(&hi2c1,
                         MPU6050_ADDR,
                         0x47,
                         I2C_MEMADD_SIZE_8BIT,
                         data,
                         2,
                         100);

        gz_raw = (int16_t)((data[0] << 8) | data[1]);

        sum += gz_raw;

        HAL_Delay(2);
    }

    Gyro_Z_Offset = (float)sum / 500.0f;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	
	MPU6050_Init();
	
	HAL_Delay(500);
	
	MPU6050_Calibrate_GyroZ();

	Yaw = 0.0f;
	
	IMU_LastTime = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

		while (1)
{
		if((HAL_GetTick() - IMU_LastTime) >= 10)
			{
				MPU6050_Read_IMU();
			}
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
}	

  }
  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
