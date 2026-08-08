/**
  ******************************************************************************
  * @file    i2s_audio_driver.c
  * @brief   I2S Audio Driver - LL-based, PA15(WS) PB3(CK) PB5(SD), Master TX
  ******************************************************************************
  */
#include "bootloader.h"
#include "i2s_audio_driver.h"
#include "stm32c0xx.h"
#include "stm32c092_compat.h"

void I2S_Audio_Init(uint32_t sample_rate)
{
    (void)sample_rate;
    
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;
    RCC->APBENR2 |= RCC_APBENR2_SPI1EN;
    
    /* PA15(I2S1_WS) AF0, PB3(I2S1_CK) AF0, PB5(I2S1_SD) AF0 */
    GPIOA->MODER &= ~GPIO_MODER_MODE15;
    GPIOA->MODER |= GPIO_MODER_AF_MODE15;
    GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL15;
    
    GPIOB->MODER &= ~(GPIO_MODER_MODE3 | GPIO_MODER_MODE5);
    GPIOB->MODER |= (GPIO_MODER_AF_MODE3 | GPIO_MODER_AF_MODE5);
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL3 | GPIO_AFRL_AFSEL5);
    
    /* Configure SPI1 as I2S Master TX, Philips, 16-bit, 32kHz */
    SPI1->I2SCFGR = SPI_I2SCFGR_I2SMOD |
                    (2 << SPI_I2SCFGR_I2SCFG_Pos) |  /* Master TX */
                    SPI_I2SCFGR_I2SE;
    
    /* Set prescaler for 32kHz: 48MHz / (2*32kHz*32) = ~23.4 */
    SPI1->I2SPR = (23 << SPI_I2SPR_MCKOE_Pos) | SPI_I2SPR_I2SDIV_0;
    
    SPI1->I2SCFGR |= SPI_I2SCFGR_I2SE;
}

void I2S_Audio_StartDMA(uint16_t* buffer, uint16_t size)
{
    (void)buffer;
    (void)size;
}

void I2S_Audio_StopDMA(void) { }

void I2S_Audio_SetVolume(uint8_t volume) { (void)volume; }

int I2S_Audio_IsPlaying(void) { return 0; }

void I2S_Audio_Write(uint16_t sample)
{
    while (!(SPI1->SR & SPI_SR_TXE));
    *((volatile uint16_t*)&SPI1->DR) = sample;
}

void I2S_Audio_Mute(uint8_t mute) { (void)mute; }

void I2S_Audio_DeInit(void)
{
    SPI1->I2SCFGR = 0;
    RCC->APBENR2 &= ~RCC_APBENR2_SPI1EN;
}
