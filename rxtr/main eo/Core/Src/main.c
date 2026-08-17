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
#include "cmsis_os.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "w5500.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* Per-device receive info (7 remote CC1101 nodes). */
typedef struct
{
  uint8_t id;          /* 22,33,44,55,66,77,88                  */
  const char *name;    /* human-readable device name            */
  uint8_t type;        /* device class: ACUSTICK/RADAR/JAMMER    */
} DeviceInfo_t;

/* Packet received from a remote device, to be passed via queue. */

// MAX_PAYLOAD_LEN must be defined before RadioPacket_t
#define MAX_PAYLOAD_LEN         61
typedef struct
{
  uint8_t payload[MAX_PAYLOAD_LEN];
  uint8_t len;
  int8_t rssi;
  uint8_t lqi;
} RadioPacket_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Maximum packet payload length the receiver can handle
#define DEVICE_ID               11u // This receiver's ID

// Packet Types (must match transmitter)
#define RADIO_PACKET_TYPE_TELEM 0x01u
#define RADIO_PACKET_TYPE_LOG   0x02u
#define RADIO_PACKET_TYPE_ACK   0x03u
#define RADIO_PACKET_TYPE_AUDIO 0x04u

#define MAX_PAYLOAD_LEN         61

// Remote CC1101 node IDs this hub listens for (acustick/radar/jammer).
#define DEVICE_COUNT            7
#define DEVICE_CONNECT_TIMEOUT_MS 8000u   // considered "connected" if traffic seen inside this window
#define DEVICE_TYPE_ACUSTICK    0
#define DEVICE_TYPE_RADAR       1
#define DEVICE_TYPE_JAMMER      2

// CC1101 Registers
#define CC1101_IOCFG0           0x02        // GDO0 output pin configuration
#define CC1101_FIFOTHR          0x03        // RX FIFO and TX FIFO thresholds
#define CC1101_PKTLEN           0x06        // Packet length
#define CC1101_PKTCTRL1         0x07        // Packet automation control
#define CC1101_PKTCTRL0         0x08        // Packet automation control
#define CC1101_ADDR             0x09        // Device address
#define CC1101_CHANNR           0x0A        // Channel number
#define CC1101_FSCTRL1          0x0B        // Frequency synthesizer control
#define CC1101_FREQ2            0x0D        // Frequency control word, high byte
#define CC1101_FREQ1            0x0E        // Frequency control word, middle byte
#define CC1101_FREQ0            0x0F        // Frequency control word, low byte
#define CC1101_MDMCFG4          0x10        // Modem configuration
#define CC1101_MDMCFG3          0x11        // Modem configuration
#define CC1101_MDMCFG2          0x12        // Modem configuration
#define CC1101_MDMCFG1          0x13        // Modem configuration
#define CC1101_MDMCFG0          0x14        // Modem configuration
#define CC1101_DEVIATN          0x15        // Modem deviation setting
#define CC1101_MCSM1            0x17        // Main Radio Cntrl State Machine config
#define CC1101_MCSM0            0x18        // Main Radio Cntrl State Machine config
#define CC1101_FOCCFG           0x19        // Frequency Offset Compensation config
#define CC1101_BSCFG            0x1A        // Bit Synchronization configuration
#define CC1101_AGCCTRL2         0x1B        // AGC control
#define CC1101_AGCCTRL1         0x1C        // AGC control
#define CC1101_AGCCTRL0         0x1D        // AGC control
#define CC1101_FREND1           0x21        // Front end RX configuration
#define CC1101_FREND0           0x22        // Front end TX configuration
#define CC1101_FSCAL3           0x23        // Frequency synthesizer calibration
#define CC1101_FSCAL2           0x24        // Frequency synthesizer calibration
#define CC1101_FSCAL1           0x25        // Frequency synthesizer calibration
#define CC1101_FSCAL0           0x26        // Frequency synthesizer calibration
#define CC1101_TEST2            0x2C        // Various test settings
#define CC1101_TEST1            0x2D        // Various test settings
#define CC1101_TEST0            0x2E        // Various test settings

// Command Strobes
#define CC1101_SRES             0x30        // Reset chip.
#define CC1101_SRX              0x34        // Enable RX.
#define CC1101_STX              0x35        // Enable TX.
#define CC1101_SIDLE            0x36        // Exit RX / TX.
#define CC1101_SFRX             0x3A        // Flush the RX FIFO buffer.
#define CC1101_SFTX             0x3B        // Flush the TX FIFO buffer.
#define CC1101_SNOP             0x3D        // No operation.

