/**
  ******************************************************************************
  * @file    cc1101.c
  * @brief   CC1101 sub-GHz RF transceiver driver for STM32C092KCT
  *          Configured for 38.4 kbps, max TX power, hardware address filtering
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "cc1101.h"
#include "gpio.h"

/*============================================================================*/
/* Private Configuration Tables                                               */
/*============================================================================*/

/* PA table for +10 dBm output power (index 0 = max power) */
static const uint8_t cc1101_pa_table[8] = {
    0xC0,  /* +10 dBm */
    0xC5,  /* +7 dBm  */
    0xC8,  /* +5 dBm  */
    0xCD,  /* +2 dBm  */
    0xD0,  /* 0 dBm   */
    0xD5,  /* -3 dBm  */
    0xD8,  /* -6 dBm  */
    0xDD   /* -10 dBm */
};

/* Register configuration for 38.4 kbps, 433 MHz, GFSK, 250m range */
static const uint8_t cc1101_cfg[][2] = {
    /* IOCFG2: GDO2 = PA_PD (PA_EN) - high during TX, low during RX */
    {CC1101_IOCFG2,        0x2D},
    /* IOCFG1: GDO1 = LNA_PD (LNA_EN) - high during RX, low during TX */
    {CC1101_IOCFG1,        0x2C},
    /* IOCFG0: GDO0 = RX FIFO threshold / packet received (assert on RX) */
    {CC1101_IOCFG0,        0x06},
    /* FIFOTHR: TX threshold = 33, RX threshold = 32 */
    {CC1101_FIFOTHR,       0x07},
    /* PKTCTRL1: Address check = 2 (check address + broadcast), CRC auto flush */
    {CC1101_PKTCTRL1,      0x04},
    /* PKTCTRL0: Variable length, CRC enable, no whitening */
    {CC1101_PKTCTRL0,      0x05},
    /* FSCTRL1: Frequency synthesizer control */
    {CC1101_FSCTRL1,       0x06},
    /* FREQ2: 433.0 MHz carrier frequency */
    {CC1101_FREQ2,         0x10},
    {CC1101_FREQ1,         0xA7},
    {CC1101_FREQ0,         0xD8},
    /* MDMCFG4: 38.4 kbps data rate */
    {CC1101_MDMCFG4,       0x0B},
    /* MDMCFG3: 38.4 kbps data rate (mantissa) */
    {CC1101_MDMCFG3,       0x3B},
    /* MDMCFG2: GFSK, 16/16 sync word, no Manchester */
    {CC1101_MDMCFG2,       0x13},
    /* MDMCFG1: 2-byte preamble, no FEC */
    {CC1101_MDMCFG1,       0x02},
    /* MDMCFG0: Channel spacing = 199.95 kHz */
    {CC1101_MDMCFG0,       0xF8},
    /* DEVIATN: 20.6 kHz deviation */
    {CC1101_DEVIATN,       0x35},
    /* MCSM0: Auto-calibrate on idle->RX/TX, no pin control */
    {CC1101_MCSM0,         0x18},
    /* FOCCFG: Frequency offset compensation */
    {CC1101_FOCCFG,        0x16},
    /* WORCTRL: Wake-on-radio disabled */
    {CC1101_WORCTRL,       0xFB},
    /* FSCAL3: Frequency synthesizer calibration */
    {CC1101_FSCAL3,        0xE9},
    {CC1101_FSCAL2,        0x2A},
    {CC1101_FSCAL1,        0x00},
    {CC1101_FSCAL0,        0x1F},
    /* TEST2: Test settings */
    {CC1101_TEST2,         0x81},
    /* TEST1: Test settings */
    {CC1101_TEST1,         0x35},
    /* TEST0: Test settings */
    {CC1101_TEST0,         0x09},
};

/*============================================================================*/
/* Private Helpers                                                            */
/*============================================================================*/

