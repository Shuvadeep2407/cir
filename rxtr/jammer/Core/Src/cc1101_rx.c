/**
  ******************************************************************************
  * @file    cc1101_rx.c
  * @brief   CC1101 receiver implementation (mirrors transmitter in main.c).
  *
  *          The register values here are intentionally the SAME as the ones
  *          main.c programs on the transmitter, so the two radios speak an
  *          identical air-protocol. Only the receive-enable flow (SRX / FIFO
  *          read / CRC check) differs from the transmit flow.
  ******************************************************************************
  */
#include "cc1101_rx.h"
#include "spi.h"

/* MISO is PA3 (SPI2, see spi.c); asserted low = CC1101 ready for an access. */
#define CC1101_MISO_PORT    GPIOA
#define CC1101_MISO_PIN     GPIO_PIN_3
#define CC1101_MISO_READY   GPIO_PIN_RESET

/* -------------------------------------------------------------------------- */
/* Command strobes (write-only, LSB = 0 -> command)                            */
/* -------------------------------------------------------------------------- */
#define CC1101_SRES         0x30u
#define CC1101_STX          0x35u
#define CC1101_SRX          0x34u
#define CC1101_SIDLE        0x36u
#define CC1101_SFRX         0x3Au

/* -------------------------------------------------------------------------- */
/* SPI header flags (bit 7 = read, bit 6 = burst)                              */
/* -------------------------------------------------------------------------- */
#define CC1101_WRITE_BURST  0x40u
#define CC1101_READ_SINGLE  0x80u
#define CC1101_READ_BURST   0xC0u

/* -------------------------------------------------------------------------- */
/* Configuration registers used on the receiver                                */
/* -------------------------------------------------------------------------- */
#define CC1101_IOCFG0       0x02u
#define CC1101_FIFOTHR      0x03u
#define CC1101_SYNC1        0x04u
#define CC1101_SYNC0        0x05u
#define CC1101_PKTLEN       0x06u
#define CC1101_PKTCTRL1     0x07u
#define CC1101_PKTCTRL0     0x08u
#define CC1101_ADDR         0x09u
#define CC1101_CHANNR       0x0Au
#define CC1101_FSCTRL1      0x0Bu
#define CC1101_FREQ2        0x0Du
#define CC1101_FREQ1        0x0Eu
#define CC1101_FREQ0        0x0Fu
#define CC1101_MDMCFG4      0x10u
#define CC1101_MDMCFG3      0x11u
#define CC1101_MDMCFG2      0x12u
#define CC1101_MDMCFG1      0x13u
#define CC1101_MDMCFG0      0x14u
#define CC1101_DEVIATN      0x15u
#define CC1101_MCSM0        0x18u
#define CC1101_FOCCFG       0x19u
#define CC1101_BSCFG        0x1Au
#define CC1101_AGCCTRL2     0x1Bu
#define CC1101_AGCCTRL1     0x1Cu
#define CC1101_AGCCTRL0     0x1Du
#define CC1101_FREND1       0x21u
#define CC1101_FREND0       0x22u
#define CC1101_FSCAL3       0x23u
#define CC1101_FSCAL2       0x24u
#define CC1101_FSCAL1       0x25u
#define CC1101_FSCAL0       0x26u
#define CC1101_TEST2        0x2Cu
#define CC1101_TEST1        0x2Du
#define CC1101_TEST0        0x2Eu
#define CC1101_PATABLE      0x3Eu
#define CC1101_RXFIFO       0x3Fu

/* -------------------------------------------------------------------------- */
/* Status registers (read-only)                                                */
/* -------------------------------------------------------------------------- */
#define CC1101_STAT_PKTSTATUS 0x38u
#define CC1101_STAT_RXBYTES   0x3Bu

/* Buffer to stage a payload plus its 2 appended status bytes. */
#define CC1101_FIFO_OVERFLOW   0x80u

/* -------------------------------------------------------------------------- */
/* Low-level SPI helpers (same protocol as the transmitter in main.c).         */
/* -------------------------------------------------------------------------- */

/** @brief Wait until the CC1101 drops MISO = low, indicating it is ready. */
static uint8_t CC1101_WaitReady(void)
{
  uint32_t tickstart = HAL_GetTick();

  while (HAL_GPIO_ReadPin(CC1101_MISO_PORT, CC1101_MISO_PIN) != CC1101_MISO_READY)
  {
    if ((HAL_GetTick() - tickstart) > 10u) /* ~10 ms timeout */
    {
      return 0u; /* not ready */
    }
  }
  return 1u;
}

/**
 * @brief Write a single 8-bit configuration register.
 */
