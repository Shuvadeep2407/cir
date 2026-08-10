/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "w5100.h"
#include <stdio.h>
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

I2S_HandleTypeDef hi2s2;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

/* Last SPI result, retained for W5100 startup diagnostics. */
static HAL_StatusTypeDef w5100_spi_last_status = HAL_OK;
static uint32_t w5100_spi_last_error = HAL_SPI_ERROR_NONE;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S2_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);

/* USER CODE BEGIN PFP */
// W5100 IO functions for STM32 HAL
void w5100_spi_select(void) {
    // Note: The pin name in main.h is W5500_CS_Pin, but this is for a W5100.
    // Ensure this is the correct pin for your W5100 shield.
    // Arduino Ethernet Shields typically use D10 (PB6 on Nucleo-F401RE).
    // Your main.h shows W5500_CS_Pin on GPIOB, Pin 6. This should work.
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);
}

void w5100_spi_unselect(void) {
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
}

void w5100_spi_write(uint8_t byte) {
    w5100_spi_last_status = HAL_SPI_Transmit(&hspi1, &byte, 1, W5100_SPI_TIMEOUT);
    w5100_spi_last_error = HAL_SPI_GetError(&hspi1);
}

uint8_t w5100_spi_read(void) {
    uint8_t byte = 0xFF;
    uint8_t dummy = 0xFF;
    w5100_spi_last_status = HAL_SPI_TransmitReceive(&hspi1, &dummy, &byte, 1, W5100_SPI_TIMEOUT);
    w5100_spi_last_error = HAL_SPI_GetError(&hspi1);
    return byte;
}

/* Diagnostic-only probe. It is safe for a W5100 because its command byte is
   not a valid W5100 opcode. A genuine W5500 returns 0x04 from VERSIONR. */
static uint8_t w5500_read_version(void) {
    uint8_t version;

    w5100_spi_select();
    w5100_spi_write(0x00);  /* VERSIONR address, MSB */
    w5100_spi_write(0x39);  /* VERSIONR address, LSB */
    w5100_spi_write(0x04);  /* W5500 common-register, read, VDM control */
    version = w5100_spi_read();
    w5100_spi_unselect();
    return version;
}

void w5100_reset(void) {
    // The Arduino Ethernet shield doesn't have a dedicated reset pin connected
    // to the MCU by default. You can either perform a software reset or
    // wire the W5100's reset pin to an MCU GPIO.
    // If you have wired it (e.g., to PB0), you would do:
    // HAL_GPIO_WritePin(W5100_RST_GPIO_Port, W5100_RST_Pin, GPIO_PIN_RESET);
    // HAL_Delay(1);
    // HAL_GPIO_WritePin(W5100_RST_GPIO_Port, W5100_RST_Pin, GPIO_PIN_SET);
    // HAL_Delay(10);
    // For now, we will rely on the software reset in W5100_Init().
}

// For printf redirection to UART
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

