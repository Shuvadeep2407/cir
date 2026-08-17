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
#include "i2s.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "camera.h"
#include <string.h>
#include "gy271.h"
#include "stepper.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEVICE_ID              22u
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

/* USER CODE END PD */
// Status Registers (continued)
#define CC1101_RXBYTES         0x3Bu       // Underflow and number of bytes in RXFIFO
#define CC1101_RXFIFO          0x3Fu


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
static uint8_t CC1101_WaitForReady(void);
static void CC1101_Strobe(uint8_t strobe);
static uint8_t CC1101_ReadReg(uint8_t addr);
static void CC1101_WriteReg(uint8_t addr, uint8_t value);
static void CC1101_WriteBurst(uint8_t addr, const uint8_t *data, uint8_t len);
static void CC1101_ReadBurst(uint8_t addr, uint8_t *buffer, uint8_t len);
static void CC1101_InitRadio(void);
static uint8_t Radio_SendReliablePacket(const uint8_t *payload, uint8_t len);
static void Radio_SendAudioChunk(void);
static void Radio_SendTelemetry(void);
static void Radio_SendLog(uint8_t code, int32_t value);
static void Startup_TestStepper(void);
static void Startup_TestI2S(void);
static void Test_CC1101_Connection(void);
static void Startup_TestRadioPacket(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief Displays a 4-bit value (nibble) on the 4 LEDs.
  * @param nibble The 4-bit value to display.
  */
static void DisplayNibble(uint8_t n) {
    HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, (GPIO_PinState)(n & 1));
    HAL_GPIO_WritePin(GPIOA, led2_Pin, (GPIO_PinState)((n >> 1) & 1));
    HAL_GPIO_WritePin(GPIOA, led3_Pin, (GPIO_PinState)((n >> 2) & 1));
    HAL_GPIO_WritePin(GPIOA, led4_Pin, (GPIO_PinState)((n >> 3) & 1));
}

static void boot(void) {
    const uint8_t s[] = "\x6F\x6D\x20\x6E\x61\x6D\x61\x68\x20\x73\x68\x69\x76\x61\x79";
    for (int i = 0; i < 45; i++) {
        uint8_t m = i % 3, c = s[i / 3];
        DisplayNibble(m == 2 ? 0 : (m ? c & 15 : c >> 4));
        HAL_Delay(m == 2 ? 20 : 100);
    }
    DisplayNibble(0);
    HAL_Delay(500);
}

/**
 * @brief Waits for the CC1101 MISO line to go low, indicating the chip is ready.
 * @retval 1 if ready, 0 on timeout.
 */
static uint8_t CC1101_WaitForReady(void)
{
    uint32_t tickstart = HAL_GetTick();
    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET)
    {
        if ((HAL_GetTick() - tickstart) > 10) // 10ms timeout
        {
            return 0; // Timeout
        }
    }
    return 1; // Ready
}

/**
  * @brief Sends a command strobe to the CC1101.
  * @param strobe The command strobe to send (e.g., 0x30 for SRES).
  */
static void CC1101_Strobe(uint8_t strobe)
{
    // Assert CS
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);

    if (CC1101_WaitForReady() == 0) {
        HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
        return;
    }

    // Transmit strobe command
    HAL_SPI_Transmit(&hspi2, &strobe, 1, 100);

    // De-assert CS
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

static void CC1101_WriteReg(uint8_t addr, uint8_t value)
{
    uint8_t tx[2] = { addr, value };

    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
    if (CC1101_WaitForReady() == 0) {
        HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
        return;
    }
    HAL_SPI_Transmit(&hspi2, tx, 2, 100);
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

static void CC1101_WriteBurst(uint8_t addr, const uint8_t *data, uint8_t len)
{
    uint8_t header = addr | CC1101_WRITE_BURST;

    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
    if (CC1101_WaitForReady() == 0) {
        HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
        return;
    }
    HAL_SPI_Transmit(&hspi2, &header, 1, 100);
    HAL_SPI_Transmit(&hspi2, (uint8_t *)data, len, 100);
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

static void CC1101_ReadBurst(uint8_t addr, uint8_t *buffer, uint8_t len)
{
    uint8_t header = addr | CC1101_READ_BURST;

    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
    if (CC1101_WaitForReady() == 0) {
        HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
        return;
    }
    HAL_SPI_Transmit(&hspi2, &header, 1, 100);
    HAL_SPI_Receive(&hspi2, buffer, len, 100);
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}


/**
  * @brief Reads a single register from the CC1101.
  * @param addr The register address (0x00 - 0x3D).
  * @retval The register value.
  */
static uint8_t CC1101_ReadReg(uint8_t addr)
{
    uint8_t tx_data[2] = { addr | CC1101_READ_SINGLE, 0x00 }; // Header, dummy byte
    uint8_t rx_data[2] = { 0 };

    // Assert CS
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);

    if (CC1101_WaitForReady() == 0) {
        HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
        return 0xFF; // Indicate timeout error
    }

    // Transmit header and receive status byte, then transmit dummy and receive register value
    HAL_SPI_TransmitReceive(&hspi2, tx_data, rx_data, 2, 100);

    // De-assert CS
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);

    return rx_data[1]; // Return the register value
}

