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
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* Decoded LRR220PRO radar target (unit: cm for position, mm/s for speed). */
typedef struct
{
  uint8_t  valid;            /* 1 = object ID valid AND valid flag set      */
  uint8_t  object_id;        /* 0x00-0xFE valid, 0xFF invalid               */
  uint8_t  object_type;      /* 0 unk,1 4-wheel,2 2-wheel,3 pedestrian      */
  uint8_t  motion;           /* 0 unk,1 stationary,2 stopped,3 moving,4 cross */
  int16_t  x_pos_cm;         /* longitudinal position, cm                   */
  int16_t  y_pos_cm;         /* lateral position (signed), cm               */
  int16_t  x_vel_mmps;       /* longitudinal relative speed, mm/s           */
  int16_t  y_vel_mmps;       /* lateral relative speed, mm/s                */
  uint8_t  existence_percent;
  uint8_t  update_flag;      /* 0 = new, 1 = existed in previous cycle      */
  uint8_t  measure_flag;     /* 0 = live, 1 = predicted                     */
  uint32_t last_frame_tick;
} RadarTarget_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEVICE_ID              55u
#define RADIO_DEST_ID          11u
#define RADIO_PACKET_TYPE_TELEM 0x01u
#define RADIO_PACKET_TYPE_ACK   0x03u
#define RADIO_PACKET_TYPE_AUDIO 0x04u
#define RADIO_ACK_TIMEOUT_MS   100u
#define RADIO_MAX_RETRIES      3
#define RADIO_PACKET_TYPE_LOG   0x02u
#define RADIO_TX_INTERVAL_MS   3000u
#define STEPPER_TEST_STEPS     400
#define STEPPER_TEST_TIMEOUT_MS 10000u
#define I2S_TEST_WORDS         128u
#define AUDIO_NUM_SAMPLES      512
#define AUDIO_SAMPLE_RATE      8000
#define AUDIO_PACKET_PAYLOAD_SIZE 56 // 61 (max) - 5 (header)

#define I2S_SOUND_THRESHOLD    1000u

#define LOG_BOOT               0x01u
#define LOG_STEPPER_CW_DONE    0x04u
#define LOG_STEPPER_CCW_DONE   0x05u
#define LOG_STEPPER_TIMEOUT    0x06u
#define LOG_I2S_SOUND_OK       0x07u
#define LOG_I2S_SOUND_LOW      0x08u
#define LOG_I2S_READ_FAIL      0x09u
#define LOG_RADIO_TEST_OK      0x0Au

#define RADIO_PACKET_TYPE_RADAR 0x05u

/* ---- LRR220PRO radar over FDCAN ---- 500 kbps, Motorola byte order. */
#define RADAR_CAN_ID_STATUS     0x80u    /* FRS_Status                        */
#define RADAR_CAN_ID_OBJ_P1     0x081u   /* FRS_Obj_XX_Part1of2 (CONFIRM ME)  */
#define RADAR_CAN_ID_OBJ_P2     0x082u   /* FRS_Obj_XX_Part2of2 (CONFIRM ME)  */

/* Field scaling from the LRR220PRO protocol. */
#define RADAR_POS_FACTOR        0.015625f   /* m    */
#define RADAR_Y_OFFSET          64.0f        /* m    */
#define RADAR_VEL_FACTOR        0.1f         /* m/s  */
#define RADAR_VEL_OFFSET        102.4f       /* m/s  */
#define RADAR_EXST_FACTOR       1.5873f      /* %    */

/* One-shot radar link check window (ms) and its log codes. */
#define RADAR_CONNECT_TIMEOUT_MS  1000u
#define LOG_RADAR_CONNECT_OK      0x0Bu
#define LOG_RADAR_CONNECT_FAIL    0x0Cu

