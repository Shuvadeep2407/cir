/**
  ******************************************************************************
  * @file    i2c_driver.h
  * @brief   I2C Bus Driver Header - PB9(SDA), PB8(SCL)
  ******************************************************************************
  */
#ifndef __I2C_DRIVER_H
#define __I2C_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define I2C_DEFAULT_FREQ        100000
#define I2C_TIMEOUT_MS          100

void    I2C_Init(uint32_t frequency);
int     I2C_Write(uint16_t dev_addr, const uint8_t* data, uint16_t len);
int     I2C_Read(uint16_t dev_addr, uint8_t* data, uint16_t len);
int     I2C_WriteReg(uint16_t dev_addr, uint8_t reg, const uint8_t* data, uint16_t len);
int     I2C_ReadReg(uint16_t dev_addr, uint8_t reg, uint8_t* data, uint16_t len);
int     I2C_IsDeviceReady(uint16_t dev_addr, uint32_t timeout_ms);
void    I2C_ScanBus(void (*callback)(uint8_t addr));

#ifdef __cplusplus
}
#endif

#endif /* __I2C_DRIVER_H */