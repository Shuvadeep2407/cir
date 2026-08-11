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
#include "w5100.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define W5100_SPI_TIMEOUT (100) // SPI timeout in ms

#define CTRL_SOCKET  0
#define VIDEO_SOCKET 1

#define CTRL_PORT  5000
#define VIDEO_PORT 5001

// Network Configuration
#define MAC_ADDR {0x00, 0x08, 0xDC, 0x01, 0x02, 0x03}
#define IP_ADDR  {192, 168, 1, 177}
#define SUB_MASK {255, 255, 255, 0}
#define GW_ADDR  {192, 168, 1, 1}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
uint8_t data_buffer[2048]; // Common buffer for socket data
uint32_t video_frame_counter = 0;

// Diagnostic variables for SPI communication
HAL_StatusTypeDef w5100_spi_last_status;
uint32_t w5100_spi_last_error;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void handle_tcp_socket(uint8_t sock_num, uint16_t port);
/* USER CODE BEGIN PFP */
// W5100 IO functions for STM32 HAL
void w5100_spi_select(void) {
    // Note: The pin name in main.h is W5500_CS_Pin, but this is for a W5100.
    // Ensure this is the correct pin for your W5100 shield.
    // Arduino Ethernet Shields typically use D10 (PB6 on Nucleo-F401RE).
    // Your main.h shows W5100_CS_Pin on GPIOB, Pin 6. This should work.
    HAL_GPIO_WritePin(W5100_CS_GPIO_Port, W5100_CS_Pin, GPIO_PIN_RESET);
}

void w5100_spi_unselect(void) {
    HAL_GPIO_WritePin(W5100_CS_GPIO_Port, W5100_CS_Pin, GPIO_PIN_SET);
}

void w5100_spi_write(uint8_t byte) {
    w5100_spi_last_status = HAL_SPI_Transmit(&hspi1, &byte, 1, W5100_SPI_TIMEOUT);
    w5100_spi_last_error = HAL_SPI_GetError(&hspi1);
}

uint8_t w5100_spi_read(void) { // This function returns the read byte
    uint8_t byte = 0xFF;
    uint8_t dummy = 0xFF;
    w5100_spi_last_status = HAL_SPI_TransmitReceive(&hspi1, &dummy, &byte, 1, W5100_SPI_TIMEOUT);
    w5100_spi_last_error = HAL_SPI_GetError(&hspi1);
    // The driver expects the read byte to be returned. The status is saved for debugging.
    return byte; 
}

void w5100_reset(void) {
    // This performs a hardware reset on the W5100.
    // It requires the W5100's RESET pin to be wired to PB0.
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_Delay(5); // A 5ms reset pulse is more than sufficient.
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_Delay(10); // Wait for the chip to stabilize after reset.
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define MAIN_LOOP_DELAY_MS 10

static void handle_tcp_socket(uint8_t sock_num, uint16_t port)
{
    uint8_t status = W5100_SocketGetStatus(sock_num);

    switch(status)
    {
        case W5100_SNSR_CLOSED:
            // Attempt to open and listen on the socket
            if (W5100_SocketInit(sock_num, W5100_SNMR_TCP, port, 0) != 0) {
                W5100_SocketListen(sock_num);
            }
            break;

        case W5100_SNSR_ESTABLISHED:
            if (sock_num == CTRL_SOCKET) {
                // Process control commands from the PC
                uint16_t ctrl_rx = W5100_SocketGetRXReceivedSize(CTRL_SOCKET);
                if (ctrl_rx > 0) {
                    uint16_t n = W5100_SocketRecv(CTRL_SOCKET, data_buffer, ctrl_rx);
                    if (n > 0) {
                        // Echo back for debugging (confirms reception)
                        W5100_SocketSend(CTRL_SOCKET, data_buffer, n);
                    }
                }
            } else if (sock_num == VIDEO_SOCKET) {
                // Send a video frame periodically. Check free TX buffer first.
                uint16_t free_tx = W5100_SocketGetTXFreeSize(VIDEO_SOCKET);
                if (free_tx > 128) { // Ensure enough space for the frame
                    // Build a mock video frame header + payload
                    const uint8_t* video_payload = (const uint8_t*)"[VIDEO] Frame data placeholder\r\n";
                    uint16_t payload_len = 32; // Corresponds to the length of the string above
                    if (payload_len > 0) {
                        uint16_t sent = W5100_SocketSend(VIDEO_SOCKET, video_payload, payload_len);
                        if (sent > 0) {
                            video_frame_counter++;
                        }
                    }
                }
            }
            break;

        case W5100_SNSR_CLOSE_WAIT:
            W5100_SocketDisconnect(sock_num);
            break;

        case W5100_SNSR_LISTEN: // Waiting for a connection, do nothing.
        default: // Unexpected state, close the socket to reset it.
            break;
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
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  // 1. Initialize the IO function struct for the driver
  w5100_io_t w5100_io_fns = {
      .spi_select   = w5100_spi_select,
      .spi_unselect = w5100_spi_unselect,
      .spi_read_byte = w5100_spi_read,
      .spi_write_byte = w5100_spi_write,
      .delay_ms = HAL_Delay,
      .reset = w5100_reset
  };
  // 2. Initialize the W5100. This function performs the hardware reset and
  //    communication test internally. If it fails, it will return -1.
  if (W5100_Init(&w5100_io_fns) != 0) {
      // W5100 initialization failed. This confirms a hardware issue
      // (wiring, power, or a faulty shield).
      Error_Handler();
  }

  // 3. Configure network parameters
  w5100_net_config_t net_cfg = {
      .mac = MAC_ADDR, .ip = IP_ADDR, .subnet = SUB_MASK, .gateway = GW_ADDR
  };
  W5100_ConfigureNetwork(&net_cfg);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {    
    handle_tcp_socket(CTRL_SOCKET, CTRL_PORT);
    handle_tcp_socket(VIDEO_SOCKET, VIDEO_PORT);

    HAL_Delay(MAIN_LOOP_DELAY_MS);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level for CS and Reset */
  HAL_GPIO_WritePin(GPIOB, W5100_CS_Pin|GPIO_PIN_0, GPIO_PIN_SET);

  /*Configure GPIO pins : PA5 PA6 PA7 for SPI1 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : W5100_CS_Pin (PB6) and PB0 (Reset) */
  GPIO_InitStruct.Pin = W5100_CS_Pin|GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM3 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM3)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