int _write(int file, char *ptr, int len)
{
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        __io_putchar(*ptr++);
    }
    return len;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2S2_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  printf("Bare-metal TCP Server Starting...\r\n");

  // 1. Initialize W5100 driver with I/O functions
  w5100_io_t io_bindings = {
      .spi_select   = w5100_spi_select,
      .spi_unselect = w5100_spi_unselect,
      .spi_read_byte  = w5100_spi_read,
      .spi_write_byte = w5100_spi_write,
      .reset        = w5100_reset, // Rely on software reset
      .delay_ms     = HAL_Delay
  };

  // 1b. Initialize W5100. Returns 0 on success, -1 if SPI not responding.
  if (W5100_Init(&io_bindings) != 0) {
      uint8_t mr_after_reset;
      uint8_t mr_after_write;
      uint8_t w5500_version;

      /* A W5100 must return 0x00 after reset and 0x10 after writing PB.
         These values identify whether this is a bus/CS fault or a bad reset. */
      W5100_Reset();
      mr_after_reset = W5100_ReadByte(W5100_MR);
      W5100_WriteByte(W5100_MR, W5100_MR_PB);
      mr_after_write = W5100_ReadByte(W5100_MR);
      W5100_WriteByte(W5100_MR, 0x00);
      w5500_version = w5500_read_version();

      printf("ERROR: W5100 not responding! Check SPI wiring (SCK=PA5, MISO=PA6, MOSI=PA7) and CS (PB6).\r\n");
      printf("SPI diagnostic: MR after reset=0x%02X (expected 0x00), after PB write=0x%02X (expected 0x10)\r\n",
             mr_after_reset, mr_after_write);
      printf("Controller probe: W5500 VERSIONR=0x%02X (W5500 expected 0x04)\r\n", w5500_version);
      printf("HAL SPI diagnostic: status=%d, error=0x%08lX\r\n",
             (int)w5100_spi_last_status, (unsigned long)w5100_spi_last_error);
      printf("Halting. Check wiring and reset.\r\n");
      while(1) { HAL_Delay(1000); }
  }

  printf("W5100 SPI communication verified.\r\n");

  // 2. Configure network parameters
  // IMPORTANT: The STM32 must be on the SAME subnet as the PC.
  // PC is on 10.87.243.x (subnet 255.255.255.0, gateway 10.87.243.71).
  // Use a free IP on that subnet, e.g., 10.87.243.100.
  w5100_net_config_t net_config = {
      .mac = {W5100_DEFAULT_MAC0, W5100_DEFAULT_MAC1, W5100_DEFAULT_MAC2, W5100_DEFAULT_MAC3, W5100_DEFAULT_MAC4, W5100_DEFAULT_MAC5},
      .ip = {10, 87, 243, 100}, // Static IP on PC's subnet
      .gateway = {10, 87, 243, 71}, // PC's gateway
      .subnet = {255, 255, 255, 0}  // Same subnet mask as PC
  };
  W5100_ConfigureNetwork(&net_config);

  printf("W5100 Initialized\r\n");
  printf("IP Address: %d.%d.%d.%d\r\n", net_config.ip[0], net_config.ip[1], net_config.ip[2], net_config.ip[3]);
  printf("GW: %d.%d.%d.%d\r\n", net_config.gateway[0], net_config.gateway[1], net_config.gateway[2], net_config.gateway[3]);
  printf("SN: %d.%d.%d.%d\r\n", net_config.subnet[0], net_config.subnet[1], net_config.subnet[2], net_config.subnet[3]);

  // 3. TCP Server Logic
  // Communication protocol:
  //   Port 5000 - Control & Debug channel (echo test, status, raw commands)
  //   Port 5001 - Video data channel (raw JPEG/video frames)
  const uint8_t  CTRL_SOCKET = 0;
  const uint8_t  VIDEO_SOCKET = 1;
  const uint16_t CTRL_PORT   = 5000;
  const uint16_t VIDEO_PORT  = 5001;
  uint8_t  data_buffer[2048]; // Buffer for incoming/outgoing data
  uint32_t video_frame_counter = 0;

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // ========== Handle Control/Debug Socket (Port 5000) ==========
    uint8_t ctrl_status = W5100_SocketGetStatus(CTRL_SOCKET);

    switch(ctrl_status)
    {
      case W5100_SNSR_CLOSED:
        // Attempt to open and listen on control socket
        if (W5100_SocketInit(CTRL_SOCKET, W5100_SNMR_TCP, CTRL_PORT, 0) != 0) {
          W5100_SocketListen(CTRL_SOCKET);
        }
        break;

      case W5100_SNSR_ESTABLISHED:
        // Process control commands from the PC
        uint16_t ctrl_rx = W5100_SocketGetRXReceivedSize(CTRL_SOCKET);
        if (ctrl_rx > 0) {
          uint16_t n = W5100_SocketRecv(CTRL_SOCKET, data_buffer, ctrl_rx);
          if (n > 0) {
            // Echo back for debugging (confirms reception)
            W5100_SocketSend(CTRL_SOCKET, data_buffer, n);
            printf("[DBG] Rx %u bytes: %.*s\r\n", n, n, (char*)data_buffer);
          }
        }
        break;

      case W5100_SNSR_CLOSE_WAIT:
        // Client disconnected from control socket, close it to reset
        W5100_SocketDisconnect(CTRL_SOCKET);
        break;

      case W5100_SNSR_LISTEN:
        // Waiting for a connection, do nothing.
        break;

      default:
        // Unexpected state, close the socket to reset it.
        W5100_SocketClose(CTRL_SOCKET);
        break;
    }

    // ========== Handle Video Socket (Port 5001) ==========
    uint8_t video_status = W5100_SocketGetStatus(VIDEO_SOCKET);

    switch(video_status)
    {
      case W5100_SNSR_CLOSED:
        // Attempt to open and listen on video socket
        if (W5100_SocketInit(VIDEO_SOCKET, W5100_SNMR_TCP, VIDEO_PORT, 0) != 0) {
          W5100_SocketListen(VIDEO_SOCKET);
        }
        break;

      case W5100_SNSR_ESTABLISHED:
        // Send a video frame periodically. Check free TX buffer first.
        uint16_t free_tx = W5100_SocketGetTXFreeSize(VIDEO_SOCKET);
        if (free_tx > 128) { // Ensure enough space for the frame
          // Build a mock video frame header + payload
          int vlen = snprintf((char*)data_buffer, sizeof(data_buffer),
                              "[VIDEO] Frame %lu | size=128 | timestamp=%lu\r\n",
                              video_frame_counter,
                              HAL_GetTick());
          uint16_t sent = W5100_SocketSend(VIDEO_SOCKET, data_buffer, vlen);
          if (sent > 0) {
            video_frame_counter++;
          }
        }
        break;

      case W5100_SNSR_CLOSE_WAIT:
        W5100_SocketDisconnect(VIDEO_SOCKET);
        break;

      case W5100_SNSR_LISTEN:
        // Waiting for a connection, do nothing.
        break;

      default:
        W5100_SocketClose(VIDEO_SOCKET);
        break;
    }

    HAL_Delay(100); // Main loop period: ~10 Hz
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
  * @brief I2S2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S2_Init(void)
{

  /* USER CODE BEGIN I2S2_Init 0 */

  /* USER CODE END I2S2_Init 0 */

  /* USER CODE BEGIN I2S2_Init 1 */

  /* USER CODE END I2S2_Init 1 */
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_8K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S2_Init 2 */

  /* USER CODE END I2S2_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

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
  /* Use a conservative 2.625 MHz while bringing up the shield. PA5/D13
     also drives the Nucleo user LED, and long shield/ICSP traces can make
     a 21 MHz signal unreliable. Increase only after the MR test is stable. */
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
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
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  /* DMA2_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : c1101_cs_Pin */
  GPIO_InitStruct.Pin = c1101_cs_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(c1101_cs_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : W5500_CS_Pin */
  GPIO_InitStruct.Pin = W5500_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(W5500_CS_GPIO_Port, &GPIO_InitStruct);

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
