/**
  ******************************************************************************
  * @file    i2s_audio_driver.h
  * @brief   I2S Audio Driver Header - PA15(WS), PB3(CK), PB5(SD), Master TX
  ******************************************************************************
  */
#ifndef __I2S_AUDIO_DRIVER_H
#define __I2S_AUDIO_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Default audio configuration */
#define AUDIO_DEFAULT_SAMPLE_RATE   32000
#define AUDIO_DEFAULT_BITS          16
#define AUDIO_DMA_BUFFER_SIZE       512

void    I2S_Audio_Init(uint32_t sample_rate);
void    I2S_Audio_StartDMA(uint16_t* buffer, uint16_t size);
void    I2S_Audio_StopDMA(void);
void    I2S_Audio_SetVolume(uint8_t volume);
int     I2S_Audio_IsPlaying(void);
void    I2S_Audio_Write(uint16_t sample);
void    I2S_Audio_Mute(uint8_t mute);
void    I2S_Audio_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __I2S_AUDIO_DRIVER_H */