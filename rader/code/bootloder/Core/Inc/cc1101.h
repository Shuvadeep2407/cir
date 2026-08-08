/**
  ******************************************************************************
  * @file    cc1101.h
  * @brief   CC1101 sub-GHz RF transceiver driver for STM32C092KCT
  *          Configured for 38.4 kbps, max TX power, hardware address filtering
  ******************************************************************************
  */

#ifndef __CC1101_H
#define __CC1101_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"

/*============================================================================*/
/* CC1101 SPI & GDO Pin Definitions                                           */
/*============================================================================*/
#define CC1101_CS_PORT          GPIOA
#define CC1101_CS_PIN           GPIO_PIN_2        /* PA2 - c1101_clk (CS)   */
#define CC1101_GDO0_PORT        GPIOA
#define CC1101_GDO0_PIN         GPIO_PIN_1        /* PA1 - DO0 (GDO0)       */
#define CC1101_GDO0_EXTI_IRQn   EXTI0_1_IRQn

/*============================================================================*/
/* CC1101 Register Addresses (Burst bit = 0x40, Read bit = 0x80)              */
/*============================================================================*/
#define CC1101_IOCFG2           0x00
#define CC1101_IOCFG1           0x01
#define CC1101_IOCFG0           0x06
#define CC1101_FIFOTHR          0x07
#define CC1101_PKTCTRL1         0x07
#define CC1101_PKTCTRL0         0x08
#define CC1101_FSCTRL1          0x09
#define CC1101_ADDR             0x09
#define CC1101_CHANNR           0x0A
#define CC1101_FREQ2            0x0D
#define CC1101_FREQ1            0x0E
#define CC1101_FREQ0            0x0F
#define CC1101_MDMCFG4          0x10
#define CC1101_MDMCFG3          0x11
#define CC1101_MDMCFG2          0x12
#define CC1101_MDMCFG1          0x13
#define CC1101_MDMCFG0          0x14
#define CC1101_DEVIATN          0x15
#define CC1101_MCSM0            0x18
#define CC1101_FOCCFG           0x19
#define CC1101_WORCTRL          0x1C
#define CC1101_FSCAL3           0x1D
#define CC1101_FSCAL2           0x1E
#define CC1101_FSCAL1           0x1F
#define CC1101_FSCAL0           0x20
#define CC1101_FSTEST           0x2B
#define CC1101_PTEST            0x2C
#define CC1101_AGCTEST          0x2D
#define CC1101_TEST2            0x2E
#define CC1101_TEST1            0x2F
#define CC1101_TEST0            0x30

/* Status registers (read-only) */
#define CC1101_PARTNUM          0x30
#define CC1101_VERSION          0x31
#define CC1101_FREQEST          0x32
#define CC1101_LQI              0x33
#define CC1101_RSSI             0x34
#define CC1101_MARCSTATE        0x35
#define CC1101_WORTIME1         0x36
#define CC1101_WORTIME0         0x37
#define CC1101_PKTSTATUS        0x38
#define CC1101_VCO_VC_DAC       0x39
#define CC1101_TXBYTES          0x3A
#define CC1101_RXBYTES          0x3B
#define CC1101_RCCTRL1_STATUS   0x3C
#define CC1101_RCCTRL0_STATUS   0x3D

/* FIFO access */
#define CC1101_TXFIFO           0x3F
#define CC1101_RXFIFO           0x3F

/* PATABLE (burst write only) */
#define CC1101_PATABLE          0x3E

/*============================================================================*/
/* CC1101 Command Strobes                                                     */
/*============================================================================*/
#define CC1101_SRES             0x30
#define CC1101_SFSTXON          0x31
#define CC1101_SXOFF            0x32
#define CC1101_SCAL             0x33
#define CC1101_SRX              0x34
#define CC1101_STX              0x35
#define CC1101_SIDLE            0x36
#define CC1101_SWOR             0x38
#define CC1101_SPWD             0x39
#define CC1101_SFRX             0x3A
#define CC1101_SFTX             0x3B
#define CC1101_SWORRST          0x3C
#define CC1101_SNOP             0x3D

/*============================================================================*/
/* CC1101 Configuration Values (38.4 kbps, 433 MHz, +10 dBm)                  */
/*============================================================================*/
#define CC1101_FREQ_433MHZ      0x10A7D8    /* 433.0 MHz */
#define CC1101_BAUD_38K4        0x0B        /* MDMCFG4: 38.4 kbps */

/* Broadcast address (used during provisioning) */
#define CC1101_ADDR_BROADCAST   0x00

/*============================================================================*/
/* CC1101 API Functions                                                       */
/*============================================================================*/

/* Low-level SPI access */
uint8_t  CC1101_ReadReg(uint8_t reg);
void     CC1101_WriteReg(uint8_t reg, uint8_t value);
void     CC1101_ReadBurst(uint8_t reg, uint8_t *buffer, uint8_t len);
void     CC1101_WriteBurst(uint8_t reg, uint8_t *buffer, uint8_t len);
void     CC1101_Strobe(uint8_t cmd);

/* High-level control */
void     CC1101_Init(void);
void     CC1101_SetAddress(uint8_t addr);
void     CC1101_SetChannel(uint8_t channel);
void     CC1101_SetPower(uint8_t pa_table_index);
void     CC1101_FlushRxFifo(void);
void     CC1101_FlushTxFifo(void);
void     CC1101_EnterRx(void);
void     CC1101_EnterTx(void);
void     CC1101_EnterIdle(void);

/* Packet operations */
uint8_t  CC1101_ReadRxFifo(uint8_t *buffer, uint8_t max_len);
void     CC1101_WriteTxFifo(uint8_t *buffer, uint8_t len);
uint8_t  CC1101_GetRxBytes(void);
uint8_t  CC1101_GetTxBytes(void);
uint8_t  CC1101_GetRssi(void);
uint8_t  CC1101_GetLqi(void);

/* GDO0 EXTI setup */
void     CC1101_EnableGdo0Interrupt(void);
void     CC1101_DisableGdo0Interrupt(void);

#ifdef __cplusplus
}
#endif

#endif /* __CC1101_H */