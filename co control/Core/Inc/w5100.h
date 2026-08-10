/**
  ******************************************************************************
  * @file    w5100.h
  * @brief   WIZnet W5100 Ethernet controller driver for STM32F401RE
  *          Arduino Ethernet Shield (W5100) interface via SPI1
  *          Robust version with command timeouts, PHY link detection,
  *          and reliable send/receive.
  ******************************************************************************
  */

#ifndef __W5100_H
#define __W5100_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*============================================================================
 * W5100 Register Definitions
 *============================================================================*/

/* Common Registers */
#define W5100_MR          0x0000  /* Mode Register */
#define W5100_GAR         0x0001  /* Gateway Address Register (4 bytes) */
#define W5100_SUBR        0x0005  /* Subnet Mask Register (4 bytes) */
#define W5100_SHAR        0x0009  /* Source Hardware Address Register (6 bytes) */
#define W5100_SIPR        0x000F  /* Source IP Address Register (4 bytes) */
#define W5100_IR          0x0015  /* Interrupt Register */
#define W5100_IMR         0x0016  /* Interrupt Mask Register */
#define W5100_RTR         0x0017  /* Retry Time Register (2 bytes) */
#define W5100_RCR         0x0019  /* Retry Count Register */
#define W5100_RMSR        0x001A  /* RX Memory Size Register */
#define W5100_TMSR        0x001B  /* TX Memory Size Register */
#define W5100_PTIMER      0x0028  /* PPPoE LCP Request Timer */
#define W5100_PMAGIC      0x0029  /* PPPoE LCP Magic Number */
#define W5100_UIPR        0x002A  /* Unreachable IP Address (4 bytes) */
#define W5100_UPORT       0x002E  /* Unreachable Port (2 bytes) */

/* Socket Registers (base + n*0x100, n = 0-3) */
#define W5100_SOCKET_BASE  0x0400
#define W5100_SOCKET_STRIDE 0x0100

#define W5100_Sn_MR        0x0000  /* Socket n Mode */
#define W5100_Sn_CR        0x0001  /* Socket n Command */
#define W5100_Sn_IR        0x0002  /* Socket n Interrupt */
#define W5100_Sn_SR        0x0003  /* Socket n Status */
#define W5100_Sn_PORT      0x0004  /* Socket n Source Port (2 bytes) */
#define W5100_Sn_DHAR      0x0006  /* Socket n Dest HW Addr (6 bytes) */
#define W5100_Sn_DIPR      0x000C  /* Socket n Dest IP Addr (4 bytes) */
#define W5100_Sn_DPORT     0x0010  /* Socket n Dest Port (2 bytes) */
#define W5100_Sn_MSSR      0x0012  /* Socket n Max Segment Size (2 bytes) */
#define W5100_Sn_TOS       0x0015  /* Socket n Type of Service */
#define W5100_Sn_TTL       0x0016  /* Socket n Time to Live */
#define W5100_Sn_TX_FSR    0x0020  /* Socket n TX Free Size (2 bytes) */
#define W5100_Sn_TX_RD     0x0022  /* Socket n TX Read Pointer (2 bytes) */
#define W5100_Sn_TX_WR     0x0024  /* Socket n TX Write Pointer (2 bytes) */
#define W5100_Sn_RX_RSR    0x0026  /* Socket n RX Received Size (2 bytes) */
#define W5100_Sn_RX_RD     0x0028  /* Socket n RX Read Pointer (2 bytes) */
#define W5100_Sn_RX_WR     0x002A  /* Socket n RX Write Pointer (2 bytes) */
#define W5100_Sn_IMR       0x002C  /* Socket n Interrupt Mask */
#define W5100_Sn_FRAG      0x002D  /* Socket n Fragment Field (2 bytes) */

/* TX/RX Buffer Base Addresses */
#define W5100_TX_BASE      0x4000
#define W5100_RX_BASE      0x6000
/* W5100 has 8KB total TX and 8KB total RX memory. With 2KB per socket
   (W5100_SetTxRxBufferSize(2,2)), each socket's buffer is 0x0800 bytes. */
