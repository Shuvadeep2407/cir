/**
  ******************************************************************************
  * @file    blink_driver.h
  * @brief   Software PWM / Timing Utilities for Bootloader
  ******************************************************************************
  */
#ifndef __BLINK_DRIVER_H
#define __BLINK_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Timing */
void    Delay_ms(uint32_t ms);
uint32_t GetTick(void);
void    Tick_Init(void);
void    Tick_Increment(void);

/* Watchdog */
void    Watchdog_Init(uint32_t timeout_ms);
void    Watchdog_Refresh(void);

/* CRC32 calculation */
uint32_t CRC32_Calculate(const uint8_t* data, uint32_t length);
uint32_t CRC32_CalculateFlash(uint32_t address, uint32_t length);

/* Flash utilities (LL based) */
int     Flash_Unlock(void);
int     Flash_Lock(void);
int     Flash_ErasePage(uint32_t address);
int     Flash_Write(uint32_t address, const uint8_t* data, uint32_t len);
int     Flash_Read(uint32_t address, uint8_t* data, uint32_t len);

/* NVM configuration storage */
int     NVM_WriteConfig(uint16_t id, const uint8_t* data, uint16_t len);
int     NVM_ReadConfig(uint16_t id, uint8_t* data, uint16_t len);

/* Application jump */
void    JumpToApplication(uint32_t app_address);
void    System_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* __BLINK_DRIVER_H */