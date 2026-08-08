/**
******************************************************************************
* @file    client_node.c
* @brief   CC1101 Client Node application for STM32C092KCT
*          1-Master / 7-Client polling network
*
*          Phase 1: Parse config from flash (ID:Password)
*          Phase 2: Dynamic provisioning (Join Request/Accept)
*          Phase 3: Main polling loop (RX -> process -> TX response)
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "client_node.h"
#include "gpio.h"
#include "gps_compass.h"
#include <string.h>
#include <stdlib.h>

#ifdef HAS_AUDIO_NODE
#include "i2s_mic.h"
#endif

/*============================================================================*/
/* Private Variables                                                          */
/*============================================================================*/

volatile uint8_t client_rx_flag = 0;
volatile uint8_t client_my_addr = CC1101_ADDR_BROADCAST;

static uint8_t rx_buffer[PKT_MAX_LEN];
static uint8_t tx_buffer[PKT_MAX_LEN];

/*============================================================================*/
/* Phase 1: Flash Memory Parsing                                              */
/*============================================================================*/

/**
  * @brief  Read configuration string from flash and extract ID and Password
  * @param  id_out:  Buffer for 4-digit ID (null-terminated)
  * @param  pwd_out: Buffer for 4-digit Password (null-terminated)
  * @retval 1 on success, 0 on failure
  */
uint8_t Client_ParseFlashConfig(char *id_out, char *pwd_out)
{
    char config[CONFIG_MAX_LEN];
    char *colon;
    char *comma;
    uint32_t i;

    /* Read config string from flash */
    for (i = 0; i < CONFIG_MAX_LEN; i++)
    {
        config[i] = *(volatile char *)(CONFIG_FLASH_ADDR + i);
        if (config[i] == '\0' || config[i] == 0xFF)
        {
            config[i] = '\0';
            break;
        }
    }
    config[CONFIG_MAX_LEN - 1] = '\0';

    /* Find colon separator between ID and Password */
    colon = strchr(config, ':');
    if (colon == NULL)
    {
        return 0;
    }

    /* Extract 4-digit ID (before colon) */
    comma = strchr(config, ',');
    if (comma == NULL || comma > colon)
    {
        return 0;
    }

    /* ID is the last 4 chars before the colon */
    {
        char *id_start = colon - 4;
        if (id_start < config)
        {
            return 0;
        }
        strncpy(id_out, id_start, 4);
        id_out[4] = '\0';
    }

    /* Password is the 4 chars after the colon */
    strncpy(pwd_out, colon + 1, 4);
    pwd_out[4] = '\0';

    return 1;
}

/*============================================================================*/
/* Phase 2: Dynamic Provisioning (One-Time Access Handshake)                  */
/*============================================================================*/

/**
  * @brief  Perform the Join Request / Join Accept handshake with the Master
  * @param  id:   4-digit ID string
  * @param  pwd:  4-digit Password string
  * @param  assigned_addr: Output for assigned 1-byte short address (0x01-0x07)
  * @retval 1 on success, 0 on failure
  */
uint8_t Client_Provision(char *id, char *pwd, uint8_t *assigned_addr)
{
    uint8_t attempt;
    uint8_t rx_len;
    uint32_t timeout;

    /* Set hardware address to broadcast for provisioning */
    CC1101_SetAddress(CC1101_ADDR_BROADCAST);

    for (attempt = 0; attempt < PROV_RETRY_COUNT; attempt++)
    {
        /* Build Join Request packet:
         * [0] len, [1] dst=0x00, [2] src=0x00, [3] cmd=0x10,
         * [4..7] ID, [8..11] Password
         */
        tx_buffer[0] = 12;                    /* Length (12 payload bytes) */
        tx_buffer[1] = NETWORK_MASTER_ADDR;   /* Destination = Master     */
        tx_buffer[2] = CC1101_ADDR_BROADCAST; /* Source = Broadcast        */
        tx_buffer[3] = PROV_CMD_JOIN_REQ;     /* Command = Join Request    */
        memcpy(&tx_buffer[4], id, 4);         /* 4-digit ID                */
        memcpy(&tx_buffer[8], pwd, 4);        /* 4-digit Password          */

        /* Flush FIFOs and transmit */
        CC1101_FlushTxFifo();
        CC1101_FlushRxFifo();
        CC1101_WriteTxFifo(tx_buffer, 13);
        CC1101_EnterTx();

        /* Wait for TX to complete (GDO0 goes low when TX done) */
        timeout = HAL_GetTick() + 100;
        while (HAL_GetTick() < timeout)
        {
            if (HAL_GPIO_ReadPin(CC1101_GDO0_PORT, CC1101_GDO0_PIN) == GPIO_PIN_RESET)
            {
                break;
            }
        }

        /* Enter RX mode and wait for Join Accept */
        CC1101_FlushTxFifo();
        CC1101_FlushRxFifo();
        CC1101_EnterRx();

        timeout = HAL_GetTick() + PROV_TIMEOUT_MS;
        while (HAL_GetTick() < timeout)
        {
            /* Check if GDO0 asserted (packet received) */
            if (HAL_GPIO_ReadPin(CC1101_GDO0_PORT, CC1101_GDO0_PIN) == GPIO_PIN_SET)
            {
                rx_len = CC1101_ReadRxFifo(rx_buffer, PKT_MAX_LEN);

                /* Validate Join Accept packet:
                 * [0] len, [1] dst=0x00, [2] src=0x00, [3] cmd=0x11,
                 * [4..7] ID, [8] assigned address
                 */
                if (rx_len >= 9 &&
                    rx_buffer[3] == PROV_CMD_JOIN_ACCEPT &&
                    memcmp(&rx_buffer[4], id, 4) == 0)
                {
                    *assigned_addr = rx_buffer[8];

                    /* Validate assigned address range */
                    if (*assigned_addr >= CLIENT_ADDR_MIN &&
                        *assigned_addr <= CLIENT_ADDR_MAX)
                    {
                        /* Permanently set hardware address */
                        CC1101_SetAddress(*assigned_addr);
                        client_my_addr = *assigned_addr;

                        /* Flush and return to RX */
                        CC1101_FlushRxFifo();
                        CC1101_EnterRx();
                        return 1;
                    }
                }

                /* Invalid packet - flush and continue waiting */
                CC1101_FlushRxFifo();
            }
        }
    }

    /* Provisioning failed */
    return 0;
}

