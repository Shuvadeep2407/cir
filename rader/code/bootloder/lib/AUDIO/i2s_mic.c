/**
  ******************************************************************************
  * @file    i2s_mic.c
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

/* Includes ------------------------------------------------------------------*/
#include "i2s_mic.h"
#include <string.h>

/*============================================================================*/
/* Private Helpers                                                            */
/*============================================================================*/

/**
  * @brief  Fast integer square root (no libm dependency)
  * @param  n: Value to take sqrt of
  * @retval Integer square root
  */
static uint32_t I2S_Mic_ISqrt(uint32_t n)
{
    uint32_t res = 0;
    uint32_t bit = 1UL << 30;

    /* Find the highest power of 4 <= n */
    while (bit > n)
    {
        bit >>= 2;
    }

    while (bit != 0)
    {
        if (n >= res + bit)
        {
            n -= res + bit;
            res = (res >> 1) + bit;
        }
        else
        {
            res >>= 1;
        }
        bit >>= 2;
    }

    return res;
}

/*============================================================================*/
/* Private Variables                                                          */
/*============================================================================*/

/* DMA circular buffer for stereo I2S data (L = mic1, R = mic2) */
static int16_t i2s_dma_buf[I2S_MIC_BUFFER_SIZE / 2];

volatile I2S_Mic_Result_t i2s_mic_result;

/*============================================================================*/
/* RMS Computation                                                            */
/*============================================================================*/

/**
  * @brief  Compute RMS of a 16-bit PCM sample block
  * @param  samples: Pointer to sample array
  * @param  count: Number of samples
  * @retval RMS value scaled to 0-1000 range
  */
uint16_t I2S_Mic_ComputeRms(int16_t *samples, uint16_t count)
{
    uint32_t sum = 0;
    uint16_t i;
    uint16_t mean;
    uint32_t rms;

    for (i = 0; i < count; i++)
    {
        sum += (uint32_t)(samples[i] * samples[i]);
    }

    mean = (uint16_t)(sum / count);
    rms = I2S_Mic_ISqrt(mean);

    /* Scale to 0-1000 range (max 16-bit RMS = 32767 -> 1000) */
    if (rms > 32767)
    {
        rms = 32767;
    }
    return (uint16_t)((rms * 1000) / 32767);
}

/*============================================================================*/
/* Audio Processing                                                           */
/*============================================================================*/

/**
  * @brief  Process a DMA audio block (half or full transfer)
  * @param  block_index: 0 = first half, 1 = second half
  */
void I2S_Mic_ProcessBlock(uint8_t block_index)
{
    uint16_t block_size = I2S_MIC_BUFFER_SIZE / (2 * I2S_MIC_BLOCKS_PER_BUFFER);
    int16_t *block_start = &i2s_dma_buf[block_index * block_size];
    int16_t mic1[512];
    int16_t mic2[512];
    uint16_t frame_count = block_size / 2;
    uint16_t i;

    /* Deinterleave stereo data: even samples = mic1 (L), odd = mic2 (R) */
    if (frame_count > 512)
    {
        frame_count = 512;
    }

    for (i = 0; i < frame_count; i++)
    {
        mic1[i] = block_start[i * 2];
        mic2[i] = block_start[i * 2 + 1];
    }

    /* Compute RMS for both channels */
    i2s_mic_result.mic1_rms = I2S_Mic_ComputeRms(mic1, frame_count);
    i2s_mic_result.mic2_rms = I2S_Mic_ComputeRms(mic2, frame_count);
    i2s_mic_result.block_index = block_index;
    i2s_mic_result.audio_valid = 1;
}

/*============================================================================*/
/* DMA Interrupt Handler                                                      */
/*============================================================================*/

/**
  * @brief  DMA interrupt handler (called from DMA1_Channel2_3_IRQHandler)
  */