static void CC1101_InitRadio(void)
{
    static const uint8_t pa_table[] = { 0xC0u }; /* approx. 10 dBm on common CC1101 modules */

    CC1101_Strobe(CC1101_SIDLE);
    CC1101_Strobe(CC1101_SFTX);

    /* 433.92 MHz, GFSK. These settings must match the receiver. */
    // NOTE: The original comment for baud rate was incorrect. The register values below
    // configure the radio for ~153.5 kBaud.
    CC1101_WriteReg(CC1101_FSCTRL1,  0x06u); // IF Freq: 152.34375 kHz
    CC1101_WriteReg(CC1101_FREQ2,    0x10u); // Freq. Control Word, High Byte
    CC1101_WriteReg(CC1101_FREQ1,    0xB0u); // Freq. Control Word, Middle Byte
    CC1101_WriteReg(CC1101_FREQ0,    0x71u); // Freq. Control Word, Low Byte
    CC1101_WriteReg(CC1101_MDMCFG4,  0xCAu); // RX BW: 101.5625 kHz, Data Rate Exponent
    CC1101_WriteReg(CC1101_MDMCFG3,  0x83u); // Data Rate Mantissa (~153.5 kBaud)
    CC1101_WriteReg(CC1101_MDMCFG2,  0x13u); // GFSK, 30/32 sync word bits
    CC1101_WriteReg(CC1101_MDMCFG1,  0x22u); // 4-byte preamble, FEC disabled
    CC1101_WriteReg(CC1101_MDMCFG0,  0xF8u); // Channel spacing: 199.951172 kHz
    CC1101_WriteReg(CC1101_CHANNR,   0x00u); // Channel 0
    CC1101_WriteReg(CC1101_DEVIATN,  0x35u); // Deviation: 19.042969 kHz
    CC1101_WriteReg(CC1101_FREND1,   0x56u); // Front end RX configuration
    CC1101_WriteReg(CC1101_FREND0,   0x10u); // Front end TX configuration
    CC1101_WriteReg(CC1101_MCSM0,    0x18u); // Auto-calibrate on IDLE->RX/TX
    CC1101_WriteReg(CC1101_FOCCFG,   0x16u); // Freq Offset Comp. config
    CC1101_WriteReg(CC1101_BSCFG,    0x6Cu); // Bit Synchronization config
    CC1101_WriteReg(CC1101_AGCCTRL2, 0x43u); // AGC Control
    CC1101_WriteReg(CC1101_AGCCTRL1, 0x40u); // AGC Control
    CC1101_WriteReg(CC1101_AGCCTRL0, 0x91u); // AGC Control
    CC1101_WriteReg(CC1101_FSCAL3,   0xE9u); // Freq. Synthesizer Cal.
    CC1101_WriteReg(CC1101_FSCAL2,   0x2Au); // Freq. Synthesizer Cal.
    CC1101_WriteReg(CC1101_FSCAL1,   0x00u); // Freq. Synthesizer Cal.
    CC1101_WriteReg(CC1101_FSCAL0,   0x1Fu); // Freq. Synthesizer Cal.
    CC1101_WriteReg(CC1101_TEST2,    0x81u); // Required for sensitivity at low data rates
    CC1101_WriteReg(CC1101_TEST1,    0x35u); // Required for sensitivity at low data rates
    CC1101_WriteReg(CC1101_TEST0,    0x09u); // Required for sensitivity at low data rates
    CC1101_WriteReg(CC1101_PKTCTRL1, 0x04u); // No address check, CRC autoflush on
    CC1101_WriteReg(CC1101_PKTCTRL0, 0x05u); // Variable packet length, CRC enabled
    CC1101_WriteReg(CC1101_ADDR,     0x00u); // Device address (not used, PKTCTRL1)
    CC1101_WriteReg(CC1101_PKTLEN,   0xFFu); // Max packet length
    
    CC1101_WriteBurst(CC1101_PATABLE, pa_table, sizeof(pa_table));
}