/*============================================================================*/
/* Phase 3: Main Polling Loop                                                 */
/*============================================================================*/

/**
  * @brief  Process a received command packet from the Master
  * @param  rx_buf: Received packet buffer
  * @param  rx_len: Received packet length
  */
void Client_ProcessPacket(uint8_t *rx_buf, uint8_t rx_len)
{
    uint8_t cmd;
    uint8_t payload[PKT_MAX_PAYLOAD];
    uint8_t payload_len = 0;

    /* Combined sensor data from main.c */
    extern Sensor_Data_t sensor_data;
#ifdef HAS_AUDIO_NODE
    /* Audio result from i2s_mic.c */
    extern volatile I2S_Mic_Result_t i2s_mic_result;
#endif

    /* Validate minimum packet length */
    if (rx_len < PKT_HEADER_LEN)
    {
        return;
    }

    cmd = rx_buf[3];

    /* Process command */
    switch (cmd)
    {
        case CMD_PING:
            /* Respond with ACK */
            payload[0] = CMD_ACK;
            payload_len = 1;
            break;

        case CMD_GET_STATUS:
            /* Mock status: [0]=status, [1]=mode */
            payload[0] = 0x01;   /* Status: OK */
            payload[1] = 0x00;   /* Mode: Normal */
            payload_len = 2;
            break;

        case CMD_GET_SENSOR:
            /* Mock sensor data: [0..1]=temp, [2..3]=humidity, [4..5]=pressure */
            payload[0] = 0x1E;   /* Temp high byte (30.0 C) */
            payload[1] = 0x00;   /* Temp low byte  */
            payload[2] = 0x28;   /* Humidity high byte (40%) */
            payload[3] = 0x00;   /* Humidity low byte */
            payload[4] = 0x01;   /* Pressure high byte (1013 hPa) */
            payload[5] = 0xFD;   /* Pressure low byte */
            payload_len = 6;
            break;

        case CMD_SET_MODE:
            /* Set mode: echo back the mode value */
            if (rx_len > PKT_HEADER_LEN)
            {
                payload[0] = rx_buf[4];  /* Mode value */
            }
            else
            {
                payload[0] = 0;
            }
            payload[1] = CMD_ACK;
            payload_len = 2;
            break;

        case CMD_GET_TEMP:
            /* Mock temperature: 30.5 C */
            payload[0] = 0x1E;   /* 30 */
            payload[1] = 0x05;   /* .5 */
            payload_len = 2;
            break;

        case CMD_GET_BATTERY:
            /* Mock battery: 3.7V */
            payload[0] = 0x03;   /* 3 */
            payload[1] = 0x07;   /* .7 */
            payload_len = 2;
            break;

        default:
            /* Unknown command - respond with NACK */
            payload[0] = CMD_NACK;
            payload_len = 1;
            break;
    }

    /* Always include GPS (Latitude, Longitude, Altitude) and Compass (Heading) */
    if (sensor_data.gps.valid)
    {
        /* Append GPS position:
         * [0..3] = latitude (scaled x100000)
         * [4..7] = longitude (scaled x100000)
         * [8..9] = altitude (scaled x10)
         */
        if (payload_len + 10 <= PKT_MAX_PAYLOAD)
        {
            int32_t lat_scaled = (int32_t)(sensor_data.gps.latitude * 100000.0f);
            int32_t lon_scaled = (int32_t)(sensor_data.gps.longitude * 100000.0f);
            int16_t alt_scaled = (int16_t)(sensor_data.gps.altitude_m * 10.0f);

            payload[payload_len + 0] = (uint8_t)(lat_scaled >> 24);
            payload[payload_len + 1] = (uint8_t)(lat_scaled >> 16);
            payload[payload_len + 2] = (uint8_t)(lat_scaled >> 8);
            payload[payload_len + 3] = (uint8_t)(lat_scaled & 0xFF);
            payload[payload_len + 4] = (uint8_t)(lon_scaled >> 24);
            payload[payload_len + 5] = (uint8_t)(lon_scaled >> 16);
            payload[payload_len + 6] = (uint8_t)(lon_scaled >> 8);
            payload[payload_len + 7] = (uint8_t)(lon_scaled & 0xFF);
            payload[payload_len + 8] = (uint8_t)(alt_scaled >> 8);
            payload[payload_len + 9] = (uint8_t)(alt_scaled & 0xFF);

            payload_len += 10;
        }
    }

    /* Append Compass heading */
    if (sensor_data.compass.valid)
    {
        /* [0..1] = heading_deg (scaled x100) */
        if (payload_len + 2 <= PKT_MAX_PAYLOAD)
        {
            int16_t heading_scaled = (int16_t)(sensor_data.compass.heading_deg * 100.0f);

            payload[payload_len + 0] = (uint8_t)(heading_scaled >> 8);
            payload[payload_len + 1] = (uint8_t)(heading_scaled & 0xFF);

            payload_len += 2;
        }
    }

#ifdef HAS_AUDIO_NODE
    /* Append compressed audio RMS levels if audio node */
    if (i2s_mic_result.audio_valid)
    {
        /* [0..1] = mic1_rms (primary sound)
         * [2..3] = mic2_rms (noise reference)
         */
        if (payload_len + 4 <= PKT_MAX_PAYLOAD)
        {
            payload[payload_len + 0] = (uint8_t)(i2s_mic_result.mic1_rms >> 8);
            payload[payload_len + 1] = (uint8_t)(i2s_mic_result.mic1_rms & 0xFF);
            payload[payload_len + 2] = (uint8_t)(i2s_mic_result.mic2_rms >> 8);
            payload[payload_len + 3] = (uint8_t)(i2s_mic_result.mic2_rms & 0xFF);

            payload_len += 4;
            i2s_mic_result.audio_valid = 0;
        }
    }
#endif

    /* Send response back to Master */
    Client_SendResponse(cmd, payload, payload_len);
}