static void CC1101_Select(void)
{
    HAL_GPIO_WritePin(CC1101_CS_PORT, CC1101_CS_PIN, GPIO_PIN_RESET);
}

static void CC1101_Deselect(void)
{
    HAL_GPIO_WritePin(CC1101_CS_PORT, CC1101_CS_PIN, GPIO_PIN_SET);
}

/*============================================================================*/
/* Low-Level SPI Access                                                       */
/*============================================================================*/

/**
  * @brief  Read a single configuration register
  */
uint8_t CC1101_ReadReg(uint8_t reg)
{
    uint8_t value = 0;
    uint8_t header = reg | 0x80;  /* Read bit */

    CC1101_Select();
    HAL_SPI_TransmitReceive(&hspi2, &header, &value, 1, HAL_MAX_DELAY);
    HAL_SPI_TransmitReceive(&hspi2, &value, &value, 1, HAL_MAX_DELAY);
    CC1101_Deselect();

    return value;
}

/**
  * @brief  Write a single configuration register
  */
void CC1101_WriteReg(uint8_t reg, uint8_t value)
{
    uint8_t header = reg;

    CC1101_Select();
    HAL_SPI_Transmit(&hspi2, &header, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi2, &value, 1, HAL_MAX_DELAY);
    CC1101_Deselect();
}

/**
  * @brief  Read multiple bytes from a register (burst mode)
  */
void CC1101_ReadBurst(uint8_t reg, uint8_t *buffer, uint8_t len)
{
    uint8_t header = reg | 0xC0;  /* Read + Burst bits */

    CC1101_Select();
    HAL_SPI_Transmit(&hspi2, &header, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, buffer, len, HAL_MAX_DELAY);
    CC1101_Deselect();
}

/**
  * @brief  Write multiple bytes to a register (burst mode)
  */
void CC1101_WriteBurst(uint8_t reg, uint8_t *buffer, uint8_t len)
{
    uint8_t header = reg | 0x40;  /* Burst bit */

    CC1101_Select();
    HAL_SPI_Transmit(&hspi2, &header, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi2, buffer, len, HAL_MAX_DELAY);
    CC1101_Deselect();
}

/**
  * @brief  Send a command strobe
  */
void CC1101_Strobe(uint8_t cmd)
{
    CC1101_Select();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);
    CC1101_Deselect();
}

/*============================================================================*/
/* High-Level Control                                                         */
/*============================================================================*/

/**
  * @brief  Initialize the CC1101 transceiver
  */
void CC1101_Init(void)
{
    uint8_t i;

    /* Configure CS pin as output */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = CC1101_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CC1101_CS_PORT, &GPIO_InitStruct);

    /* Configure GDO0 pin as input (EXTI) */
    GPIO_InitStruct.Pin = CC1101_GDO0_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CC1101_GDO0_PORT, &GPIO_InitStruct);

    /* Reset the CC1101 */
    CC1101_Deselect();
    HAL_Delay(10);
    CC1101_Select();
    HAL_Delay(10);
    CC1101_Deselect();
    HAL_Delay(10);
    CC1101_Strobe(CC1101_SRES);
    HAL_Delay(10);

    /* Write configuration registers */
    for (i = 0; i < sizeof(cc1101_cfg) / sizeof(cc1101_cfg[0]); i++)
    {
        CC1101_WriteReg(cc1101_cfg[i][0], cc1101_cfg[i][1]);
    }

    /* Set PA table for +10 dBm */
    CC1101_WriteBurst(CC1101_PATABLE, (uint8_t *)cc1101_pa_table, 8);

    /* Set default address to broadcast (will be changed during provisioning) */
    CC1101_SetAddress(CC1101_ADDR_BROADCAST);

    /* Flush FIFOs */
    CC1101_FlushRxFifo();
    CC1101_FlushTxFifo();

    /* Enter RX mode */
    CC1101_EnterRx();
}

/**
  * @brief  Set the CC1101 hardware address (ADDR register)
  */
