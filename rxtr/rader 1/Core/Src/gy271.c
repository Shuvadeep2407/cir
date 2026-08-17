/**
  ******************************************************************************
  * @file    gy271.c
  * @brief   HMC5883L driver (GY-271 module) using blocking I2C1 transfers.
  ******************************************************************************
  */
#include "gy271.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1;

static uint8_t WriteReg(uint8_t reg, uint8_t val)
{
  return (HAL_I2C_Mem_Write(&hi2c1, HMC5883L_ADDR, reg,
                            I2C_MEMADD_SIZE_8BIT, &val, 1, 100) == HAL_OK);
}

/**
  * @brief Configure the HMC5883L: 8-sample avg / 15 Hz normal, 1.3 gauss gain,
  *        continuous measurement mode.
  */
void HMC5883L_Init(void)
{
  (void)WriteReg(HMC5883L_REG_CRA,  0x70u);  /* 8 samples, 15 Hz, normal bias */
  HAL_Delay(6);
  (void)WriteReg(HMC5883L_REG_CRB,  0x20u);  /* gain 1.3 gauss (1090 LSB/gauss) */
  HAL_Delay(6);
  (void)WriteReg(HMC5883L_REG_MODE, 0x00u);  /* continuous measurement mode */
  HAL_Delay(10);
}

/**
  * @brief Verify the sensor by reading the three identification registers.
  * @retval 1 if all match the expected 'H', '4', '3', otherwise 0.
  */
uint8_t HMC5883L_Test(void)
{
  uint8_t id[3] = {0};
  if (HAL_I2C_Mem_Read(&hi2c1, HMC5883L_ADDR, HMC5883L_REG_IDA,
                       I2C_MEMADD_SIZE_8BIT, id, 3, 100) != HAL_OK)
  {
    return 0;
  }
  return (id[0] == 0x48u && id[1] == 0x34u && id[2] == 0x33u); /* 'H','4','3' */
}

/**
  * @brief Read the three-axis field strength values.
  *        Data register order is: X MSB, X LSB, Z MSB, Z LSB, Y MSB, Y LSB.
  */
void HMC5883L_ReadXYZ(int16_t *x, int16_t *y, int16_t *z)
{
  uint8_t d[6] = {0};
  if (HAL_I2C_Mem_Read(&hi2c1, HMC5883L_ADDR, HMC5883L_REG_DATA,
                       I2C_MEMADD_SIZE_8BIT, d, 6, 100) == HAL_OK)
  {
    *x = (int16_t)((d[0] << 8) | d[1]);
    *z = (int16_t)((d[2] << 8) | d[3]);
    *y = (int16_t)((d[4] << 8) | d[5]);
  }
  else
  {
    *x = *y = *z = 0;
  }
}