#define W5100_SOCKET_BUF_SIZE   0x0800  /* 2KB per socket with default config */
#define W5100_SOCKET_BUF_MASK   (W5100_SOCKET_BUF_SIZE - 1)

/*============================================================================
 * W5100 Register Values
 *============================================================================*/

/* Mode Register (MR) */
#define W5100_MR_RST       0x80    /* Software Reset */
#define W5100_MR_PB        0x10    /* Ping Block */
#define W5100_MR_PPPOE     0x08    /* PPPoE Mode */
#define W5100_MR_AI        0x02    /* Auto-Increment */
#define W5100_MR_IND       0x01    /* Indirect Bus I/F Mode */

/* Socket Mode Register (Sn_MR) */
#define W5100_SNMR_CLOSE   0x00    /* Close */
#define W5100_SNMR_TCP     0x01    /* TCP */
#define W5100_SNMR_UDP     0x02    /* UDP */
#define W5100_SNMR_IPRAW   0x03    /* IP LAYER RAW */
#define W5100_SNMR_MACRAW  0x04    /* MAC LAYER RAW */
#define W5100_SNMR_PPPOE   0x05    /* PPPoE */
#define W5100_SNMR_ND      0x20    /* No Delayed ACK */
#define W5100_SNMR_MULTI   0x80    /* Multicast */

/* Socket Command Register (Sn_CR) */
#define W5100_SNCR_OPEN    0x01    /* Initialize socket */
#define W5100_SNCR_LISTEN  0x02    /* Wait for connection (TCP) */
#define W5100_SNCR_CONNECT 0x04    /* Send connection request (TCP) */
#define W5100_SNCR_DISCON  0x08    /* Send disconnect (TCP) */
#define W5100_SNCR_CLOSE   0x10    /* Close socket */
#define W5100_SNCR_SEND    0x20    /* Send data */
#define W5100_SNCR_SEND_MAC 0x21   /* Send data with MAC (MACRAW) */
#define W5100_SNCR_SEND_KEEP 0x22  /* Send keep alive (TCP) */
#define W5100_SNCR_RECV    0x40    /* Receive data */

/* Socket Status Register (Sn_SR) */
#define W5100_SNSR_CLOSED      0x00
#define W5100_SNSR_INIT        0x13
#define W5100_SNSR_LISTEN      0x14
#define W5100_SNSR_SYNSENT     0x15
#define W5100_SNSR_SYNRECV     0x16
#define W5100_SNSR_ESTABLISHED 0x17
#define W5100_SNSR_FIN_WAIT    0x18
#define W5100_SNSR_CLOSING     0x1A
#define W5100_SNSR_TIME_WAIT   0x1B
#define W5100_SNSR_CLOSE_WAIT  0x1C
#define W5100_SNSR_LAST_ACK    0x1D
#define W5100_SNSR_UDP         0x22
#define W5100_SNSR_IPRAW       0x32
#define W5100_SNSR_MACRAW      0x42
#define W5100_SNSR_PPPOE       0x5F

/* Socket Interrupt Register (Sn_IR) */
#define W5100_SNIR_SEND_OK   0x10
#define W5100_SNIR_TIMEOUT   0x08
#define W5100_SNIR_RECV      0x04
#define W5100_SNIR_DISCON    0x02
#define W5100_SNIR_CON       0x01

/*============================================================================
 * Robust Driver Configuration
 *============================================================================*/

/* Command timeout in loop iterations (prevents infinite hang) */
#define W5100_CMD_TIMEOUT   10000


/*============================================================================
 * Robust Driver Configuration
 *============================================================================*/

/* Command timeout in loop iterations (prevents infinite hang) */
#define W5100_CMD_TIMEOUT   10000

/*============================================================================
 * Configuration
 *============================================================================*/

/**
 * @brief Structure for abstracting hardware-specific functions.
 */