/**
  * @brief  Send a response packet back to the Master
  * @param  cmd: Command being responded to
  * @param  payload: Response payload
  * @param  payload_len: Payload length
  */
void Client_SendResponse(uint8_t cmd, uint8_t *payload, uint8_t payload_len)
{
    uint8_t total_len;
    uint32_t timeout;

    /* Build response packet:
     * [0] len, [1] dst=0x00, [2] src=my_addr, [3] cmd, [4..] payload
     */
    total_len = PKT_HEADER_LEN + payload_len;
    tx_buffer[0] = total_len - 1;             /* Length byte */
    tx_buffer[1] = NETWORK_MASTER_ADDR;       /* Destination = Master */
    tx_buffer[2] = client_my_addr;            /* Source = my address  */
    tx_buffer[3] = cmd;                       /* Command echo         */
    if (payload_len > 0)
    {
        memcpy(&tx_buffer[4], payload, payload_len);
    }

    /* Flush TX FIFO and transmit */
    CC1101_FlushTxFifo();
    CC1101_WriteTxFifo(tx_buffer, total_len);
    CC1101_EnterTx();

    /* Wait for TX to complete (GDO0 goes low) */
    timeout = HAL_GetTick() + 100;
    while (HAL_GetTick() < timeout)
    {
        if (HAL_GPIO_ReadPin(CC1101_GDO0_PORT, CC1101_GDO0_PIN) == GPIO_PIN_RESET)
        {
            break;
        }
    }
}

/**
  * @brief  Return to RX mode after processing
  */
void Client_EnterRxMode(void)
{
    CC1101_FlushTxFifo();
    CC1101_FlushRxFifo();
    CC1101_EnterRx();
}

/**
  * @brief  GDO0 interrupt handler - called from EXTI0_1_IRQHandler
  */
void Client_Gdo0Handler(void)
{
    /* Clear the EXTI pending bit */
    __HAL_GPIO_EXTI_CLEAR_IT(CC1101_GDO0_PIN);

    /* Set flag - main loop will process the packet */
    client_rx_flag = 1;
}