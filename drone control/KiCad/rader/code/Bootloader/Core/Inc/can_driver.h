/**
  ******************************************************************************
  * @file    can_driver.h
  * @brief   FDCAN Driver Header - LL-based, 1Mbps, PB0(RX)/PB1(TX)
  ******************************************************************************
  */
#ifndef __CAN_DRIVER_H
#define __CAN_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CAN_DEFAULT_BITRATE     1000000
#define CAN_MAX_DATA_LEN        8
#define CAN_RX_FIFO_SIZE        16

/* FDCAN IDs for OTA */
#define CAN_OTA_START_ID        0x100
#define CAN_OTA_DATA_ID         0x101
#define CAN_OTA_END_ID          0x102
#define CAN_OTA_ACK_ID          0x103

/* Return codes */
#define CAN_OK                  0
#define CAN_ERROR               -1
#define CAN_BUS_OFF             -2
#define CAN_TIMEOUT             -3

void    CAN_Init(uint32_t bitrate);
int     CAN_Transmit(uint32_t id, const uint8_t* data, uint8_t len, int is_extended);
int     CAN_Receive(uint32_t* id, uint8_t* data, uint8_t* len, uint32_t timeout_ms);
void    CAN_SetFilter(uint32_t id, uint32_t mask);
int     CAN_CheckBusOff(void);
void    CAN_ResetBus(void);
uint32_t CAN_GetErrorCount(void);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_DRIVER_H */