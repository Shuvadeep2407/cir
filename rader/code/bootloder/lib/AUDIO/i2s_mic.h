/**
  ******************************************************************************
  * @file    i2s_mic.h
  * @brief   Dual I2S Digital MEMS Microphone Array Library
  *
  *          Microphone 1 (Left channel): Primary environmental sound
  *          Microphone 2 (Right channel): Ambient noise reference
  *
  *          Uses I2S1 in Master RX stereo mode with DMA circular buffer.
  *          DMA Half/Full transfer interrupts trigger audio processing
  *          to compute RMS energy levels for low-bandwidth RF transmission.
  ******************************************************************************
  */

#ifndef __I2S_MIC_H
#define __I2S_MIC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2s.h"

/*============================================================================*/
/* I2S Microphone Configuration                                               */
/*============================================================================*/
#define I2S_MIC_SAMPLE_RATE     32000   /* I2S sample rate (32 kHz) */
#define I2S_MIC_DATA_BITS       16      /* 16-bit PCM data */

/* DMA circular buffer size (stereo = 2 channels per frame) */
#define I2S_MIC_BUFFER_SIZE     4096    /* Bytes (2048 frames stereo) */
/* STM32C0: DMA1 Channel 3 uses the shared DMA1_Channel2_3_IRQn */
#define I2S_MIC_DMA_CHANNEL     DMA1_Channel2_3_IRQn
#define I2S_MIC_DMA_IRQHandler  DMA1_Channel2_3_IRQHandler

/* Blocks per buffer (1 = half transfer, 2 = full transfer) */
#define I2S_MIC_BLOCKS_PER_BUFFER 2

/*============================================================================*/
/* Audio Processing Result                                                    */
/*============================================================================*/
typedef struct
{
    uint16_t mic1_rms;      /* RMS level of mic 1 (primary) 0-1000 */
    uint16_t mic2_rms;      /* RMS level of mic 2 (noise ref) 0-1000 */
    uint8_t  audio_valid;   /* 1 = new audio data available */
    uint8_t  block_index;   /* Which DMA block was processed (0 or 1) */
} I2S_Mic_Result_t;

/*============================================================================*/
/* API Functions                                                              */
/*============================================================================*/

/* Initialize I2S for dual microphone capture with DMA circular buffer */
uint8_t I2S_Mic_Init(void);

/* Start continuous DMA audio capture */
void I2S_Mic_Start(void);

/* Stop DMA audio capture */
void I2S_Mic_Stop(void);

/* Process audio block (called from DMA half/full transfer interrupt) */
void I2S_Mic_ProcessBlock(uint8_t block_index);

/* Get the latest compressed audio results */
uint8_t I2S_Mic_GetResult(I2S_Mic_Result_t *result);

/* Compute RMS of a 16-bit PCM block */
uint16_t I2S_Mic_ComputeRms(int16_t *samples, uint16_t count);

/* DMA interrupt handler (called from DMA1_Channel2_3_IRQHandler) */
void I2S_Mic_DmaHandler(void);

/* Global result structure for main loop access */
extern volatile I2S_Mic_Result_t i2s_mic_result;

#ifdef __cplusplus
}
#endif

#endif /* __I2S_MIC_H */