#define CC1101_IOCFG0          0x02u
#define CC1101_FIFOTHR         0x03u
#define CC1101_PKTLEN          0x06u
#define CC1101_PKTCTRL1        0x07u
#define CC1101_PKTCTRL0        0x08u
#define CC1101_ADDR            0x09u
#define CC1101_CHANNR          0x0Au
#define CC1101_FSCTRL1         0x0Bu
#define CC1101_FREQ2           0x0Du
#define CC1101_FREQ1           0x0Eu
#define CC1101_FREQ0           0x0Fu
#define CC1101_MDMCFG4         0x10u
#define CC1101_MDMCFG3         0x11u
#define CC1101_MDMCFG2         0x12u
#define CC1101_MDMCFG1         0x13u
#define CC1101_MDMCFG0         0x14u
#define CC1101_DEVIATN         0x15u
#define CC1101_MCSM0           0x18u
#define CC1101_FOCCFG          0x19u
#define CC1101_BSCFG           0x1Au
#define CC1101_AGCCTRL2        0x1Bu
#define CC1101_AGCCTRL1        0x1Cu
#define CC1101_AGCCTRL0        0x1Du
#define CC1101_FREND1          0x21u
#define CC1101_FREND0          0x22u
#define CC1101_FSCAL3          0x23u
#define CC1101_FSCAL2          0x24u
#define CC1101_FSCAL1          0x25u
#define CC1101_FSCAL0          0x26u
#define CC1101_TEST2           0x2Cu
#define CC1101_TEST1           0x2Du
#define CC1101_TEST0           0x2Eu
#define CC1101_PATABLE         0x3Eu
#define CC1101_TXFIFO          0x3Fu

// Command Strobes
#define CC1101_SRES            0x30u
#define CC1101_SRX             0x34u
#define CC1101_STX             0x35u
#define CC1101_SIDLE           0x36u
#define CC1101_SFRX            0x3Au
#define CC1101_SFTX            0x3Bu
#define CC1101_WRITE_BURST     0x40u
#define CC1101_READ_SINGLE     0x80u
#define CC1101_READ_BURST      0xC0u

// Status Registers
#define CC1101_PARTNUM         0x30u       // Part number
#define CC1101_VERSION         0x31u       // Current version number
#define CC1101_MARCSTATE       0x35u       // Main Radio Control FSM state
#define CC1101_RXBYTES         0x3Bu       // Underflow and number of bytes in RXFIFO
#define CC1101_RXFIFO          0x3Fu

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

FDCAN_HandleTypeDef hfdcan1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* Latest radar state, shared between the FDCAN ISR and the main loop. */
static volatile RadarTarget_t  g_radar_target;
static volatile uint8_t        g_radar_online;   /* 1 = status frame received   */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_FDCAN1_Init(void);
/* USER CODE BEGIN PFP */
static void boot(void);

/* ---- CC1101 low-level ---- */
static uint8_t CC1101_WaitForReady(void);
static void CC1101_Strobe(uint8_t strobe);
static uint8_t CC1101_ReadReg(uint8_t addr);
static void CC1101_WriteReg(uint8_t addr, uint8_t value);
static void CC1101_WriteBurst(uint8_t addr, const uint8_t *data, uint8_t len);
static void CC1101_ReadBurst(uint8_t addr, uint8_t *buffer, uint8_t len);
static void CC1101_InitRadio(void);
static void Test_CC1101_Connection(void);
static void Radio_TransmitPacket(const uint8_t *payload, uint8_t len);
static uint8_t Radio_SendReliablePacket(const uint8_t *payload, uint8_t len);
static void PutU16(uint8_t *buf, uint8_t *idx, uint16_t value);
static void PutU32(uint8_t *buf, uint8_t *idx, uint32_t value);
static void Radio_SendLog(uint8_t code, int32_t value);

