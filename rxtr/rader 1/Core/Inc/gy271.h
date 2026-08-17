/**
  ******************************************************************************
  * @file    gy271.h
  * @brief   Driver for the GY-271 board (HMC5883L 3-axis magnetometer) over I2C1.
  ******************************************************************************
  */
#ifndef __GY271_H__
#define __GY271_H__

#include "main.h"

/* HMC5883L 7-bit I2C address shifted left (HAL 8-bit format, 7-bit addressing) */
#define HMC5883L_ADDR      (0x1Eu << 1)

/* Register map */
#define HMC5883L_REG_CRA    0x00u  /* Configuration register A (avg/data-rate)   */
#define HMC5883L_REG_CRB    0x01u  /* Configuration register B (gain)            */
#define HMC5883L_REG_MODE   0x02u  /* Mode register (continuous / single / idle) */
#define HMC5883L_REG_DATA   0x03u  /* XYZ data start (6 bytes)                   */
#define HMC5883L_REG_STATUS 0x09u  /* Status register                            */
#define HMC5883L_REG_IDA    0x0Au  /* Identification A ('H')                     */
#define HMC5883L_REG_IDB    0x0Bu  /* Identification B ('4')                     */
#define HMC5883L_REG_IDC    0x0Cu  /* Identification C ('3')                     */

void HMC5883L_Init(void);                          /* configure sensor */
uint8_t HMC5883L_Test(void);                       /* 1 = valid HMC5883L detected */
void HMC5883L_ReadXYZ(int16_t *x, int16_t *y, int16_t *z);

#endif /* __GY271_H__ */