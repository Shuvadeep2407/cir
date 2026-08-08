/**
  ******************************************************************************
  * @file    ota.h
  * @brief   Over-The-Air Firmware Update Module
  *          Supports GSM (primary) and CAN (secondary) update channels
  ******************************************************************************
  */
#ifndef __OTA_H
#define __OTA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* OTA protocol constants */
#define OTA_PACKET_SIZE         256
#define OTA_HEADER_SIZE         16
#define OTA_MAX_PAYLOAD_SIZE    (OTA_PACKET_SIZE - OTA_HEADER_SIZE)  /* 240 */

/* OTA packet types */
#define OTA_PKT_START           0x01    /* Start of firmware transfer */
#define OTA_PKT_DATA            0x02    /* Firmware data chunk */
#define OTA_PKT_END             0x03    /* End of firmware transfer */
#define OTA_PKT_ACK             0x04    /* Acknowledgment */
#define OTA_PKT_NACK            0x05    /* Negative acknowledgment */
#define OTA_PKT_ABORT           0x06    /* Abort transfer */
#define OTA_PKT_STATUS          0x07    /* Status request */

/* OTA packet structure */
typedef struct {
    uint8_t  type;                      /* Packet type */
    uint8_t  seq;                       /* Sequence number */
    uint16_t crc16;                     /* CRC16 of payload */
    uint32_t offset;                    /* Data offset in firmware */
    uint16_t length;                    /* Payload length */
    uint8_t  reserved[6];               /* Reserved */
    uint8_t  payload[OTA_MAX_PAYLOAD_SIZE];
} __attribute__((packed)) OTA_Packet_t;

/* OTA firmware header (first packet payload) */
typedef struct {
    uint32_t magic;                     /* 0x4F544131 ("OTA1") */
    uint32_t firmware_size;             /* Total firmware size */
    uint32_t firmware_crc32;            /* CRC32 of entire firmware */
    uint32_t firmware_version;          /* Version number */
    uint8_t  reserved[8];               /* Reserved */
} __attribute__((packed)) OTA_FirmwareHeader_t;

#define OTA_MAGIC               0x3141544FUL  /* "OTA1" */

/* OTA states */
typedef enum {
    OTA_STATE_IDLE = 0,
    OTA_STATE_WAITING_START,
    OTA_STATE_RECEIVING,
    OTA_STATE_FLASHING,
    OTA_STATE_VERIFYING,
    OTA_STATE_COMPLETE,
    OTA_STATE_ERROR
} OTA_State_t;

/* OTA return codes */
#define OTA_OK                  0
#define OTA_ERR_INVALID         -1
#define OTA_ERR_CRC             -2
#define OTA_ERR_FLASH           -3
#define OTA_ERR_TIMEOUT         -4
#define OTA_ERR_SIZE            -5
#define OTA_ERR_ABORTED         -6
#define OTA_ERR_NO_MEM          -7

/* OTA configuration */
#define OTA_RX_TIMEOUT_MS       10000
#define OTA_MAX_RETRIES         3
#define OTA_ACK_TIMEOUT_MS      2000

/* Function prototypes */
int     OTA_Init(uint8_t source);
int     OTA_StartTransfer(uint32_t total_size, uint32_t crc32, uint32_t version);
int     OTA_ReceiveChunk(const uint8_t* data, uint16_t len, uint32_t offset);
int     OTA_EndTransfer(void);
int     OTA_AbortTransfer(void);
int     OTA_ProcessPacket(OTA_Packet_t* pkt);
int     OTA_GetProgress(uint32_t* received, uint32_t* total);
OTA_State_t OTA_GetState(void);
int     OTA_FlashFirmware(void);
int     OTA_VerifyFirmware(void);

/* Channel-specific OTA */
int     OTA_ReceiveFromGSM(void);
int     OTA_ReceiveFromCAN(void);

#ifdef __cplusplus
}
#endif

#endif /* __OTA_H */