/* ---- Radar (FDCAN) ---- */
static uint32_t Radar_GetField(const uint8_t *d, uint8_t start_bit, uint8_t len);
static void Radar_DecodePart1(const FDCAN_RxHeaderTypeDef *h, const uint8_t *d);
static void Radar_DecodePart2(const FDCAN_RxHeaderTypeDef *h, const uint8_t *d);
static void Radar_DecodeStatus(const uint8_t *d);
static void Radar_FDCAN_Start(void);
static void Radar_CheckConnection(void);
static void Radio_SendRadarTelemetry(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE BEGIN 0 */

/* Called right after GPIO init by the generated MX_GPIO_Init(). */
static void DisplayNibble(uint8_t n) {
    HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, (GPIO_PinState)(n & 1));
    HAL_GPIO_WritePin(GPIOA, led2_Pin, (GPIO_PinState)((n >> 1) & 1));
    HAL_GPIO_WritePin(GPIOA, led3_Pin, (GPIO_PinState)((n >> 2) & 1));
    HAL_GPIO_WritePin(GPIOA, led4_Pin, (GPIO_PinState)((n >> 3) & 1));
}
static void boot(void)
{
	const uint8_t s[] = "\x6F\x6D\x20\x6E\x61\x6D\x61\x68\x20\x73\x68\x69\x76\x61\x79";
	for (int i = 0; i < 45; i++) {
		uint8_t m = i % 3, c = s[i / 3];
		DisplayNibble(m == 2 ? 0 : (m ? c & 15 : c >> 4));
		HAL_Delay(m == 2 ? 20 : 100);
	}
	DisplayNibble(0);
	HAL_Delay(500);
}

/* ======================= CC1101 low-level (SPI2 = PA0/PA3/PA4, CS = PA2) ======================= */
static uint8_t CC1101_WaitForReady(void)
{
  uint32_t t = HAL_GetTick();
  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET)  /* MISO low = ready */
  {
    if ((HAL_GetTick() - t) > 10u) { return 0u; }
  }
  return 1u;
}