void CC1101_SetAddress(uint8_t addr)
{
    CC1101_WriteReg(CC1101_ADDR, addr);
}

/**
  * @brief  Set the RF channel
  */
void CC1101_SetChannel(uint8_t channel)
{
    CC1101_WriteReg(CC1101_CHANNR, channel);
}

/**
  * @brief  Set TX power via PA table index (0 = max, 7 = min)
  */
void CC1101_SetPower(uint8_t pa_table_index)
{
    if (pa_table_index > 7)
    {
        pa_table_index = 0;
    }
    CC1101_WriteBurst(CC1101_PATABLE, (uint8_t *)&cc1101_pa_table[pa_table_index], 1);
}

/**
  * @brief  Flush the RX FIFO
  */
void CC1101_FlushRxFifo(void)
{
    CC1101_Strobe(CC1101_SIDLE);
    CC1101_Strobe(CC1101_SFRX);
}

/**
  * @brief  Flush the TX FIFO
  */
void CC1101_FlushTxFifo(void)
{
    CC1101_Strobe(CC1101_SIDLE);
    CC1101_Strobe(CC1101_SFTX);
}

/**
  * @brief  Enter RX mode
  */
void CC1101_EnterRx(void)
{
    CC1101_Strobe(CC1101_SRX);
}

/**
  * @brief  Enter TX mode
  */
void CC1101_EnterTx(void)
{
    CC1101_Strobe(CC1101_STX);
}

/**
  * @brief  Enter IDLE mode
  */
void CC1101_EnterIdle(void)
{
    CC1101_Strobe(CC1101_SIDLE);
}

/*============================================================================*/
/* Packet Operations                                                          */
/*============================================================================*/

/**
  * @brief  Read the RX FIFO (packet data)
  * @retval Number of bytes read
  */
uint8_t CC1101_ReadRxFifo(uint8_t *buffer, uint8_t max_len)
{
    uint8_t len = 0;
    uint8_t rx_bytes = CC1101_GetRxBytes();

    if (rx_bytes > max_len)
    {
        rx_bytes = max_len;
    }

    if (rx_bytes > 0)
    {
        CC1101_ReadBurst(CC1101_RXFIFO, buffer, rx_bytes);
        len = rx_bytes;
    }

    return len;
}

/**
  * @brief  Write packet data to the TX FIFO
  */
void CC1101_WriteTxFifo(uint8_t *buffer, uint8_t len)
{
    CC1101_WriteBurst(CC1101_TXFIFO, buffer, len);
}

/**
  * @brief  Get number of bytes in RX FIFO
  */
uint8_t CC1101_GetRxBytes(void)
{
    return CC1101_ReadReg(CC1101_RXBYTES) & 0x7F;
}

/**
  * @brief  Get number of bytes in TX FIFO
  */
uint8_t CC1101_GetTxBytes(void)
{
    return CC1101_ReadReg(CC1101_TXBYTES) & 0x7F;
}

/**
  * @brief  Get RSSI value (dBm = RSSI/2 - 74)
  */
uint8_t CC1101_GetRssi(void)
{
    return CC1101_ReadReg(CC1101_RSSI);
}

/**
  * @brief  Get LQI value
  */
uint8_t CC1101_GetLqi(void)
{
    return CC1101_ReadReg(CC1101_LQI);
}

/*============================================================================*/
/* GDO0 Interrupt                                                             */
/*============================================================================*/

/**
  * @brief  Enable GDO0 EXTI interrupt
  */
void CC1101_EnableGdo0Interrupt(void)
{
    HAL_NVIC_SetPriority(CC1101_GDO0_EXTI_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(CC1101_GDO0_EXTI_IRQn);
}

/**
  * @brief  Disable GDO0 EXTI interrupt
  */
void CC1101_DisableGdo0Interrupt(void)
{
    HAL_NVIC_DisableIRQ(CC1101_GDO0_EXTI_IRQn);
}