static void CC1101_WriteReg(uint8_t addr, uint8_t value)
{
  uint8_t tx[2] = { addr, value };

  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
  if (CC1101_WaitReady())
  {
    (void)HAL_SPI_Transmit(&hspi2, tx, 2, 100);
  }
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

/**
 * @brief Burst-write the PATABLE (single-byte on common CC1101 modules).
 */
static void CC1101_WritePaTable(uint8_t pa0)
{
  uint8_t header = CC1101_PATABLE | CC1101_WRITE_BURST; /* 0x7E */

  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
  if (CC1101_WaitReady())
  {
    (void)HAL_SPI_Transmit(&hspi2, &header, 1, 100);
    (void)HAL_SPI_Transmit(&hspi2, &pa0, 1, 100);
  }
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

/**
 * @brief Send a command strobe (e.g. SRES, SRX, SIDLE).
 */
static void CC1101_Strobe(uint8_t strobe)
{
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
  if (CC1101_WaitReady())
  {
    (void)HAL_SPI_Transmit(&hspi2, &strobe, 1, 100);
  }
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}

/**
 * @brief Read a single configuration or status register.
 */
static uint8_t CC1101_ReadReg(uint8_t addr)
{
  uint8_t header = addr | CC1101_READ_SINGLE;
  uint8_t rx = 0xFF;

  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
  if (CC1101_WaitReady())
  {
    (void)HAL_SPI_Transmit(&hspi2, &header, 1, 100);
    (void)HAL_SPI_Receive(&hspi2, &rx, 1, 100);
  }
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
  return rx;
}

/**
 * @brief Read a single byte from the receive FIFO (0x3F read-single = 0xBF).
 */
static uint8_t CC1101_ReadFifoByte(void)
{
  return CC1101_ReadReg(CC1101_RXFIFO); /* header becomes 0xBF */
}

/**
 * @brief Burst-read len bytes from the RX FIFO (0x3F read-burst = 0xFF).
 */
static void CC1101_ReadFifoBurst(uint8_t *buf, uint8_t len)
{
  uint8_t header = CC1101_RXFIFO | CC1101_READ_BURST; /* 0xFF */

  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET);
  if (CC1101_WaitReady())
  {
    (void)HAL_SPI_Transmit(&hspi2, &header, 1, 100);
    (void)HAL_SPI_Receive(&hspi2, buf, len, 100);
  }
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);
}
/* -------------------------------------------------------------------------- */
/* Public API                                                                  */
/* -------------------------------------------------------------------------- */

/**
 * @brief  Hardware reset of the CC1101, mirroring the user's cc1101_reset():
 *         a chip-enable pulse (CS high->low->high) followed by the SRES strobe.
 * @note   After SRES the radio is in IDLE with all registers at reset values.
 */
static void CC1101RX_Reset(void)
{
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);   /* deselect */
  HAL_Delay(1);
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_RESET); /* select   */
  HAL_Delay(1);
  HAL_GPIO_WritePin(c1101_cs_GPIO_Port, c1101_cs_Pin, GPIO_PIN_SET);   /* deselect */
  HAL_Delay(1);
  CC1101_Strobe(CC1101_SRES);   /* SRES command strobe */
  HAL_Delay(1);
}

/**
 * @brief  Program the CC1101 as a receiver using the configuration supplied by
 *         the user's receiver snippet (cc1101_init): 433.92 MHz, 38.4 kbps,
 *         2-FSK, variable-length packets with CRC, then enter continuous RX.
 */
void CC1101RX_Init(void)
{
  /* Put the radio in a known state and flush the RX FIFO. */
  CC1101RX_Reset();
  CC1101_Strobe(CC1101_SFRX);

  /* ---- RF / modem: EXACTLY the supplied receiver config ---- */
  CC1101_WriteReg(CC1101_FSCTRL1,  0x06u);
  CC1101_WriteReg(CC1101_FREQ2,    0x10u);
  CC1101_WriteReg(CC1101_FREQ1,    0xB0u);
  CC1101_WriteReg(CC1101_FREQ0,    0x71u);

  CC1101_WriteReg(CC1101_MDMCFG4,  0xCAu);
  CC1101_WriteReg(CC1101_MDMCFG3,  0x83u);
  CC1101_WriteReg(CC1101_MDMCFG2,  0x13u);
  CC1101_WriteReg(CC1101_MDMCFG1,  0x22u);   /* as supplied by receiver config */
  CC1101_WriteReg(CC1101_MDMCFG0,  0xF8u);
  CC1101_WriteReg(CC1101_CHANNR,   0x00u);

  CC1101_WriteReg(CC1101_DEVIATN,  0x35u);

  CC1101_WriteReg(CC1101_FREND1,   0x56u);
  CC1101_WriteReg(CC1101_FREND0,   0x10u);
  CC1101_WriteReg(CC1101_MCSM0,    0x18u);
  CC1101_WriteReg(CC1101_FOCCFG,   0x16u);
  CC1101_WriteReg(CC1101_BSCFG,    0x6Cu);

  CC1101_WriteReg(CC1101_AGCCTRL2, 0x43u);
  CC1101_WriteReg(CC1101_AGCCTRL1, 0x40u);
  CC1101_WriteReg(CC1101_AGCCTRL0, 0x91u);

  CC1101_WriteReg(CC1101_FSCAL3,   0xE9u);
  CC1101_WriteReg(CC1101_FSCAL2,   0x2Au);
  CC1101_WriteReg(CC1101_FSCAL1,   0x00u);
  CC1101_WriteReg(CC1101_FSCAL0,   0x1Fu);

  CC1101_WriteReg(CC1101_TEST2,    0x81u);
  CC1101_WriteReg(CC1101_TEST1,    0x35u);
  CC1101_WriteReg(CC1101_TEST0,    0x09u);

  /* ---- Packet automation: variable length, CRC on, status bytes appended. */
  CC1101_WriteReg(CC1101_PKTCTRL1, 0x04u);
  CC1101_WriteReg(CC1101_PKTCTRL0, 0x05u);
  CC1101_WriteReg(CC1101_ADDR,     0x00u);
  CC1101_WriteReg(CC1101_PKTLEN,   0xFFu);   /* as supplied (max length 0xFF) */

  /* The supplied receiver config does NOT write SYNC1/SYNC0, so the sync word
     keeps its reset default 0xD391 - the transmitter in main.c also leaves it
     unset, so the two radios still share the same sync word. */

  /* PATABLE only sets TX power (unused while listening); kept for symmetry. */
  CC1101_WritePaTable(0xC0u);

  /* Flush any residual FIFO data and actually start receiving. */
  CC1101_Strobe(CC1101_SFRX);
  CC1101_Strobe(CC1101_SRX);
}