// Status Registers
#define CC1101_PARTNUM          0x30        // Part number
#define CC1101_VERSION          0x31        // Current version number
#define CC1101_MARCSTATE        0x35        // Main Radio Control FSM state
#define CC1101_RXBYTES          0x3B        // Underflow and number of bytes

// FIFO Access & Header bits
#define CC1101_RXFIFO           0x3F
#define CC1101_TXFIFO           0x3F
#define CC1101_WRITE_BURST      0x40
#define CC1101_READ_SINGLE      0x80
#define CC1101_READ_BURST       0xC0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s2;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* Definitions for Task_MainEO */
osThreadId_t Task_MainEOHandle;
const osThreadAttr_t Task_MainEO_attributes = {
  .name = "Task_MainEO",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for Task_CC1101 */
osThreadId_t Task_CC1101Handle;
const osThreadAttr_t Task_CC1101_attributes = {
  .name = "Task_CC1101",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Task_PCComm */
osThreadId_t Task_PCCommHandle;
const osThreadAttr_t Task_PCComm_attributes = {
  .name = "Task_PCComm",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for RadioPacketQueue */
osMessageQueueId_t RadioPacketQueueHandle;
const osMessageQueueAttr_t RadioPacketQueue_attributes = {
  .name = "RadioPacketQueue"
};
/* USER CODE BEGIN PV */

/* Definitions for DeviceDataMutex */
osMutexId_t DeviceDataMutexHandle;
const osMutexAttr_t DeviceDataMutex_attributes = {
  .name = "DeviceDataMutex"
};

/* Static table of the 7 remote devices (index 0..6). */
static const DeviceInfo_t g_dev[DEVICE_COUNT] =
{
  { 22, "ACUSTICK1", DEVICE_TYPE_ACUSTICK },
  { 33, "ACUSTICK2", DEVICE_TYPE_ACUSTICK },
  { 44, "ACUSTICK3", DEVICE_TYPE_ACUSTICK },
  { 55, "RADAR1",    DEVICE_TYPE_RADAR    },
  { 66, "RADAR2",    DEVICE_TYPE_RADAR    },
  { 77, "RADAR3",    DEVICE_TYPE_RADAR    },
  { 88, "JAMMER",    DEVICE_TYPE_JAMMER   },
};

/* Latest valid payload + connection state per device. */
static uint8_t   g_dev_present[DEVICE_COUNT];
static uint8_t   g_dev_len[DEVICE_COUNT];
static uint8_t   g_dev_payload[DEVICE_COUNT][MAX_PAYLOAD_LEN];
static uint8_t   g_dev_type[DEVICE_COUNT];
static uint8_t   g_dev_seq[DEVICE_COUNT];
static uint32_t  g_dev_last_tick[DEVICE_COUNT];

/* USART2 RX line accumulator for PC commands. */
static uint8_t   g_cmd_line[16];
static uint8_t   g_cmd_n = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S2_Init(void);
static void MX_SPI3_Init(void);
static void MX_USART6_UART_Init(void);
void StartMainEOTask(void *argument);
void StartCC1101Task(void *argument);
void StartPCCommTask(void *argument);

/* USER CODE BEGIN PFP */
void cc1101_init(void);
uint8_t cc1101_read_reg(uint8_t addr);
void cc1101_send_packet(const uint8_t *data, uint8_t len);
void cc1101_command_strobe(uint8_t cmd);
void cc1101_read_burst(uint8_t addr, uint8_t *buffer, uint8_t count);
int8_t  Device_IndexOf(uint8_t src_id);
void    Device_Store(uint8_t src_id, const uint8_t *payload, uint8_t len);
void    Uart_CheckCommand(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void cc1101_select() {
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
}

void cc1101_deselect() {
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

void cc1101_write_reg(uint8_t addr, uint8_t data) {
    uint8_t tx_data[2] = {addr, data};
    cc1101_select();
    HAL_SPI_Transmit(&hspi1, tx_data, 2, 100);
    cc1101_deselect();
}

uint8_t cc1101_read_reg(uint8_t addr) {
    uint8_t tx_data[2] = {addr | CC1101_READ_SINGLE, 0x00}; // Address header, then dummy byte for clocking
    uint8_t rx_data[2] = {0};
    cc1101_select();
    // The first received byte is the status byte, the second is the register value
    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, 100);
    cc1101_deselect();
    return rx_data[1]; // Return the register value
}

void cc1101_read_burst(uint8_t addr, uint8_t *buffer, uint8_t count) {
    uint8_t tx_addr = addr | CC1101_READ_BURST;
    cc1101_select();
    HAL_SPI_Transmit(&hspi1, &tx_addr, 1, 100);
    HAL_SPI_Receive(&hspi1, buffer, count, 100);
    cc1101_deselect();
}

void cc1101_write_burst(uint8_t addr, const uint8_t *buffer, uint8_t count) {
    uint8_t tx_addr = addr | CC1101_WRITE_BURST;
    cc1101_select();
    HAL_SPI_Transmit(&hspi1, &tx_addr, 1, 100);
    HAL_SPI_Transmit(&hspi1, (uint8_t*)buffer, count, 100);
    cc1101_deselect();
}

void cc1101_command_strobe(uint8_t cmd) {
    cc1101_select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    cc1101_deselect();
}

void cc1101_reset() {
    cc1101_deselect();
    HAL_Delay(1);
    cc1101_select();
    HAL_Delay(1);
    cc1101_deselect();
    HAL_Delay(1);
    cc1101_command_strobe(CC1101_SRES);
    HAL_Delay(1);
}

void cc1101_send_packet(const uint8_t *data, uint8_t len) {
    if (len == 0 || len > MAX_PAYLOAD_LEN) {
        return;
    }

    uint8_t fifo_data[MAX_PAYLOAD_LEN + 1];
    fifo_data[0] = len;
    memcpy(&fifo_data[1], data, len);

    cc1101_command_strobe(CC1101_SIDLE);
    cc1101_command_strobe(CC1101_SFTX);
    cc1101_write_burst(CC1101_TXFIFO, fifo_data, len + 1);
    cc1101_command_strobe(CC1101_STX);

    // Wait for transmission to complete. MARCSTATE will be 0x01 (IDLE)
    // Add a timeout to prevent lockup in case of a hardware issue.
    uint32_t start_tick = HAL_GetTick();
    while (cc1101_read_reg(CC1101_MARCSTATE) != 0x01) {
        if (HAL_GetTick() - start_tick > 50) { break; } // 50ms timeout
    }
}
// A basic configuration for the CC1101 for 433MHz.
// This should be adapted for the specific application (frequency, data rate, etc.)
void cc1101_init() {
    cc1101_reset();

    // NOTE: The comments below were incorrect. The register values configure the radio
    // for ~153.5 kBaud, not 1.2 kBaud. The settings match the transmitter.
    static const uint8_t pa_table[] = { 0xC0 }; // approx. 10 dBm
    cc1101_write_reg(CC1101_FSCTRL1,  0x06); // IF Freq: 152.34375 kHz
    cc1101_write_reg(CC1101_FREQ2,    0x10); // Freq. Control Word, High Byte
    cc1101_write_reg(CC1101_FREQ1,    0xB0); // Freq. Control Word, Middle Byte
    cc1101_write_reg(CC1101_FREQ0,    0x71); // Freq. Control Word, Low Byte
    cc1101_write_reg(CC1101_MDMCFG4,  0xCA); // RX BW: 101.5625 kHz, Data Rate Exponent
    cc1101_write_reg(CC1101_MDMCFG3,  0x83); // Data Rate Mantissa (~153.5 kBaud)
    cc1101_write_reg(CC1101_MDMCFG2,  0x13); // GFSK, 30/32 sync word bits
    cc1101_write_reg(CC1101_MDMCFG1,  0x22); // 4-byte preamble, FEC disabled
    cc1101_write_reg(CC1101_MDMCFG0,  0xF8); // Channel spacing: 199.951172 kHz
    cc1101_write_reg(CC1101_CHANNR,   0x00); // Channel 0
    cc1101_write_reg(CC1101_DEVIATN,  0x35); // Deviation: 19.042969 kHz
    cc1101_write_reg(CC1101_FREND1,   0x56); // Front end RX configuration
    cc1101_write_reg(CC1101_FREND0,   0x10); // Front end TX configuration
    cc1101_write_reg(CC1101_MCSM0,    0x18); // Auto-calibrate on IDLE->RX/TX
    cc1101_write_reg(CC1101_FOCCFG,   0x16); // Freq Offset Comp. config
    cc1101_write_reg(CC1101_BSCFG,    0x6C); // Bit Synchronization config
    cc1101_write_reg(CC1101_AGCCTRL2, 0x43); // AGC Control
    cc1101_write_reg(CC1101_AGCCTRL1, 0x40); // AGC Control
    cc1101_write_reg(CC1101_AGCCTRL0, 0x91); // AGC Control
    cc1101_write_reg(CC1101_FSCAL3,   0xE9); // Freq. Synthesizer Cal.
    cc1101_write_reg(CC1101_FSCAL2,   0x2A); // Freq. Synthesizer Cal.
    cc1101_write_reg(CC1101_FSCAL1,   0x00); // Freq. Synthesizer Cal.
    cc1101_write_reg(CC1101_FSCAL0,   0x1F); // Freq. Synthesizer Cal.
    cc1101_write_reg(CC1101_TEST2,    0x81); // Required for sensitivity at low data rates
    cc1101_write_reg(CC1101_TEST1,    0x35); // Required for sensitivity at low data rates
    cc1101_write_reg(CC1101_TEST0,    0x09); // Required for sensitivity at low data rates
    cc1101_write_reg(CC1101_PKTCTRL1, 0x04); // No address check, CRC autoflush on
    cc1101_write_reg(CC1101_PKTCTRL0, 0x05); // Variable packet length, CRC enabled
    cc1101_write_reg(CC1101_ADDR,     0x00); // Device address (not used, PKTCTRL1)
    cc1101_write_reg(CC1101_PKTLEN,   0xFF); // Max packet length

    cc1101_write_burst(0x3E, pa_table, sizeof(pa_table)); // Write PATABLE for TX power
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
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_I2S2_Init();
  MX_SPI3_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  cc1101_init();

  // Check if CC1101 is present by reading its version register
  uint8_t version = cc1101_read_reg(CC1101_VERSION);
  char startup_msg[64];
  if (version == 0x14 || version == 0x11 || version == 0x0f) { // 0x14 is common, 0x11/0x0f might be clones/other versions
      int len = snprintf(startup_msg, sizeof(startup_msg), "CC1101 Detected. Version: 0x%02X\r\n", version);
      HAL_UART_Transmit(&huart2, (uint8_t*)startup_msg, len, 100);
  } else {
      int len = snprintf(startup_msg, sizeof(startup_msg), "CC1101 Not Found. Version read: 0x%02X\r\n", version);
      HAL_UART_Transmit(&huart2, (uint8_t*)startup_msg, len, 100);
  }

  W5500_SetSpiBus(W5500_SPI_BUS_3); // Main operation default: W5500 on SPI3, CS PC8
  W5500_Init(); // Initialize the W5500 chip
  if (W5500_TestConnection())
  {
      int len = snprintf(startup_msg, sizeof(startup_msg), "W5500 OK %s CS=PC8 VERSIONR=0x04\r\n", W5500_GetSpiBusName());
      HAL_UART_Transmit(&huart2, (uint8_t*)startup_msg, len, 100);
  }
  else
  {
      uint8_t w5500_version = W5500_ReadByte(0x0039);
      int len = snprintf(startup_msg, sizeof(startup_msg), "W5500 FAIL %s CS=PC8 VERSIONR=0x%02X\r\n", W5500_GetSpiBusName(), w5500_version);
      HAL_UART_Transmit(&huart2, (uint8_t*)startup_msg, len, 100);
  }
  cc1101_command_strobe(CC1101_SRX); // Enter RX mode

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* creation of DeviceDataMutex */
  DeviceDataMutexHandle = osMutexNew(&DeviceDataMutex_attributes);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of RadioPacketQueue */
  RadioPacketQueueHandle = osMessageQueueNew (20, sizeof(RadioPacket_t), &RadioPacketQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_MainEO */
  Task_MainEOHandle = osThreadNew(StartMainEOTask, NULL, &Task_MainEO_attributes);

  /* creation of Task_CC1101 */
  Task_CC1101Handle = osThreadNew(StartCC1101Task, NULL, &Task_CC1101_attributes);

  /* creation of Task_PCComm */
  Task_PCCommHandle = osThreadNew(StartPCCommTask, NULL, &Task_PCComm_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) // This loop should not be reached
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
  // For MEMS microphone, I2S should be in Master RX mode
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
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

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
  HAL_GPIO_WritePin(GPIOC, a1_Pin|b1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, a2_Pin|b2_Pin|c1101_cs_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(w5500_cs_GPIO_Port, w5500_cs_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : a1_Pin b1_Pin */
  GPIO_InitStruct.Pin = a1_Pin|b1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : a2_Pin b2_Pin c1101_cs_Pin */
  GPIO_InitStruct.Pin = a2_Pin|b2_Pin|c1101_cs_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : w5500_cs_Pin */
  GPIO_InitStruct.Pin = w5500_cs_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(w5500_cs_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* ============ Helper: find table index for a source ID, or -1 ============ */
int8_t Device_IndexOf(uint8_t src_id)
{
  uint8_t i;
  for (i = 0; i < DEVICE_COUNT; i++)
  {
    if (g_dev[i].id == src_id) { return (int8_t)i; }
  }
  return -1;
}

/* ============ Store the newest valid frame from a known device ============ */
void Device_Store(uint8_t src_id, const uint8_t *payload, uint8_t len)
{
  int8_t idx = Device_IndexOf(src_id);
  if (idx < 0 || len < 4 || len > MAX_PAYLOAD_LEN) { return; }
  
  osMutexAcquire(DeviceDataMutexHandle, osWaitForever);
  
  g_dev_present[idx]    = 1;
  g_dev_len[idx]        = len;
  g_dev_type[idx]       = payload[2];   /* packet type */
  g_dev_seq[idx]        = payload[3];   /* sequence    */
  memcpy(g_dev_payload[idx], payload, len);
  g_dev_last_tick[idx]  = HAL_GetTick();

  osMutexRelease(DeviceDataMutexHandle);
}

/*
 * To fully integrate a MEMS microphone, you would typically:
 * 1. Include ST's PDM-to-PCM conversion library (e.g., from X-CUBE-MEMSMIC1).
 * 2. Define a buffer to store raw PDM data received via DMA.
 * 3. Define a buffer for the converted PCM data.
 * 4. Start the I2S DMA transfer in circular mode:
 *    HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)pdm_buffer, PDM_BUFFER_SIZE_IN_HALFWORDS);
 * 5. In the DMA Half-Transfer Complete and Full-Transfer Complete callbacks:
 *    - Call the PDM_Filter function to convert PDM to PCM.
 *    - Process the PCM data (e.g., send it over Ethernet, analyze it).
 *    - Ensure proper buffer management to avoid data loss.
 */

static uint8_t Device_Connected(uint8_t idx)
{
  if (idx >= DEVICE_COUNT || !g_dev_present[idx]) { return 0u; }
  return ((uint32_t)(HAL_GetTick() - g_dev_last_tick[idx]) < DEVICE_CONNECT_TIMEOUT_MS) ? 1u : 0u;
}

/* ============ Dump one device's latest scan to the PC (USART2) ============ */
static void Device_SendLatest(uint8_t idx)
{
  char line[256];
  int  o = 0;
  uint8_t i;

  if (idx >= DEVICE_COUNT) { return; }

  osMutexAcquire(DeviceDataMutexHandle, osWaitForever);

  if (!g_dev_present[idx])
  {
    o += snprintf(line + o, (size_t)(sizeof(line) - o),
                  "DEV %u %s NOT-SEEN\r\n", (unsigned)g_dev[idx].id, g_dev[idx].name);
    HAL_UART_Transmit(&huart2, (uint8_t *)line, (uint16_t)o, 200);
    return;
  }

  o += snprintf(line + o, (size_t)(sizeof(line) - o),
                "DEV %u %s %s AGE=%lums T=%02X S=%u HEX=",
                (unsigned)g_dev[idx].id, g_dev[idx].name,
                Device_Connected(idx) ? "CONNECTED" : "TIMEOUT",
                (unsigned long)(HAL_GetTick() - g_dev_last_tick[idx]),
                g_dev_type[idx], (unsigned)g_dev_seq[idx]);
  for (i = 0; i < g_dev_len[idx]; i++)
  {
    o += snprintf(line + o, (size_t)(sizeof(line) - o), "%02X ", g_dev_payload[idx][i]);
  }
  o += snprintf(line + o, (size_t)(sizeof(line) - o), "\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t *)line, (uint16_t)o, 200);

  osMutexRelease(DeviceDataMutexHandle);
}

static void Device_SendAll(void)
{
  uint8_t i;
  for (i = 0; i < DEVICE_COUNT; i++) { Device_SendLatest(i); }
}

static void Uart_SendString(const char *s)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)s, (uint16_t)strlen(s), 200);
}

static void W5500_SendTestResult(void)
{
  char resp[80];
  uint8_t version = W5500_ReadByte(0x0039);
  int len = snprintf(resp, sizeof(resp), "W5500 %s CS=PC8 VERSIONR=0x%02X %s\r\n",
                     W5500_GetSpiBusName(), version,
                     (version == 0x04u) ? "OK" : "FAIL");
  HAL_UART_Transmit(&huart2, (uint8_t *)resp, (uint16_t)len, 200);
}

/* ============ Poll USART2 for a PC command and act on it ============
   Lines are terminated by CR or LF.
     LIST            -> list device ids + connection status
     GETALL (or GET) -> dump the latest scan of every device
     GET<2-digit id> -> dump the latest scan of that device  (e.g. GET55)
*/
void Uart_CheckCommand(void)
{
  uint8_t c;
  uint32_t now = HAL_GetTick();
  static uint32_t last_char = 0;

  while (HAL_UART_Receive(&huart2, &c, 1, 0) == HAL_OK)
  {
    if (c == '\r' || c == '\n')
    {
      if (g_cmd_n > 0)
      {
        g_cmd_line[g_cmd_n] = 0;                    /* NUL terminate */
        g_cmd_n = g_cmd_n > (sizeof(g_cmd_line) - 1) ? (uint8_t)(sizeof(g_cmd_line) - 1) : g_cmd_n;

        if (strncmp((char *)g_cmd_line, "LIST", 4) == 0)
        {
          uint8_t i;
          for (i = 0; i < DEVICE_COUNT; i++)
          {
            char resp[96]; int r = 0;
            r += snprintf(resp + r, (size_t)(sizeof(resp) - r),
                          "DEV %u %s %s\r\n",
                          (unsigned)g_dev[i].id, g_dev[i].name,
                          Device_Connected(i) ? "CONNECTED" : "NOT-SEEN");
            HAL_UART_Transmit(&huart2, (uint8_t *)resp, (uint16_t)r, 200);
          }
        }
        else if (strncmp((char *)g_cmd_line, "GETALL", 6) == 0)
        {
          Device_SendAll();
        }
        else if (strncmp((char *)g_cmd_line, "W5500SPI1", 9) == 0)
        {
          W5500_SetSpiBus(W5500_SPI_BUS_1);
          W5500_SendTestResult();
        }
        else if (strncmp((char *)g_cmd_line, "W5500SPI3", 9) == 0)
        {
          W5500_SetSpiBus(W5500_SPI_BUS_3);
          W5500_SendTestResult();
        }
        else if (strncmp((char *)g_cmd_line, "W5500TEST", 9) == 0)
        {
          W5500_SendTestResult();
        }
        else if (strncmp((char *)g_cmd_line, "GET", 3) == 0 && g_cmd_n == 5)
        {
          uint8_t id = (uint8_t)((g_cmd_line[3] - '0') * 10u + (g_cmd_line[4] - '0'));
          int8_t idx = Device_IndexOf(id);
          if (idx >= 0) { Device_SendLatest((uint8_t)idx); }
          else          { Uart_SendString("CMD: unknown device\r\n"); }
        }
        else
        {
          Uart_SendString("CMD: LIST | GETALL | GET<id> | W5500SPI1 | W5500SPI3 | W5500TEST\r\n");
        }
        g_cmd_n = 0;
      }
      last_char = now;
    }
    else if (g_cmd_n < sizeof(g_cmd_line) - 1)
    {
      g_cmd_line[g_cmd_n++] = c;
      last_char = now;
    }
  }

  /* Drop a partial (incomplete) command line after a short idle gap. */
  if (g_cmd_n > 0 && (now - last_char) > 50u) { g_cmd_n = 0; }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartMainEOTask */
/**
  * @brief  Function implementing the Task_MainEO thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMainEOTask */
void StartMainEOTask(void *argument)
{
  /* init code for USB_HOST */
  MX_USB_HOST_Init();
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartCC1101Task */
/**
* @brief Function implementing the Task_CC1101 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCC1101Task */
void StartCC1101Task(void *argument)
{
  /* USER CODE BEGIN StartCC1101Task */
  /* Infinite loop */
  for(;;) {
    uint8_t rx_bytes_reg = cc1101_read_reg(CC1101_RXBYTES);

    // Check for RX FIFO overflow
    if (rx_bytes_reg & 0x80) {
        cc1101_command_strobe(CC1101_SIDLE);
        cc1101_command_strobe(CC1101_SFRX);
        cc1101_command_strobe(CC1101_SRX);
    }
    // Check if there are any bytes in the RX FIFO
    else if (rx_bytes_reg & 0x7F) {
        uint8_t packet_len = cc1101_read_reg(CC1101_RXFIFO);

        if (packet_len > 0 && packet_len <= MAX_PAYLOAD_LEN) {
            RadioPacket_t new_packet;
            uint8_t status_bytes[2];

            // Read payload and then the 2 status bytes
            cc1101_read_burst(CC1101_RXFIFO, new_packet.payload, packet_len);
            cc1101_read_burst(CC1101_RXFIFO, status_bytes, 2);

            new_packet.len = packet_len;
            new_packet.rssi = status_bytes[0];
            new_packet.lqi = status_bytes[1] & 0x7F;

            // Check CRC status bit
            if (status_bytes[1] & 0x80) {
                // --- BEGIN ACK LOGIC ---
                if (packet_len >= 4) {
                    uint8_t dest_id = new_packet.payload[1]; // The original sender's ID
                    uint8_t packet_type = new_packet.payload[2];
                    uint8_t seq_num = new_packet.payload[3];

                    // Only ACK LOG or TELEM packets
                    if (packet_type == RADIO_PACKET_TYPE_LOG || packet_type == RADIO_PACKET_TYPE_TELEM) {
                        uint8_t ack_packet[4] = {dest_id, DEVICE_ID, RADIO_PACKET_TYPE_ACK, seq_num};
                        cc1101_send_packet(ack_packet, sizeof(ack_packet));
                    }
                }
                // --- END ACK LOGIC ---

                // Store the latest valid packet from this device
                Device_Store(new_packet.payload[1], new_packet.payload, new_packet.len);

                // Send the packet to the processing task
                osMessageQueuePut(RadioPacketQueueHandle, &new_packet, 0U, 0U);
            }
        }

        // Always flush and re-enter RX mode after a read to ensure clean state
        cc1101_command_strobe(CC1101_SIDLE);
        cc1101_command_strobe(CC1101_SFRX);
        cc1101_command_strobe(CC1101_SRX);
    }

    // This task polls the radio hardware. A small delay is appropriate.
    osDelay(10);
  }
  /* USER CODE END StartCC1101Task */
}

/* USER CODE BEGIN Header_StartPCCommTask */
/**
* @brief Function implementing the Task_PCComm thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartPCCommTask */
void StartPCCommTask(void *argument)
{
  /* USER CODE BEGIN StartPCCommTask */
  /* Infinite loop */
  for(;;) {
    RadioPacket_t rx_packet;
    osStatus_t status;

    // Check for incoming PC commands (non-blocking)
    Uart_CheckCommand();

    // Wait for a new radio packet from the queue (blocking with timeout)
    status = osMessageQueueGet(RadioPacketQueueHandle, &rx_packet, NULL, 10); // Wait 10ms
    if (status == osOK) {
        // A new packet was received, format and forward it to the PC
        char line_buf[256];
        int offset = 0;

        offset += snprintf(line_buf + offset, sizeof(line_buf) - offset, "Packet: ");

        for (uint8_t i = 0; i < rx_packet.len; i++) {
            offset += snprintf(line_buf + offset, sizeof(line_buf) - offset, "%02X ", rx_packet.payload[i]);
        }

        offset += snprintf(line_buf + offset, sizeof(line_buf) - offset, "| RSSI: %d, LQI: %d, CRC OK\r\n", rx_packet.rssi, rx_packet.lqi);
        
        // Forward to PC via UART2. Could also be sent via W5500 here.
        HAL_UART_Transmit(&huart2, (uint8_t*)line_buf, offset, 200);
    }
  }
  /* USER CODE END StartPCCommTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM5 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM5)
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