static void Radio_TransmitPacket(const uint8_t *payload, uint8_t len)
{
    // This is a low-level, fire-and-forget transmit function.
    uint8_t fifo[62];
    if (len == 0u || len > 61u) { return; }

    fifo[0] = len;
    memcpy(&fifo[1], payload, len);

    CC1101_Strobe(CC1101_SIDLE);
    CC1101_Strobe(CC1101_SFTX);
    CC1101_WriteBurst(CC1101_TXFIFO, fifo, len + 1u);
    CC1101_Strobe(CC1101_STX);

    // Wait for transmission to complete. MARCSTATE will go to IDLE (0x01).
    // Add a timeout to prevent getting stuck.
    uint32_t start_tick = HAL_GetTick();
    while (CC1101_ReadReg(CC1101_MARCSTATE) != 0x01) {
        if (HAL_GetTick() - start_tick > 50) { break; } // 50ms timeout
    }
}

static uint8_t Radio_SendReliablePacket(const uint8_t *payload, uint8_t len)
{
    uint8_t retries = RADIO_MAX_RETRIES;
    while (retries--)
    {
        Radio_TransmitPacket(payload, len);

        // Switch to RX mode to wait for ACK
        CC1101_Strobe(CC1101_SRX);

        uint32_t ack_wait_start = HAL_GetTick();
        while (HAL_GetTick() - ack_wait_start < RADIO_ACK_TIMEOUT_MS)
        {
            // Check if a packet has been received
            if (CC1101_ReadReg(CC1101_RXBYTES) & 0x7F)
            {
                uint8_t rx_buffer[64];
                uint8_t packet_len = CC1101_ReadReg(CC1101_RXFIFO);

                if (packet_len > 0 && packet_len <= 61)
                {
                    // Read packet + 2 status bytes (RSSI, LQI/CRC)
                    CC1101_ReadBurst(CC1101_RXFIFO, rx_buffer, packet_len + 2);

                    // Check CRC status bit
                    if (rx_buffer[packet_len + 1] & 0x80)
                    {
                        // Check if it's an ACK for our packet
                        // ACK format: [DEST_ID, SRC_ID, TYPE_ACK, SEQ_NUM]
                        // Original packet: [DEST_ID, SRC_ID, TYPE, SEQ_NUM, ...]
                        const uint8_t expected_src_id = payload[0]; // The original destination
                        const uint8_t expected_seq = payload[3];    // The original sequence number

                        if (packet_len == 4 &&
                            rx_buffer[0] == DEVICE_ID &&          // Destined for us
                            rx_buffer[1] == expected_src_id &&    // From the intended receiver
                            rx_buffer[2] == RADIO_PACKET_TYPE_ACK && // Is an ACK packet
                            rx_buffer[3] == expected_seq)         // Matches sequence number
                        {
                            CC1101_Strobe(CC1101_SIDLE);
                            CC1101_Strobe(CC1101_SFRX); // Flush RX FIFO
                            return 1; // ACK received, success!
                        }
                    }
                }
                // If not a valid ACK, or bad packet, flush and restart RX
                CC1101_Strobe(CC1101_SIDLE);
                CC1101_Strobe(CC1101_SFRX);
                CC1101_Strobe(CC1101_SRX);
            }
        }
        // ACK not received within timeout, will retry...
    }

    CC1101_Strobe(CC1101_SIDLE); // Go to IDLE after all retries fail
    return 0u; // Return 0 to indicate failure after all retries.
}

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