/**
 * @brief  Convert a raw CC1101 RSSI byte to dBm.
 * @note   Formula: dBm = (RSSI / 2) - 74, with RSSI treated as signed 8-bit.
 */
static int8_t CC1101_RssiToDbm(uint8_t raw)
{
  int16_t rssi = (int16_t)raw;

  if (rssi >= 128)
  {
    rssi -= 256;
  }
  return (int8_t)((rssi / 2) - 74);
}

/**
 * @brief  Wait for a frame and return its payload.
 * @see    CC1101RX_Receive() in the header.
 */
uint16_t CC1101RX_Receive(uint8_t *out, uint16_t max_len,
                          CC1101_RxInfo *info, uint16_t timeout_ms)
{
  uint32_t start = HAL_GetTick();
  uint8_t  plen;
  uint8_t  i;
  uint8_t  rxbytes;

  if (out == NULL || max_len == 0u)
  {
    return 0u;
  }

  /* Make sure the radio is listening on every call. */
  CC1101_Strobe(CC1101_SRX);

  /* ---- 1) Wait until the FIFO holds the length byte. */
  for (;;)
  {
    rxbytes = CC1101_ReadReg(CC1101_STAT_RXBYTES); /* header 0xBB */

    if (rxbytes & CC1101_FIFO_OVERFLOW)
    {
      CC1101_Strobe(CC1101_SFRX);   /* overflow: drop and keep listening */
      rxbytes = 0u;
    }

    if ((rxbytes & 0x7Fu) > 0u)
    {
      break;
    }

    if ((uint16_t)(HAL_GetTick() - start) >= timeout_ms)
    {
      return 0u; /* nothing arrived */
    }
  }

  /* ---- 2) Read the variable-length byte (first FIFO byte). */
  plen = CC1101_ReadFifoByte();

  if (plen == 0u || plen > CC1101_RX_MAX_PAYLOAD)
  {
    CC1101_Strobe(CC1101_SFRX);
    return 0u; /* invalid length */
  }

  /* ---- 3) Wait for the rest: plen payload + 2 appended status bytes. */
  for (;;)
  {
    rxbytes = CC1101_ReadReg(CC1101_STAT_RXBYTES);

    if (rxbytes & CC1101_FIFO_OVERFLOW)
    {
      CC1101_Strobe(CC1101_SFRX);
      return 0u;
    }

    if ((rxbytes & 0x7Fu) >= (plen + 2u))
    {
      break;
    }

    if ((uint16_t)(HAL_GetTick() - start) >= timeout_ms)
    {
      CC1101_Strobe(CC1101_SFRX);
      return 0u;
    }
  }

  /* ---- 4) Read payload + 2 appended status bytes (RSSI, LQI). */
  {
    uint8_t staging[CC1101_RX_MAX_PAYLOAD + 2u];

    CC1101_ReadFifoBurst(staging, (uint8_t)(plen + 2u));

    for (i = 0u; i < plen; i++)
    {
      if (i < max_len)
      {
        out[i] = staging[i];
      }
    }

    if (info != NULL)
    {
      info->rssi_dbm = CC1101_RssiToDbm(staging[plen]);
      info->lqi      = staging[plen + 1u];
      /* CRC_OK is bit 0 of the PKTSTATUS status register (header 0xB8). */
      info->crc_ok   = (CC1101_ReadReg(CC1101_STAT_PKTSTATUS) & 0x01u) ? 1u : 0u;
    }
  }

  /* ---- 5) Flush what is left so the next frame starts clean. */
  CC1101_Strobe(CC1101_SFRX);

  return (plen > max_len) ? max_len : plen;
}