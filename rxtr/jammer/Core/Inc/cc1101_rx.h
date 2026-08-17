/**
  ******************************************************************************
  * @file    cc1101_rx.h
  * @brief   CC1101 RECEIVER module - twin of the transmitter in main.c.
  *
  *          This module is configured to exactly match the CC1101 transmitter
  *          that main.c programs, so a second CC1101 wired to another MCU as a
  *          receiver can demodulate and decode those packets.
  *
  *          Matching link parameters (must be identical on RX and TX):
  *            - Carrier      : 433.92 MHz (FREQ 0x10/0xB0/0x71)
  *            - Data rate    : 38.4 kbps
  *            - Modulation   : 2-FSK (GFSK-style SmartRF settings)
  *            - Deviation    : ~20.6 kHz (DEVIATN 0x35)
  *            - Sync word    : default 0xD391 (neither side changes it)
  *            - Packet format: variable length (first FIFO byte = length),
  *                             CRC enabled, status bytes appended
  *                             (PKTCTRL0 = 0x05, PKTCTRL1 = 0x04)
  *            - Address check: disabled on TX, kept disabled here
  *
  *          The transmitter sends frames like:  [DEST][SRC][TYPE][payload...]
  *          with DEST = 11 (this node) and SRC = 22.  See CC1101RX_Receive.
  ******************************************************************************
  */
#ifndef __CC1101_RX_H__
#define __CC1101_RX_H__

#include "main.h"
#include <stdint.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/* Tunable constants                                                          */
/* -------------------------------------------------------------------------- */

/* This node is the intended destination of the transmitter (RADIO_DEST_ID 11). */
#define CC1101_RX_OWN_ID        11u

/* Largest payload the transmitter can send. The CC1101 FIFO holds 61 bytes,
   and each frame uses 1 length byte; keep this <= 60. */
#define CC1101_RX_MAX_PAYLOAD   60u

/* Maximum wait for the first FIFO byte of a frame (ms). */
#define CC1101_RX_TIMEOUT_MS    1000u

/* -------------------------------------------------------------------------- */
/* Result info attached to a received frame                                    */
/* -------------------------------------------------------------------------- */
typedef struct
{
  uint8_t  crc_ok;     /* 1 = CRC valid, 0 = bad/dropped                         */
  int8_t   rssi_dbm;   /* Received signal strength in dBm                        */
  uint8_t  lqi;        /* Raw link-quality indicator byte                        */
} CC1101_RxInfo;

/* -------------------------------------------------------------------------- */
/* API                                                                         */
/* -------------------------------------------------------------------------- */

/**
 * @brief  Configure the CC1101 as a receiver with settings identical to the
 *         transmitter in main.c and place it in RX mode.
 * @note   Call once after MX_SPI2_Init(); requires SPI2 pins (PA0/PA3/PA4)
 *         and CS (PA2, c1101_cs) configured.
 */
void CC1101RX_Init(void);

/**
 * @brief  Wait (up to timeout_ms) for a frame and, if one arrives, copy the
 *         payload (the bytes AFTER the CC1101 length byte) to out.
 *
 *         The payload delivered is exactly what the transmitter built, i.e.
 *           payload[0] = DEST, [1] = SRC, [2] = TYPE, [3..] = data.
 *         TYPE 0x01 = telemetry, TYPE 0x02 = log.
 *
 * @param  out       Buffer of at least CC1101_RX_MAX_PAYLOAD bytes.
 * @param  max_len   Size of 'out' (byte count that fits).
 * @param  info      Optional pointer to a CC1101_RxInfo to fill, or NULL.
 * @param  timeout_ms Maximum block time to wait (ms).
 * @retval payload length (> 0) on success, 0 on timeout/invalid frame.
 */
uint16_t CC1101RX_Receive(uint8_t *out, uint16_t max_len,
                          CC1101_RxInfo *info, uint16_t timeout_ms);

#endif /* __CC1101_RX_H__ */