static void CC1101_Strobe(uint8_t strobe)
{
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
  if (CC1101_WaitForReady() == 0u)
  {
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
    return;
  }
  HAL_SPI_Transmit(&hspi2, &strobe, 1, 100);
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

static uint8_t CC1101_ReadReg(uint8_t addr)
{
  uint8_t tx[2] = { addr | CC1101_READ_SINGLE, 0x00u };
  uint8_t rx[2] = { 0u, 0u };
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
  if (CC1101_WaitForReady() == 0u)
  {
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
    return 0xFFu;
  }
  HAL_SPI_TransmitReceive(&hspi2, tx, rx, 2, 100);
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
  return rx[1];
}

static void CC1101_WriteReg(uint8_t addr, uint8_t value)
{
  uint8_t tx[2] = { addr, value };
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
  if (CC1101_WaitForReady() == 0u)
  {
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
    return;
  }
  HAL_SPI_Transmit(&hspi2, tx, 2, 100);
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

static void CC1101_WriteBurst(uint8_t addr, const uint8_t *data, uint8_t len)
{
  uint8_t hdr = addr | CC1101_WRITE_BURST;
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
  if (CC1101_WaitForReady() == 0u)
  {
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
    return;
  }
  HAL_SPI_Transmit(&hspi2, &hdr, 1, 100);
  HAL_SPI_Transmit(&hspi2, (uint8_t *)data, len, 100);
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

static void CC1101_ReadBurst(uint8_t addr, uint8_t *buffer, uint8_t len)
{
  uint8_t hdr = addr | CC1101_READ_BURST;
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
  if (CC1101_WaitForReady() == 0u)
  {
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
    return;
  }
  HAL_SPI_Transmit(&hspi2, &hdr, 1, 100);
  HAL_SPI_Receive(&hspi2, buffer, len, 100);
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

/* 433.92 MHz GFSK radio config; MUST be identical on the destination node (ID 11). */
static void CC1101_InitRadio(void)
{
  static const uint8_t pa[] = { 0xC0u };

  CC1101_Strobe(CC1101_SRES);
  CC1101_Strobe(CC1101_SIDLE);
  CC1101_WriteReg(CC1101_FSCTRL1, 0x06u);
  CC1101_WriteReg(CC1101_FREQ2, 0x10u);
  CC1101_WriteReg(CC1101_FREQ1, 0xB0u);
  CC1101_WriteReg(CC1101_FREQ0, 0x71u);
  CC1101_WriteReg(CC1101_MDMCFG4, 0xCAu);
  CC1101_WriteReg(CC1101_MDMCFG3, 0x83u);
  CC1101_WriteReg(CC1101_MDMCFG2, 0x13u);
  CC1101_WriteReg(CC1101_MDMCFG1, 0x22u);
  CC1101_WriteReg(CC1101_MDMCFG0, 0xF8u);
  CC1101_WriteReg(CC1101_CHANNR, 0x00u);
  CC1101_WriteReg(CC1101_DEVIATN, 0x35u);
  CC1101_WriteReg(CC1101_FREND1, 0x56u);
  CC1101_WriteReg(CC1101_FREND0, 0x10u);
  CC1101_WriteReg(CC1101_MCSM0, 0x18u);
  CC1101_WriteReg(CC1101_FOCCFG, 0x16u);
  CC1101_WriteReg(CC1101_BSCFG, 0x6Cu);
  CC1101_WriteReg(CC1101_AGCCTRL2, 0x43u);
  CC1101_WriteReg(CC1101_AGCCTRL1, 0x40u);
  CC1101_WriteReg(CC1101_AGCCTRL0, 0x91u);
  CC1101_WriteReg(CC1101_FSCAL3, 0xE9u);
  CC1101_WriteReg(CC1101_FSCAL2, 0x2Au);
  CC1101_WriteReg(CC1101_FSCAL1, 0x00u);
  CC1101_WriteReg(CC1101_FSCAL0, 0x1Fu);
  CC1101_WriteReg(CC1101_TEST2, 0x81u);
  CC1101_WriteReg(CC1101_TEST1, 0x35u);
  CC1101_WriteReg(CC1101_TEST0, 0x09u);
  CC1101_WriteReg(CC1101_PKTCTRL1, 0x04u);   /* CRC auto-flush, no address check */
  CC1101_WriteReg(CC1101_PKTCTRL0, 0x05u);   /* variable length, CRC enabled     */
  CC1101_WriteReg(CC1101_ADDR,     0x00u);
  CC1101_WriteReg(CC1101_PKTLEN,   0xFFu);
  CC1101_WriteBurst(CC1101_PATABLE, pa, sizeof(pa));
}

static void Test_CC1101_Connection(void)
{
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
}

/* -------- Frame build helpers (little-endian, matching the GUI/receiver) -------- */
static void PutU16(uint8_t *buf, uint8_t *idx, uint16_t value)
{
  buf[(*idx)++] = (uint8_t)(value);
  buf[(*idx)++] = (uint8_t)(value >> 8);
}

static void PutU32(uint8_t *buf, uint8_t *idx, uint32_t value)
{
  buf[(*idx)++] = (uint8_t)(value);
  buf[(*idx)++] = (uint8_t)(value >> 8);
  buf[(*idx)++] = (uint8_t)(value >> 16);
  buf[(*idx)++] = (uint8_t)(value >> 24);
}

/* Fire-and-forget CC1101 frame: [len][payload...]. */
static void Radio_TransmitPacket(const uint8_t *payload, uint8_t len)
{
  uint8_t fifo[62];
  uint32_t start;

  if (len == 0u || len > 61u) { return; }
  fifo[0] = len;
  memcpy(&fifo[1], payload, len);

  CC1101_Strobe(CC1101_SIDLE);
  CC1101_Strobe(CC1101_SFTX);
  CC1101_WriteBurst(CC1101_TXFIFO, fifo, (uint8_t)(len + 1u));
  CC1101_Strobe(CC1101_STX);

  start = HAL_GetTick();
  while (CC1101_ReadReg(CC1101_MARCSTATE) != 0x01u)   /* MARCSTATE IDLE = sent */
  {
    if ((HAL_GetTick() - start) > 50u) { break; }
  }
}

/*
 * Payload header used by all transmitters:
 *   payload[0] = DEST   (this node sends to RADIO_DEST_ID = 11)
 *   payload[1] = SRC    (this node = DEVICE_ID = 55)
 *   payload[2] = TYPE   (0x01 telem, 0x02 log, 0x03 ack, 0x05 radar)
 *   payload[3] = SEQ
 * A matching ACK is: [DEST][SRC][TYPE_ACK][SEQ] (4 bytes).
 */
static uint8_t Radio_SendReliablePacket(const uint8_t *payload, uint8_t len)
{
  uint8_t retries = RADIO_MAX_RETRIES;

  while (retries--)
  {
    Radio_TransmitPacket(payload, len);

    CC1101_Strobe(CC1101_SRX);   /* wait for an ACK from the receiver */
    {
      uint32_t t0 = HAL_GetTick();
      while ((HAL_GetTick() - t0) < RADIO_ACK_TIMEOUT_MS)
      {
        if (CC1101_ReadReg(CC1101_RXBYTES) & 0x7Fu)
        {
          uint8_t rxb[64];
          uint8_t plen = CC1101_ReadReg(CC1101_RXFIFO);
          if (plen > 0u && plen <= 61u)
          {
            CC1101_ReadBurst(CC1101_RXFIFO, rxb, (uint8_t)(plen + 2u));
            if ((rxb[plen + 1u] & 0x80u) &&              /* CRC OK                */
                plen == 4u &&
                rxb[0] == DEVICE_ID &&                   /* destined for us       */
                rxb[1] == payload[0] &&                  /* from the receiver     */
                rxb[2] == RADIO_PACKET_TYPE_ACK &&       /* is an ACK             */
                rxb[3] == payload[3])                    /* matches sequence      */
            {
              CC1101_Strobe(CC1101_SIDLE);
              CC1101_Strobe(CC1101_SFRX);
              return 1u;
            }
          }
          CC1101_Strobe(CC1101_SIDLE);
          CC1101_Strobe(CC1101_SFRX);
          CC1101_Strobe(CC1101_SRX);
        }
      }
    }
  }
  CC1101_Strobe(CC1101_SIDLE);
  return 0u;
}

static void Radio_SendLog(uint8_t code, int32_t value)
{
  static uint8_t log_seq = 0u;
  uint8_t packet[13];
  uint8_t i = 0u;

  packet[i++] = RADIO_DEST_ID;
  packet[i++] = DEVICE_ID;
  packet[i++] = RADIO_PACKET_TYPE_LOG;
  packet[i++] = log_seq++;
  packet[i++] = code;
  PutU32(packet, &i, HAL_GetTick());
  PutU32(packet, &i, (uint32_t)value);
  (void)Radio_SendReliablePacket(packet, i);
}

/* ======================= LRR220PRO radar decode (FDCAN) ======================= */
/*
 * Extract 'len' bits starting at 'start_bit' from an 8-byte payload,
 * interpreting the payload as a little-endian 64-bit word.
 * NOTE: verify the exact start_bit/length against a real capture (the protocol
 *       defines a Motorola bit order that must be validated frame-by-frame).
 */
static uint32_t Radar_GetField(const uint8_t *d, uint8_t start_bit, uint8_t len)
{
  uint64_t v = 0u;
  uint8_t i;
  for (i = 0u; i < 8u; i++) { v |= ((uint64_t)d[i]) << (8u * i); }
  if ((start_bit + len) > 64u) { len = (uint8_t)(64u - start_bit); }
  if (len >= 32u) { return (uint32_t)(v >> start_bit); }
  return (uint32_t)((v >> start_bit) & ((1u << len) - 1u));
}

static void Radar_DecodePart1(const FDCAN_RxHeaderTypeDef *h, const uint8_t *d)
{
  RadarTarget_t t;
  uint32_t raw;
  (void)h;

  memset(&t, 0, sizeof(t));
  t.object_id    = (uint8_t)Radar_GetField(d,  0, 8);   /* Obj_ID               */
  t.update_flag  = (uint8_t)Radar_GetField(d, 24, 1);   /* Obj_UpdateFlag       */

  raw = Radar_GetField(d, 34, 14);                      /* Obj_Xpos             */
  t.x_pos_cm = (int16_t)((float)raw * RADAR_POS_FACTOR * 100.0f);

  raw = Radar_GetField(d, 16, 13);                      /* Obj_YPos             */
  t.y_pos_cm = (int16_t)(((float)raw * RADAR_POS_FACTOR - RADAR_Y_OFFSET) * 100.0f);

  raw = Radar_GetField(d, 13, 11);                      /* Obj_XVelRel          */
  t.x_vel_mmps = (int16_t)(((float)raw * RADAR_VEL_FACTOR - RADAR_VEL_OFFSET) * 1000.0f);

  raw = Radar_GetField(d, 53, 11);                      /* Obj_YVelRel          */
  t.y_vel_mmps = (int16_t)(((float)raw * RADAR_VEL_FACTOR - RADAR_VEL_OFFSET) * 1000.0f);

  t.motion      = (uint8_t)Radar_GetField(d,  8, 3);    /* Obj_MotionPattern    */
  t.object_type = (uint8_t)Radar_GetField(d, 11, 3);    /* Obj_Type             */

  t.existence_percent = (uint8_t)((uint32_t)Radar_GetField(d, 52, 6) * RADAR_EXST_FACTOR);
  if (t.existence_percent > 100u) { t.existence_percent = 100u; }

  /* validity: Obj_ID != 0xFF AND Obj_ValidFlag set */
  t.valid = ((t.object_id != 0xFFu) && (Radar_GetField(d, 23, 1) != 0u)) ? 1u : 0u;
  t.last_frame_tick = HAL_GetTick();

  __disable_irq();
  g_radar_target = t;
  __enable_irq();
}

static void Radar_DecodePart2(const FDCAN_RxHeaderTypeDef *h, const uint8_t *d)
{
  /* Part-2 only documents the alive counter / checksum in the supplied pages;
     nothing more to store here until the complete frame table is available. */
  (void)h; (void)d;
}

static void Radar_DecodeStatus(const uint8_t *d)
{
  (void)d;                       /* a status frame was seen -> radar online */
  g_radar_online = 1u;
}

/* Weak HAL callback, reached via FDCAN1_IT0_IRQHandler -> HAL_FDCAN_IRQHandler. */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  FDCAN_RxHeaderTypeDef hdr;
  uint8_t data[8];

  if (hfdcan != &hfdcan1) { return; }
  (void)RxFifo0ITs;
  if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &hdr, data) != HAL_OK) { return; }

  if      (hdr.Identifier == RADAR_CAN_ID_STATUS)     { Radar_DecodeStatus(data); }
  else if (hdr.Identifier == RADAR_CAN_ID_OBJ_P1)     { Radar_DecodePart1(&hdr, data); }
  else if (hdr.Identifier == RADAR_CAN_ID_OBJ_P2)     { Radar_DecodePart2(&hdr, data); }
}

static void Radar_FDCAN_Start(void)
{
  FDCAN_FilterTypeDef f = {0};

  /* Accept all standard frames, then dispatch by ID inside the callback. */
  f.IdType       = FDCAN_STANDARD_ID;
  f.FilterIndex  = 0;
  f.FilterType   = FDCAN_FILTER_MASK;
  f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  f.FilterID1    = 0x000u;    /* mask = 0 -> every standard ID passes */
  f.FilterID2    = 0x000u;
  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &f) != HAL_OK) { Error_Handler(); }

  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) { Error_Handler(); }
  (void)HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0u);
}