typedef struct {
    void (*spi_select)(void);      /*!< Select W5100 SPI slave (CS low). */
    void (*spi_unselect)(void);    /*!< Unselect W5100 SPI slave (CS high). */
    uint8_t (*spi_read_byte)(void); /*!< Read a single byte from SPI. */
    void (*spi_write_byte)(uint8_t byte); /*!< Write a single byte to SPI. */
    void (*reset)(void);           /*!< Hardware reset (optional, can be NULL). */
    void (*delay_ms)(uint32_t ms); /*!< Millisecond delay (required). */
} w5100_io_t;

/**
 * @brief W5100 network configuration structure.
 */
typedef struct {
    uint8_t mac[6];     /*!< MAC Address */
    uint8_t ip[4];      /*!< IP Address */
    uint8_t gateway[4]; /*!< Gateway Address */
    uint8_t subnet[4];  /*!< Subnet Mask */
} w5100_net_config_t;

#define W5100_MAX_SOCKETS    4
#define W5100_SPI_TIMEOUT    1000

/* Default network configuration */
#define W5100_DEFAULT_MAC0   0x02
#define W5100_DEFAULT_MAC1   0x00
#define W5100_DEFAULT_MAC2   0x00
#define W5100_DEFAULT_MAC3   0x00
#define W5100_DEFAULT_MAC4   0x00
#define W5100_DEFAULT_MAC5   0x01

#define W5100_DEFAULT_IP0    10
#define W5100_DEFAULT_IP1    87
#define W5100_DEFAULT_IP2    243
#define W5100_DEFAULT_IP3    100

#define W5100_DEFAULT_GW0    10
#define W5100_DEFAULT_GW1    87
#define W5100_DEFAULT_GW2    243
#define W5100_DEFAULT_GW3    71

#define W5100_DEFAULT_SUB0   255
#define W5100_DEFAULT_SUB1   255
#define W5100_DEFAULT_SUB2   255
#define W5100_DEFAULT_SUB3   0

/*============================================================================
 * Public API
 *============================================================================*/

/* Initialization */
int8_t W5100_Init(const w5100_io_t* io_fns); // Returns 0 on success, -1 on failure
void W5100_ConfigureNetwork(const w5100_net_config_t* net_config);
void W5100_Reset(void);
void W5100_SetMACAddress(const uint8_t *mac);
void W5100_SetIPAddress(const uint8_t *ip);
void W5100_SetGateway(const uint8_t *gw);
void W5100_SetSubnetMask(const uint8_t *sub);
void W5100_GetNetworkInfo(w5100_net_config_t* net_config);
void W5100_SetTxRxBufferSize(uint8_t tx_size, uint8_t rx_size); /* Only 2,2 is supported. */

/* Diagnostics */
int8_t W5100_TestCommunication(void);  /* 0 on success, -1 on failure */

/* Socket operations */
uint8_t W5100_SocketInit(uint8_t sock, uint8_t protocol, uint16_t port, uint8_t flag);
uint8_t W5100_SocketConnect(uint8_t sock, const uint8_t *ip, uint16_t port);
uint8_t W5100_SocketListen(uint8_t sock);
uint8_t W5100_SocketDisconnect(uint8_t sock);
uint8_t W5100_SocketClose(uint8_t sock);
uint8_t W5100_SocketGetStatus(uint8_t sock);
uint16_t W5100_SocketGetTXFreeSize(uint8_t sock);
uint16_t W5100_SocketGetRXReceivedSize(uint8_t sock);
uint16_t W5100_SocketSend(uint8_t sock, const uint8_t *data, uint16_t len);
uint16_t W5100_SocketRecv(uint8_t sock, uint8_t *data, uint16_t len);
void W5100_SocketSetDestIP(uint8_t sock, const uint8_t *ip);
void W5100_SocketSetDestPort(uint8_t sock, uint16_t port);

/* Low-level register access */
void W5100_WriteByte(uint16_t addr, uint8_t data);
uint8_t W5100_ReadByte(uint16_t addr);
void W5100_WriteBuffer(uint16_t addr, const uint8_t *data, uint16_t len);
void W5100_ReadBuffer(uint16_t addr, uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __W5100_H */