void I2S_Mic_DmaHandler(void)
{
    /* Half transfer complete (DMA1_Channel3) */
    if (__HAL_DMA_GET_IT_SOURCE(&hdma_i2s1_rx, DMA_IT_HT) &&
        __HAL_DMA_GET_FLAG(&hdma_i2s1_rx, DMA_FLAG_HT3))
    {
        __HAL_DMA_CLEAR_FLAG(&hdma_i2s1_rx, DMA_FLAG_HT3);
        I2S_Mic_ProcessBlock(0);
    }

    /* Full transfer complete (DMA1_Channel3) */
    if (__HAL_DMA_GET_IT_SOURCE(&hdma_i2s1_rx, DMA_IT_TC) &&
        __HAL_DMA_GET_FLAG(&hdma_i2s1_rx, DMA_FLAG_TC3))
    {
        __HAL_DMA_CLEAR_FLAG(&hdma_i2s1_rx, DMA_FLAG_TC3);
        I2S_Mic_ProcessBlock(1);
    }

    /* Transfer error (DMA1_Channel3) */
    if (__HAL_DMA_GET_IT_SOURCE(&hdma_i2s1_rx, DMA_IT_TE) &&
        __HAL_DMA_GET_FLAG(&hdma_i2s1_rx, DMA_FLAG_TE3))
    {
        __HAL_DMA_CLEAR_FLAG(&hdma_i2s1_rx, DMA_FLAG_TE3);
    }
}

/*============================================================================*/
/* Public API                                                                 */
/*============================================================================*/

/**
  * @brief  Initialize I2S for dual microphone capture with DMA
  * @retval 1 on success, 0 on failure
  */
uint8_t I2S_Mic_Init(void)
{
    /* Clear result structure */
    memset((void *)&i2s_mic_result, 0, sizeof(I2S_Mic_Result_t));
    memset(i2s_dma_buf, 0, sizeof(i2s_dma_buf));

    /* Re-initialize I2S1 for Master RX mode (stereo) */
    hi2s1.Instance = SPI1;
    hi2s1.Init.Mode = I2S_MODE_MASTER_RX;
    hi2s1.Init.Standard = I2S_STANDARD_PHILIPS;
    hi2s1.Init.DataFormat = I2S_DATAFORMAT_16B;
    hi2s1.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
    hi2s1.Init.AudioFreq = I2S_AUDIOFREQ_32K;
    hi2s1.Init.CPOL = I2S_CPOL_HIGH;

    if (HAL_I2S_Init(&hi2s1) != HAL_OK)
    {
        return 0;
    }

    /* Enable DMA RX on I2S1 */
    if (HAL_I2S_Receive_DMA(&hi2s1, (uint16_t *)i2s_dma_buf, I2S_MIC_BUFFER_SIZE / 2) != HAL_OK)
    {
        return 0;
    }

    /* Enable DMA half/full transfer interrupts */
    __HAL_DMA_ENABLE_IT(&hdma_i2s1_rx, DMA_IT_HT);
    __HAL_DMA_ENABLE_IT(&hdma_i2s1_rx, DMA_IT_TC);
    __HAL_DMA_ENABLE_IT(&hdma_i2s1_rx, DMA_IT_TE);

    /* Enable DMA interrupt in NVIC */
    HAL_NVIC_SetPriority(I2S_MIC_DMA_CHANNEL, 2, 0);
    HAL_NVIC_EnableIRQ(I2S_MIC_DMA_CHANNEL);

    return 1;
}

/**
  * @brief  Start continuous DMA audio capture
  */
void I2S_Mic_Start(void)
{
    HAL_I2S_Receive_DMA(&hi2s1, (uint16_t *)i2s_dma_buf, I2S_MIC_BUFFER_SIZE / 2);
}

/**
  * @brief  Stop DMA audio capture
  */
void I2S_Mic_Stop(void)
{
    HAL_I2S_DMAStop(&hi2s1);
}

/**
  * @brief  Get the latest compressed audio results
  * @param  result: Output result structure
  * @retval 1 if new data available, 0 otherwise
  */
uint8_t I2S_Mic_GetResult(I2S_Mic_Result_t *result)
{
    if (result == NULL)
    {
        return 0;
    }

    if (i2s_mic_result.audio_valid)
    {
        result->mic1_rms = i2s_mic_result.mic1_rms;
        result->mic2_rms = i2s_mic_result.mic2_rms;
        result->block_index = i2s_mic_result.block_index;
        result->audio_valid = i2s_mic_result.audio_valid;
        i2s_mic_result.audio_valid = 0;
        return 1;
    }

    return 0;
}