/*
 * Radar telemetry frame sent to node ID 11:
 *   [0]  DEST=11   [1] SRC=55   [2] TYPE=0x05   [3] SEQ
 *   [4]  sub = 0x01 (decoded target)
 *   [5]  obj_id (0xFF if none)
 *   [6]  bits: bit7 = radar online, bits4..6 = object_type,
 *              bits1..3 = motion, bit0 = valid
 *   [7..8]    x_pos_cm      (little-endian)
 *   [9..10]   y_pos_cm
 *   [11..12]  x_vel_mmps
 *   [13..14]  y_vel_mmps
 *   [15]  existence_percent
 */
static void Radio_SendRadarTelemetry(void)
{
  static uint8_t seq = 0u;
  uint8_t packet[16];
  uint8_t i = 0u;
  RadarTarget_t t;

  __disable_irq();
  t = (RadarTarget_t)g_radar_target;
  __enable_irq();

  packet[i++] = RADIO_DEST_ID;
  packet[i++] = DEVICE_ID;
  packet[i++] = RADIO_PACKET_TYPE_RADAR;
  packet[i++] = seq++;
  packet[i++] = 0x01u;                                  /* sub: decoded target  */
  packet[i++] = t.valid ? t.object_id : 0xFFu;
  packet[i++] = (uint8_t)((t.valid ? 1u : 0u) | (t.motion << 1) | (t.object_type << 4) |
                          (g_radar_online ? 0x80u : 0u));
  PutU16(packet, &i, (uint16_t)t.x_pos_cm);
  PutU16(packet, &i, (uint16_t)t.y_pos_cm);
  PutU16(packet, &i, (uint16_t)t.x_vel_mmps);
  PutU16(packet, &i, (uint16_t)t.y_vel_mmps);
  packet[i++] = t.existence_percent;

  (void)Radio_SendReliablePacket(packet, i);
}

