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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  TEST_PASS = 0,
  TEST_ERROR,
  TEST_TIMEOUT,
  TEST_MISMATCH
} TestResult_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C_ADDR        (0x30 << 1)

/* If your board LED is active low, swap SET/RESET here */
#define LED1_ON()       HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)
#define LED1_OFF()      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)

#define LED2_ON()       HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET)
#define LED2_OFF()      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET)

#define LED3_ON()       HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET)
#define LED3_OFF()      HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t txData[] = "HELLO_I2C";
uint8_t rxData[sizeof(txData)];

volatile uint8_t slaveRxDone = 0;
volatile uint8_t i2cError = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
static void Prepare_I2C_Test(void);
static TestResult_t Run_I2C_Test(void);
static TestResult_t Check_I2C_Data(void);
static void Recover_I2C_Bus(void);

static void Indicate_Test_Pass(void);
static void Indicate_Test_Error(void);
static void Indicate_Test_Mismatch(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
static void Prepare_I2C_Test(void)
{
  i2cError = 0;
  slaveRxDone = 0;
  memset(rxData, 0, sizeof(rxData));
}

static TestResult_t Run_I2C_Test(void)
{
  HAL_StatusTypeDef status;
  uint32_t tickStart;
  printf("==========================Test Start=========================\r\n");
  /* 1. Let I2C2 (Slave) enter receive state first */
  status = HAL_I2C_Slave_Receive_IT(&hi2c2, rxData, sizeof(rxData));
  if (status != HAL_OK)
  {
    return TEST_ERROR;
  }

  /* Small delay to ensure slave is ready */
  HAL_Delay(1);

  /* 2. Then transmit from I2C1 (Master) */
  status = HAL_I2C_Master_Transmit(&hi2c1, I2C_ADDR, txData, sizeof(txData), 1000);
  if (status != HAL_OK)
  {
    return TEST_ERROR;
  }

  /* 3. Wait for slave receive complete */
  tickStart = HAL_GetTick();
  while (slaveRxDone == 0U)
  {
    if ((HAL_GetTick() - tickStart) > 1000U)
    {
      return TEST_TIMEOUT;
    }
  }

  return TEST_PASS;
}

static TestResult_t Check_I2C_Data(void)
{
  if (memcmp(txData, rxData, sizeof(txData)) == 0)
  {
    return TEST_PASS;
  }

  return TEST_MISMATCH;
}

static void Recover_I2C_Bus(void)
{
  HAL_I2C_DeInit(&hi2c1);
  HAL_I2C_DeInit(&hi2c2);

  HAL_Delay(10);

  MX_I2C1_Init();
  MX_I2C2_Init();
}

static void Indicate_Test_Pass(void)
{
  LED1_ON();
  LED2_ON();
  LED3_ON();
  printf("pass\r\n");
}

static void Indicate_Test_Error(void)
{
  LED1_OFF();
  LED2_OFF();
  LED3_OFF();
  printf("fail\r\n");
}

static void Indicate_Test_Mismatch(void)
{
  LED1_ON();
  LED2_OFF();
  LED3_OFF();
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C2)
  {
    slaveRxDone = 1;
  }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
  (void)hi2c;
  i2cError = 1;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  TestResult_t testResult;

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
  MX_I2C2_Init();
  MX_USART3_UART_Init();

  /* USER CODE BEGIN 2 */
  LED1_OFF();
  LED2_OFF();
  LED3_OFF();
  HAL_Delay(100);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    Prepare_I2C_Test();

    testResult = Run_I2C_Test();

    if ((testResult != TEST_PASS) || (i2cError != 0U))
    {
      Recover_I2C_Bus();
      Indicate_Test_Error();
      HAL_Delay(500);
      continue;
    }

    testResult = Check_I2C_Data();

    if (testResult == TEST_PASS)
    {
      Indicate_Test_Pass();
    }
    else
    {
      Indicate_Test_Mismatch();
    }

    HAL_Delay(500);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 275;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                              | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
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