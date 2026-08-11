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
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void DisplayNibble(uint8_t nibble);
static void boot(void);
static void CC1101_Strobe(uint8_t strobe);
static uint8_t CC1101_ReadReg(uint8_t addr);
static void Test_CC1101_Connection(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief Displays a 4-bit value (nibble) on the 4 LEDs.
  * @param nibble The 4-bit value to display.
  */
static void DisplayNibble(uint8_t nibble)
{
    HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, led2_Pin, (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, led3_Pin, (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, led4_Pin, (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
  * @brief Runs the startup LED sequence, spelling a message in ASCII.
  */
static void boot(void)
{
  const char msg[] = "om namah shivay";
  const int nibble_delay = 100;
  const int char_delay = 20;

  for (int i = 0; msg[i] != '\0'; i++)
  {
    uint8_t ascii_val = (uint8_t)msg[i];

    // Display high nibble then low nibble for each character
    DisplayNibble((ascii_val >> 4) & 0x0F);
    HAL_Delay(nibble_delay);
    DisplayNibble(ascii_val & 0x0F);
    HAL_Delay(nibble_delay);

    // Brief pause between characters
    DisplayNibble(0x00);
    HAL_Delay(char_delay);
  }

  // Turn all LEDs off at the end
  DisplayNibble(0x00);
  HAL_Delay(500);
}

/**
  * @brief Sends a command strobe to the CC1101.
  * @param strobe The command strobe to send (e.g., 0x30 for SRES).
  */
static void CC1101_Strobe(uint8_t strobe)
{
    uint32_t tickstart = HAL_GetTick();

    // Assert CS
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);

    // Wait for MISO to go low, indicating chip is ready. Timeout to prevent lockup.
    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET)
    {
        if ((HAL_GetTick() - tickstart) > 10) // 10ms timeout
        {
            HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
            return; // Exit on timeout
        }
    }

    // Transmit strobe command
    HAL_SPI_Transmit(&hspi2, &strobe, 1, 100);

    // De-assert CS
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

/**
  * @brief Reads a single register from the CC1101.
  * @param addr The register address (0x00 - 0x3D).
  * @retval The register value.
  */
static uint8_t CC1101_ReadReg(uint8_t addr)
{
    uint8_t tx_header = addr | 0x80; // Set R/W bit to 1 for read, single byte access
    uint8_t rx_data = 0xFF; // Default to error value
    uint32_t tickstart = HAL_GetTick();

    // Assert CS
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);

    // Wait for MISO to go low, indicating chip is ready. Timeout to prevent lockup.
    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET)
    {
        if ((HAL_GetTick() - tickstart) > 10) // 10ms timeout
        {
            HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
            return 0xFF; // Indicate timeout error
        }
    }

    HAL_SPI_Transmit(&hspi2, &tx_header, 1, 100); // Send address header
    HAL_SPI_Receive(&hspi2, &rx_data, 1, 100);    // Read register value

    // De-assert CS
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);

    return rx_data;
}

/**
  * @brief Tests the SPI connection to the CC1101 module.
  *        Blinks LED2 (green) if successful, LED1 (red) if failed.
  */
static void Test_CC1101_Connection(void)
{
    // Reset the CC1101 chip to ensure it's in a known state.
    CC1101_Strobe(0x30); // SRES strobe
    HAL_Delay(1);        // Short delay for the chip to stabilize after reset.

    // The VERSION register (address 0x31) should return a known value, e.g., 0x14.
    // A value of 0x00 or 0xFF often indicates a communication failure.
    uint8_t version = CC1101_ReadReg(0x31);

    if ((version != 0x00) && (version != 0xFF))
    {
        // Success: Blink LED2 (green) 5 times
        for (int i = 0; i < 5; i++)
        {
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_RESET);
            HAL_Delay(100);
        }
    }
    else
    {
        // Failure: Halt and blink LED1 (red) continuously
        Error_Handler();
    }
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
  MX_SPI2_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  boot();
  Test_CC1101_Connection();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
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
    // Continuous fast blink on LED1 to indicate a hard fault
    HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
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
