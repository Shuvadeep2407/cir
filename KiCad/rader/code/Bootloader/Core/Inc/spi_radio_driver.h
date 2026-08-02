/**
  ******************************************************************************
  * @file    spi_radio_driver.h
  * @brief   CC1101 SPI Radio Driver Header - PA0(SCK), PA3(MISO), PA4(MOSI)
  ******************************************************************************
  */
#ifndef __SPI_RADIO_DRIVER_H
#define __SPI_RADIO_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* CC1101 Register addresses */
#define CC1101_IOCFG2           0x00
#define CC1101_IOCFG1           0x01
#define CC1101_IOCFG0           0x02
#define CC1101_FIFOTHR          0x03
#define CC1101_SYNC1            0x04
#define CC1101_SYNC0            0x05
#define CC1101_PKTLEN           0x06
#define CC1101_PKTCTRL1         0x07
#define CC1101_PKTCTRL0         0x08
#define CC1101_ADDR             0x09
#define CC1101_CHANNR           0x0A
#define CC1101_FSCTRL1          0x0B
#define CC1101_FSCTRL0          0x0C
#define CC1101_FREQ2            0x0D
#define CC1101_FREQ1            0x0E
#define CC1101_FREQ0            0x0F
#define CC1101_MDMCFG4          0x10
#define CC1101_MDMCFG3          0x11
#define CC1101_MDMCFG2          0x12
#define CC1101_MDMCFG1          0x13
#define CC1101_MDMCFG0          0x14
#define CC1101_DEVIATN          0x15
#define CC1101_MCSM2            0x16
#define CC1101_MCSM1            0x17
#define CC1101_MCSM0            0x18
#define CC1101_FOCCFG           0x19
#define CC1101_BSCFG            0x1A
#define CC1101_AGCCTRL2         0x1B
#define CC1101_AGCCTRL1         0x1C
#define CC1101_AGCCTRL0         0x1D
#define CC1101_WOREVT1          0x1E
#define CC1101_WOREVT0          0x1F
#define CC1101_WORCTRL          0x20
#define CC1101_FREND1           0x21
#define CC1101_FREND0           0x22
#define CC1101_FSCAL3           0x23
#define CC1101_FSCAL2           0x24
#define CC1101_FSCAL1           0x25
#define CC1101_FSCAL0           0x26
#define CC1101_RCCTRL1          0x27
#define CC1101_RCCTRL0          0x28
#define CC1101_FSTEST           0x29
#define CC1101_PTEST            0x2A
#define CC1101_AGCTEST          0x2B
#define CC1101_TEST2            0x2C
#define CC1101_TEST1            0x2D
#define CC1101_TEST0            0x2E

/* CC1101 Command strobes */
#define CC1101_SRES             0x30
#define CC1101_SFSTXON          0x31
#define CC1101_SIDLE            0x33
#define CC1101_SRX              0x34
#define CC1101_STX              0x35
#define CC1101_SFRX             0x3A
#define CC1101_SFTX             0x3B

/* Packet length */
#define RADIO_MAX_PACKET        64

/* Radio states */
#define RADIO_IDLE              0
#define RADIO_RX                1
#define RADIO_TX                2

/* Return codes */
#define RADIO_OK                0
#define RADIO_ERROR             -1
#define RADIO_TIMEOUT           -2
#define RADIO_CRC_ERR           -3

void    SPI_Radio_Init(void);
int     SPI_Radio_WriteReg(uint8_t reg, uint8_t value);
int     SPI_Radio_ReadReg(uint8_t reg, uint8_t* value);
int     SPI_Radio_SendPacket(const uint8_t* data, uint8_t len);
int     SPI_Radio_ReceivePacket(uint8_t* data, uint8_t* len, uint32_t timeout_ms);
void    SPI_Radio_SetFrequency(uint32_t freq_hz);
int     SPI_Radio_CheckChannel(void);
void    SPI_Radio_SetPower(uint8_t dbm);
void    SPI_Radio_Sleep(void);
void    SPI_Radio_Wake(void);

#ifdef __cplusplus
}
#endif

#endif /* __SPI_RADIO_DRIVER_H */