/*
 * One-shot radar connection check.  Waits a short window for FDCAN traffic
 * (FRS_Status frame -> g_radar_online set, or any object frame -> last_frame_tick
 * advances), then reports the result on the LEDs and over the radio log:
 *   - connected : LED2 on, LED1 off, LOG_RADAR_CONNECT_OK
 *   - no traffic : LED1 on, LED2 off, LOG_RADAR_CONNECT_FAIL
 */
static void Radar_CheckConnection(void)
{
  uint32_t t0 = HAL_GetTick();
  uint32_t last_target_tick;
  uint8_t  saw_traffic = 0u;

  __disable_irq();
  last_target_tick = g_radar_target.last_frame_tick;
  __enable_irq();

  while ((HAL_GetTick() - t0) < RADAR_CONNECT_TIMEOUT_MS)
  {
    if (g_radar_online)
    {
      saw_traffic = 1u;
      break;
    }
    __disable_irq();
    if (g_radar_target.last_frame_tick != last_target_tick) { saw_traffic = 1u; }
    __enable_irq();
    if (saw_traffic) { break; }
    HAL_Delay(10);
  }

  if (saw_traffic)
  {
    HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_RESET);
    Radio_SendLog(LOG_RADAR_CONNECT_OK, (int32_t)(t0 / 1000u));
  }
  else
  {
    HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_SET);
    Radio_SendLog(LOG_RADAR_CONNECT_FAIL, 0);
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
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_FDCAN1_Init();
  /* USER CODE BEGIN 2 */
  Test_CC1101_Connection();
  CC1101_InitRadio();
  Radio_SendLog(LOG_BOOT, DEVICE_ID);

  /* Start listening to the LRR220PRO radar on FDCAN1 and decode it. */
  Radar_FDCAN_Start();

  /* Quick one-shot radar connection check (LED + radio log). */
  Radar_CheckConnection();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    Radio_SendRadarTelemetry();
    HAL_Delay(RADIO_TX_INTERVAL_MS);
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

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  /* LRR220PRO radar runs at 500 kbps.  FDCAN kernel clock = APB1 = 48 MHz,
     so tq_total = 1 + (TS1+1) + (TS2+1) = 12 tq, prescaler = 8
     -> 48e6 / (8 * 12) = 500 kbps. */
  hfdcan1.Init.NominalPrescaler = 8;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 8;
  hfdcan1.Init.NominalTimeSeg2 = 1;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

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
  hi2c1.Init.Timing = 0x10805D88;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

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
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, c1101_cs_Pin|A4988_dir_Pin|A4988_en_Pin|led2_Pin
                          |led3_Pin|led4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : c1101_cs_Pin A4988_dir_Pin A4988_en_Pin led2_Pin
                           led3_Pin led4_Pin */
  GPIO_InitStruct.Pin = c1101_cs_Pin|A4988_dir_Pin|A4988_en_Pin|led2_Pin
                          |led3_Pin|led4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : led1_Pin */
  GPIO_InitStruct.Pin = led1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(led1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  boot();
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
  void assert_failed(uint8_t *file, uint32_t line)
  {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\\r\\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