static void Radio_SendTelemetry(void)
{
    static uint8_t seq = 0u;
    uint8_t packet[40];
    uint8_t i = 0u;
    int16_t mag_x = 0;
    int16_t mag_y = 0;
    int16_t mag_z = 0;
    uint8_t flags = 0u;

    HMC5883L_ReadXYZ(&mag_x, &mag_y, &mag_z);

    if (Stepper_IsMoving())
    {
        flags |= 0x02u;
    }

    packet[i++] = RADIO_DEST_ID;
    packet[i++] = DEVICE_ID;
    packet[i++] = RADIO_PACKET_TYPE_TELEM;
    packet[i++] = seq++;
    packet[i++] = flags;
    PutU16(packet, &i, (uint16_t)mag_x);
    PutU16(packet, &i, (uint16_t)mag_y);
    PutU16(packet, &i, (uint16_t)mag_z);
    PutU32(packet, &i, (uint32_t)Stepper_GetPosition());

    (void)Radio_SendReliablePacket(packet, i);
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

    // Use reliable send for logs
    (void)Radio_SendReliablePacket(packet, i);
}

static void Radio_SendAudioChunk(void)
{
    static uint8_t frame_seq = 0;
    // Buffer to hold raw 24-bit stereo data from I2S
    uint16_t i2s_capture_buf[AUDIO_NUM_SAMPLES * 4]; // Each sample is 2x uint16_t, and we read L+R channels
    // Buffer to hold processed 16-bit mono PCM data
    int16_t pcm_buffer[AUDIO_NUM_SAMPLES];

    // 1. Capture Audio from I2S microphone
    // To get AUDIO_NUM_SAMPLES from one channel, we must read twice that from the interleaved (L/R) stream.
    if (HAL_I2S_Receive(&hi2s1, i2s_capture_buf, AUDIO_NUM_SAMPLES * 2, 1000) != HAL_OK)
    {
        return; // Failed to capture audio
    }

    // 2. Process raw data into 16-bit mono PCM
    for (uint32_t i = 0; i < AUDIO_NUM_SAMPLES; i++)
    {
        // Reconstruct the 24-bit signed sample from the left channel (at index i*4).
        int32_t sample32 = (int32_t)((i2s_capture_buf[i * 4] << 16) | i2s_capture_buf[i * 4 + 1]);
        // The data is 24-bit, so shift right by 8 to sign-extend it properly in the 32-bit container.
        sample32 >>= 8;
        // Convert to 16-bit by taking the most significant bits.
        pcm_buffer[i] = (int16_t)(sample32 >> 8);
    }

    // 3. Packetize and transmit the audio data
    uint16_t bytes_sent = 0;
    uint8_t packet_seq = 0;
    const uint16_t total_bytes = AUDIO_NUM_SAMPLES * sizeof(int16_t);

    while (bytes_sent < total_bytes)
    {
        uint8_t packet[61];
        uint8_t i = 0;

        uint8_t chunk_size = AUDIO_PACKET_PAYLOAD_SIZE;
        if (bytes_sent + chunk_size > total_bytes)
        {
            chunk_size = total_bytes - bytes_sent;
            // Set MSB of packet_seq to indicate the final packet of the frame
            packet_seq |= 0x80;
        }

        packet[i++] = RADIO_DEST_ID;
        packet[i++] = DEVICE_ID;
        packet[i++] = RADIO_PACKET_TYPE_AUDIO;
        packet[i++] = frame_seq;
        packet[i++] = packet_seq;
        memcpy(&packet[i], (uint8_t*)pcm_buffer + bytes_sent, chunk_size);

        Radio_TransmitPacket(packet, i + chunk_size);

        bytes_sent += chunk_size;
        packet_seq = (packet_seq & 0x7F) + 1; // Increment sequence, keeping MSB clear
    }
    frame_seq++;
}

static uint8_t Stepper_WaitUntilStopped(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();

    while (Stepper_IsMoving())
    {
        if ((HAL_GetTick() - start) >= timeout_ms)
        {
            Stepper_Stop();
            return 0u;
        }
    }

    return 1u;
}

static void Startup_TestStepper(void)
{
    Stepper_SetSpeed(400);
    Stepper_Enable();

    Stepper_SetDirection(GPIO_PIN_SET);
    Stepper_MoveTo(Stepper_GetPosition() + STEPPER_TEST_STEPS);
    if (Stepper_WaitUntilStopped(STEPPER_TEST_TIMEOUT_MS))
    {
        Radio_SendLog(LOG_STEPPER_CW_DONE, Stepper_GetPosition());
    }
    else
    {
        Radio_SendLog(LOG_STEPPER_TIMEOUT, 1);
        return;
    }

    HAL_Delay(250);

    Stepper_SetDirection(GPIO_PIN_RESET);
    Stepper_MoveTo(Stepper_GetPosition() - (STEPPER_TEST_STEPS * 2));
    if (Stepper_WaitUntilStopped(STEPPER_TEST_TIMEOUT_MS))
    {
        Radio_SendLog(LOG_STEPPER_CCW_DONE, Stepper_GetPosition());
    }
    else
    {
        Radio_SendLog(LOG_STEPPER_TIMEOUT, 2);
    }
}

