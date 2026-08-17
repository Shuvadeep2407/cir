/*
 * w5500.c
 * 
 *  Created on: May 20, 2024
 *      Author: Gemini Code Assist
 */

#include "w5500.h"

// W5500 can use SPI1 for bench testing or SPI3 for main operation.
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi3;

#define W5500_VERSIONR_ADDR 0x0039u
#define W5500_VERSIONR_VALUE 0x04u

static W5500_SpiBus_t s_w5500_spi_bus = W5500_SPI_BUS_3;

static SPI_HandleTypeDef *W5500_GetSpiHandle(void) {
    return (s_w5500_spi_bus == W5500_SPI_BUS_3) ? &hspi3 : &hspi1;
}

static void W5500_Select(void) {
    HAL_GPIO_WritePin(w5500_cs_GPIO_Port, w5500_cs_Pin, GPIO_PIN_RESET);
}

static void W5500_Deselect(void) {
    HAL_GPIO_WritePin(w5500_cs_GPIO_Port, w5500_cs_Pin, GPIO_PIN_SET);
}

void W5500_Init(void) {
    W5500_Deselect();

}

void W5500_SetSpiBus(W5500_SpiBus_t bus) {
    W5500_Deselect();
    if (bus == W5500_SPI_BUS_3) {
        s_w5500_spi_bus = W5500_SPI_BUS_3;
    } else {
        s_w5500_spi_bus = W5500_SPI_BUS_1;
    }
}

W5500_SpiBus_t W5500_GetSpiBus(void) {
    return s_w5500_spi_bus;
}

const char *W5500_GetSpiBusName(void) {
    return (s_w5500_spi_bus == W5500_SPI_BUS_3) ? "SPI3" : "SPI1";
}

uint8_t W5500_TestConnection(void) {
    return (W5500_ReadByte(W5500_VERSIONR_ADDR) == W5500_VERSIONR_VALUE) ? 1u : 0u;
}

uint8_t W5500_ReadByte(uint16_t addr) {
    uint8_t tx_data[4];
    uint8_t ret; 
    
    tx_data[0] = (addr & 0xFF00) >> 8; // Address high byte
    tx_data[1] = (addr & 0x00FF);      // Address low byte
    tx_data[2] = 0x00; // Read operation, 1 byte length 
    tx_data[3] = 0x01;

    W5500_Select();
    HAL_SPI_Transmit(W5500_GetSpiHandle(), tx_data, 3, 100); // Send address and control phase
    HAL_SPI_Receive(W5500_GetSpiHandle(), &ret, 1, 100);     // Read data
    W5500_Deselect();

    return ret;
}

void W5500_WriteByte(uint16_t addr, uint8_t data) {
    uint8_t tx_data[4];

    tx_data[0] = (addr & 0xFF00) >> 8; // Address high byte 
    tx_data[1] = (addr & 0x00FF);      // Address low byte
    tx_data[2] = 0x04; // Common register block, write, variable length mode
    tx_data[3] = 0x01;

    W5500_Select();
    HAL_SPI_Transmit(W5500_GetSpiHandle(), tx_data, 3, 100);
    HAL_SPI_Transmit(W5500_GetSpiHandle(), &data, 1, 100);
    W5500_Deselect();
}

void W5500_ReadBurst(uint16_t addr, uint8_t* p_data, uint16_t len) {
    uint8_t tx_header[3];
    tx_header[0] = (addr & 0xFF00) >> 8;
    tx_header[1] = (addr & 0x00FF);
    tx_header[2] = 0x00; // Read operation, variable length 

    W5500_Select();
    HAL_SPI_Transmit(W5500_GetSpiHandle(), tx_header, 3, 100);
    HAL_SPI_Receive(W5500_GetSpiHandle(), p_data, len, 100);
    W5500_Deselect();
}

void W5500_WriteBurst(uint16_t addr, uint8_t* p_data, uint16_t len) {
    uint8_t tx_header[3];
    tx_header[0] = (addr & 0xFF00) >> 8;
    tx_header[1] = (addr & 0x00FF);
    tx_header[2] = 0x04; // Common register block, write, variable length mode

    W5500_Select();
    HAL_SPI_Transmit(W5500_GetSpiHandle(), tx_header, 3, 100);
    HAL_SPI_Transmit(W5500_GetSpiHandle(), p_data, len, 100);
    W5500_Deselect();
}
