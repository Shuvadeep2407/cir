/**
  ******************************************************************************
  * @file    spi_radio_driver.c
  * @brief   CC1101 SPI Radio Driver - LL-based SPI2, 24Mbps
  ******************************************************************************
  */
#include "bootloader.h"
#include "spi_radio_driver.h"
#include "blink_driver.h"
#include "stm32c0xx.h"
#include "stm32c092_compat.h"

/**
  * @brief  SPI2 transmit/receive byte
  */
static uint8_t SPI2_Transfer(uint8_t tx)
{
    while (!(SPI2->SR & SPI_SR_TXE));
    *((volatile uint8_t*)&SPI2->DR) = tx;
    while (!(SPI2->SR & SPI_SR_RXNE));
    return (uint8_t)SPI2->DR;
}

/**
  * @brief  Send command strobe to CC1101
  */
static void radio_cmd(uint8_t cmd)
{
    SPI2_Transfer(cmd);
}

/**
  * @brief  Initialize SPI2 and CC1101 radio
  */
void SPI_Radio_Init(void)
{
    /* Enable clocks */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR1 |= RCC_APBENR1_SPI2EN;
    
    /* Configure PA0(SPI2_SCK) AF0, PA3(SPI2_MISO) AF0, PA4(SPI2_MOSI) AF3 */
    GPIOA->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE3 | GPIO_MODER_MODE4);
    GPIOA->MODER |= (GPIO_MODER_AF_MODE0 | GPIO_MODER_AF_MODE3 | GPIO_MODER_AF_MODE4);
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL0 | GPIO_AFRL_AFSEL3 | GPIO_AFRL_AFSEL4);
    GPIOA->AFR[0] |= (3 << (4 * 4));  /* PA4 = AF3 */
    
    /* Configure SPI2: Master, 24MHz, 8-bit */
    SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_BR_0 | SPI_CR1_SPE;
    SPI2->CR2 = 0;
    
    /* Reset CC1101 */
    radio_cmd(CC1101_SRES);
    Delay_ms(1);
    
    /* Configure CC1101 for 433MHz, 38.4kbps, GFSK */
    SPI_Radio_WriteReg(CC1101_IOCFG2, 0x06);
    SPI_Radio_WriteReg(CC1101_IOCFG0, 0x06);
    SPI_Radio_WriteReg(CC1101_PKTLEN, 0x3D);
    SPI_Radio_WriteReg(CC1101_PKTCTRL1, 0x04);
    SPI_Radio_WriteReg(CC1101_PKTCTRL0, 0x32);
    SPI_Radio_WriteReg(CC1101_ADDR, 0x00);
    SPI_Radio_WriteReg(CC1101_CHANNR, 0x00);
    SPI_Radio_WriteReg(CC1101_FREQ2, 0x10);
    SPI_Radio_WriteReg(CC1101_FREQ1, 0xB0);
    SPI_Radio_WriteReg(CC1101_FREQ0, 0x71);
    SPI_Radio_WriteReg(CC1101_MDMCFG4, 0xCA);
    SPI_Radio_WriteReg(CC1101_MDMCFG3, 0x83);
    SPI_Radio_WriteReg(CC1101_MDMCFG2, 0x93);
    SPI_Radio_WriteReg(CC1101_MDMCFG1, 0x22);
    SPI_Radio_WriteReg(CC1101_MDMCFG0, 0xF8);
    SPI_Radio_WriteReg(CC1101_DEVIATN, 0x34);
    SPI_Radio_WriteReg(CC1101_MCSM0, 0x18);
    SPI_Radio_WriteReg(CC1101_FOCCFG, 0x16);
    SPI_Radio_WriteReg(CC1101_FREND0, 0x10);
    
    radio_cmd(CC1101_SRX);
}

int SPI_Radio_WriteReg(uint8_t reg, uint8_t value)
{
    SPI2_Transfer(reg & 0x3F);
    SPI2_Transfer(value);
    return RADIO_OK;
}

int SPI_Radio_ReadReg(uint8_t reg, uint8_t* value)
{
    SPI2_Transfer(reg | 0x80);
    *value = SPI2_Transfer(0);
    return RADIO_OK;
}

int SPI_Radio_SendPacket(const uint8_t* data, uint8_t len)
{
    if (len > RADIO_MAX_PACKET) return RADIO_ERROR;
    radio_cmd(CC1101_SFTX);
    radio_cmd(CC1101_STX);
    SPI2_Transfer(len);
    for (uint8_t i = 0; i < len; i++) SPI2_Transfer(data[i]);
    Delay_ms(10);
    radio_cmd(CC1101_SIDLE);
    return RADIO_OK;
}

int SPI_Radio_ReceivePacket(uint8_t* data, uint8_t* len, uint32_t timeout_ms)
{
    uint32_t start = GetTick();
    uint8_t status;
    while ((GetTick() - start) < timeout_ms)
    {
        SPI_Radio_ReadReg(0x3B, &status);
        if (status & 0x7F)
        {
            radio_cmd(CC1101_SFRX);
            radio_cmd(CC1101_SRX);
            *len = SPI2_Transfer(0);
            if (*len > RADIO_MAX_PACKET) *len = RADIO_MAX_PACKET;
            for (uint8_t i = 0; i < *len; i++) data[i] = SPI2_Transfer(0);
            SPI2_Transfer(0); SPI2_Transfer(0);
            return RADIO_OK;
        }
    }
    return RADIO_TIMEOUT;
}

void SPI_Radio_SetFrequency(uint32_t freq_hz)
{
    uint32_t f = (freq_hz * 65536) / 26000000;
    SPI_Radio_WriteReg(CC1101_FREQ2, (f >> 16) & 0xFF);
    SPI_Radio_WriteReg(CC1101_FREQ1, (f >> 8) & 0xFF);
    SPI_Radio_WriteReg(CC1101_FREQ0, f & 0xFF);
}

int SPI_Radio_CheckChannel(void)
{
    uint8_t rssi;
    SPI_Radio_ReadReg(0x34, &rssi);
    return (rssi < 0xC0) ? RADIO_OK : RADIO_ERROR;
}

void SPI_Radio_SetPower(uint8_t dbm)
{
    uint8_t pa = 0x10;
    if (dbm >= 10) pa = 0xC0;
    else if (dbm >= 5) pa = 0x60;
    else if (dbm >= 0) pa = 0x10;
    else pa = 0x03;
    SPI_Radio_WriteReg(CC1101_FREND0, pa);
}

void SPI_Radio_Sleep(void)
{
    radio_cmd(CC1101_SIDLE);
    radio_cmd(CC1101_SRES);
}

void SPI_Radio_Wake(void)
{
    radio_cmd(CC1101_SRX);
}
