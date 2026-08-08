/**
  ******************************************************************************
  * @file    client_node.h
  * @brief   CC1101 Client Node application for STM32C092KCT
  *          1-Master / 7-Client polling network
  ******************************************************************************
  */

#ifndef __CLIENT_NODE_H
#define __CLIENT_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cc1101.h"

/*============================================================================*/
/* Flash Configuration                                                        */
/*============================================================================*/
#define CONFIG_FLASH_ADDR       0x08004000UL   /* Config string in flash */
#define CONFIG_MAX_LEN          32

/*============================================================================*/
/* Network Protocol Constants                                                 */
/*============================================================================*/
#define NETWORK_MASTER_ADDR     0x00            /* Master address (broadcast) */
#define CLIENT_ADDR_MIN         0x01            /* Client address range 1-7  */
#define CLIENT_ADDR_MAX         0x07

/*============================================================================*/
/* Command Constants (sent by Master)                                         */
/*============================================================================*/
#define CMD_PING                0x01            /* Ping request               */
#define CMD_GET_STATUS          0x02            /* Get device status          */
#define CMD_GET_SENSOR          0x03            /* Get sensor data            */
#define CMD_SET_MODE            0x04            /* Set operating mode         */
#define CMD_GET_TEMP            0x05            /* Get temperature            */
#define CMD_GET_BATTERY         0x06            /* Get battery voltage        */
#define CMD_ACK                 0x7F            /* Acknowledge                */
#define CMD_NACK                0x7E            /* Not acknowledged           */

/*============================================================================*/
/* Packet Structure                                                           */
/*============================================================================*/
/* CC1101 variable-length packet format:
 *   [0] = Length (payload bytes, excluding length byte)
 *   [1] = Destination address
 *   [2] = Source address
 *   [3] = Command
 *   [4..N] = Payload data
 */
#define PKT_HEADER_LEN          4               /* len + dst + src + cmd     */
#define PKT_MAX_PAYLOAD         16              /* Max payload bytes         */
#define PKT_MAX_LEN             (PKT_HEADER_LEN + PKT_MAX_PAYLOAD)

/*============================================================================*/
/* Provisioning Protocol                                                      */
/*============================================================================*/
#define PROV_CMD_JOIN_REQ       0x10            /* Join Request              */
#define PROV_CMD_JOIN_ACCEPT    0x11            /* Join Accept               */
#define PROV_TIMEOUT_MS         5000            /* 5s wait for Join Accept   */
#define PROV_RETRY_COUNT        3               /* Retry Join Request        */

/*============================================================================*/
/* Client Node API                                                            */
/*============================================================================*/

/* Phase 1: Flash parsing */
uint8_t Client_ParseFlashConfig(char *id_out, char *pwd_out);

/* Phase 2: Provisioning */
uint8_t Client_Provision(char *id, char *pwd, uint8_t *assigned_addr);

/* Phase 3: Main loop processing */
void     Client_ProcessPacket(uint8_t *rx_buf, uint8_t rx_len);
void     Client_SendResponse(uint8_t cmd, uint8_t *payload, uint8_t payload_len);
void     Client_EnterRxMode(void);

/* GDO0 interrupt handler (called from EXTI0_1_IRQHandler) */
void     Client_Gdo0Handler(void);

/* Global state */
extern volatile uint8_t  client_rx_flag;
extern volatile uint8_t  client_my_addr;

#ifdef __cplusplus
}
#endif

#endif /* __CLIENT_NODE_H */