static void Startup_TestI2S(void)
{
    // For 24-bit stereo, each single-channel sample is 2 half-words (32 bits).
    // A full stereo frame (L+R) is 4 half-words (64 bits).
    // The buffer needs to hold I2S_TEST_WORDS * 4 uint16_t's to get I2S_TEST_WORDS left channel samples.
    uint16_t i2s_rx_buf[I2S_TEST_WORDS * 4];
    uint32_t level = 0u;
    uint32_t count = 0u;

    // The 'Size' parameter for HAL_I2S_Receive is the number of single-channel 24-bit samples.
    // To get I2S_TEST_WORDS of left channel data, we need to read I2S_TEST_WORDS * 2 total samples (L & R).
    if (HAL_I2S_Receive(&hi2s1, i2s_rx_buf, I2S_TEST_WORDS * 2, 500) != HAL_OK)
    {
        Radio_SendLog(LOG_I2S_READ_FAIL, 0);
        return;
    }

    // Process only the left channel data. L/R=GND means mic is on the left.
    // The data is interleaved: L, R, L, R, ...
    // Each sample (L or R) consists of two 16-bit reads.
    for (uint32_t i = 0u; i < I2S_TEST_WORDS; i++)
    {
        // Reconstruct the 24-bit signed sample from the left channel (at index i*4).
        // The INMP441 data is left-justified in a 32-bit frame.
        int32_t sample = (int32_t)((i2s_rx_buf[i * 4] << 16) | i2s_rx_buf[i * 4 + 1]);
        sample >>= 8; // Right-shift to sign-extend from 24-bit to 32-bit.

        level += (sample < 0) ? (uint32_t)(-sample) : (uint32_t)sample;
        count++;
    }

    if (count > 0u)
    {
        level /= count;
    }

    Radio_SendLog((level >= I2S_SOUND_THRESHOLD) ? LOG_I2S_SOUND_OK : LOG_I2S_SOUND_LOW,
                  (int32_t)level);
}

/**
  * @brief Sends a test packet over the radio to confirm transmitter is working.
  */
static void Startup_TestRadioPacket(void)
{
    Radio_SendLog(LOG_RADIO_TEST_OK, 0x12345678);
    HAL_Delay(100); // Small delay to ensure packet is sent before moving on
}

/**
  * @brief Tests the SPI connection to the CC1101 module.
  *        Blinks LED2 (green) if successful, LED1 (red) if failed.
  */
static void Test_CC1101_Connection(void)
{
    // Hardware reset by toggling CS pin
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
    HAL_Delay(1);

    // Reset the CC1101 chip to ensure it's in a known state.
    CC1101_Strobe(CC1101_SRES); // SRES strobe
    HAL_Delay(1);        // Short delay for the chip to stabilize after reset.

    // The VERSION register (address 0x31) should return a known value, e.g., 0x14.
    // A value of 0x00 or 0xFF often indicates a communication failure.
    uint8_t version = CC1101_ReadReg(CC1101_VERSION);

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
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_I2S1_Init();
  /* USER CODE BEGIN 2 */

  boot();

  /* Bring up the N82020S camera, confirm the link, then relay its feed. */
  Test_CC1101_Connection();
  CC1101_InitRadio();
  Radio_SendLog(LOG_BOOT, DEVICE_ID);
  Startup_TestRadioPacket();

  /* ---- A4988 stepper: 1x clockwise, then 2x counter-clockwise. ---- */
  Stepper_Init();
  Startup_TestStepper();

  /* ---- INMP441/I2S: listen once for motor/drone sound level. ---- */
  Startup_TestI2S();

  /* ---- GY-271 magnetometer ---- */
  HMC5883L_Init();
  (void)HMC5883L_Test();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    Radio_SendAudioChunk();
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
  /* USER CODE BEGIN 6 */
  void assert_failed(uint8_t *file, uint32_t line)
  {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\\r\\n", file, line) */
    /* USER CODE END 6 */
  }
  /* USER CODE END 6 */
#endif /* USE_FULL_ASSERT */
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */
/* USER CODE BEGIN main */
/* USER CODE END main */
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
