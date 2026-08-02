/**
  ******************************************************************************
  * @file    gsm_driver.h
  * @brief   GSM Module (USART1) Driver Header - LL-based for minimal size
  ******************************************************************************
  */
#ifndef __GSM_DRIVER_H
#define __GSM_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* UART buffer sizes */
#define GSM_TX_BUF_SIZE         512
#define GSM_RX_BUF_SIZE         1024

/* Response timeouts */
#define GSM_AT_TIMEOUT_MS       5000
#define GSM_NETWORK_TIMEOUT_MS  60000
#define GSM_PAYLOAD_TIMEOUT_MS  30000

/* AT Command responses */
#define GSM_OK                  0
#define GSM_ERROR               -1
#define GSM_TIMEOUT             -2
#define GSM_BUSY                -3

/* GSM Module power control pin - GPIO PA2 (c1101_clk reused for GSM_PWR) */
#define GSM_PWR_PIN             GPIO_PIN_2
#define GSM_PWR_PORT            GPIOA

/* SIM800L/SIM7000 etc typical baud */
#define GSM_DEFAULT_BAUD        115200

/* Function prototypes */
void    GSM_Init(uint32_t baud);
void    GSM_SendData(const uint8_t* data, uint16_t len);
int     GSM_ReceiveData(uint8_t* buffer, uint16_t max_len, uint32_t timeout_ms);
int     GSM_SendAT(const char* cmd);
int     GSM_WaitResponse(const char* expected, uint32_t timeout_ms);
int     GSM_SendPayload(const uint8_t* data, uint32_t len);
int     GSM_CheckNetwork(void);
void    GSM_PowerOff(void);
int     GSM_IsDataAvailable(void);
uint16_t GSM_GetRxCount(void);
void    GSM_ClearRxBuffer(void);

#ifdef __cplusplus
}
#endif

#endif /* __GSM_DRIVER_H */