/*
 * w5500.h
 * 
 *  Created on: May 20, 2024
 *      Author: Gemini Code Assist
 *
 *  Basic hardware abstraction for W5500 Ethernet controller.
 *  This is NOT a full TCP/IP stack, but provides the low-level SPI functions
 *  needed to integrate a driver like Wiznet's ioLibrary.
 */

#ifndef INC_W5500_H_
#define INC_W5500_H_

#include "main.h"

typedef enum
{
    W5500_SPI_BUS_1 = 1,
    W5500_SPI_BUS_3 = 3
} W5500_SpiBus_t;

/**
 * @brief Initializes the W5500 chip.
 */ 
void W5500_Init(void);

void W5500_SetSpiBus(W5500_SpiBus_t bus);
W5500_SpiBus_t W5500_GetSpiBus(void);
const char *W5500_GetSpiBusName(void);

/**
 * @brief Checks SPI communication by reading the W5500 version register.
 * @return 1 if the W5500 responds with version 0x04, otherwise 0.
 */
uint8_t W5500_TestConnection(void);
HAL_StatusTypeDef W5500_ReadByteChecked(uint16_t addr, uint8_t *data);

/**
 * @brief Writes a single byte to a W5500 register.
 * @param addr Register address.
 * @param data Byte to write.
 */
void W5500_WriteByte(uint16_t addr, uint8_t data); 

/**
 * @brief Reads a single byte from a W5500 register.
 * @param addr Register address.
 * @return Byte read from the register.
 */
uint8_t W5500_ReadByte(uint16_t addr);
 
void W5500_WriteBurst(uint16_t addr, uint8_t* p_data, uint16_t len);
void W5500_ReadBurst(uint16_t addr, uint8_t* p_data, uint16_t len);


#endif /* INC_W5500_H_ */
