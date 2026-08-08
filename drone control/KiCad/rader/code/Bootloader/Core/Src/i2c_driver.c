/**
  ******************************************************************************
  * @file    i2c_driver.c
  * @brief   I2C Bus Driver - LL-based, PB9(SDA) PB8(SCL), 100kHz
  ******************************************************************************
  */
#include "bootloader.h"
#include "i2c_driver.h"
#include "stm32c0xx.h"
#include "stm32c092_compat.h"
#include "blink_driver.h"
#include "stm32c0xx_hal_rcc.h" /* For HAL_RCC_GetPCLK1Freq */

/* I2C TIMING register value for different frequencies @48MHz PCLK1 */
/* This value can be calculated using the STM32CubeMX I2C timing tool */
#define I2C_TIMING_100KHZ   0x10805D88
#define I2C_TIMING_400KHZ   0x00801D2D

void I2C_Init(uint32_t frequency)
{
    uint32_t timing_reg = I2C_TIMING_100KHZ;
    
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    RCC->APBENR1 |= RCC_APBENR1_I2C1EN;
    
    /* PB8(SCL) AF6, PB9(SDA) AF6 - Open drain */
    GPIOB->MODER &= ~(GPIO_MODER_MODE8 | GPIO_MODER_MODE9);
    GPIOB->MODER |= (GPIO_MODER_AF_MODE8 | GPIO_MODER_AF_MODE9);
    GPIOB->OTYPER |= (GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9);
    GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL8 | GPIO_AFRH_AFSEL9);
    GPIOB->AFR[1] |= (GPIO_AF6_I2C1 << GPIO_AFRH_AFSEL8_Pos) | (GPIO_AF6_I2C1 << GPIO_AFRH_AFSEL9_Pos);  /* AF6 for PB8 and PB9 */
    
    /* Reset I2C1 */
    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 = 0;
    
    /* Configure timing */
    if (frequency == 400000)
    {
        timing_reg = I2C_TIMING_400KHZ;
    }
    I2C1->TIMINGR = timing_reg;

    I2C1->CR1 = I2C_CR1_PE;
}

int I2C_Write(uint16_t dev_addr, const uint8_t* data, uint16_t len)
{
    uint32_t tickstart = GetTick();

    I2C1->CR2 = (dev_addr << 1) | I2C_CR2_START | I2C_CR2_AUTOEND | (len << 16);
    for (uint16_t i = 0; i < len; i++)
    {
        while (!(I2C1->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)))
        {
            if ((GetTick() - tickstart) > I2C_TIMEOUT_MS) return -1;
        }
        if (I2C1->ISR & I2C_ISR_NACKF)
        {
            I2C1->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;
            return -1;
        }
        I2C1->TXDR = data[i];
    }
    while (!(I2C1->ISR & I2C_ISR_STOPF))
    {
        if ((GetTick() - tickstart) > I2C_TIMEOUT_MS) return -1;
    }
    I2C1->ICR = I2C_ICR_STOPCF;
    return 0;
}

int I2C_Read(uint16_t dev_addr, uint8_t* data, uint16_t len)
{
    uint32_t tickstart = GetTick();
    uint16_t bytes_read = 0;

    while (bytes_read < len)
    {
        uint16_t chunk_len = (len - bytes_read > 255) ? 255 : (len - bytes_read);
        uint32_t cr2_val = (dev_addr << 1) | I2C_CR2_RD_WRN | (chunk_len << 16);
        if (bytes_read == 0) {
            cr2_val |= I2C_CR2_START;
        }
        if ((len - bytes_read) <= 255) {
            cr2_val |= I2C_CR2_AUTOEND;
        }
        I2C1->CR2 = cr2_val;

        for (uint16_t i = 0; i < chunk_len; i++)
        {
            while (!(I2C1->ISR & I2C_ISR_RXNE))
            {
                if ((GetTick() - tickstart) > I2C_TIMEOUT_MS) return -1;
            }
            data[bytes_read++] = (uint8_t)I2C1->RXDR;
        }
    }
    while (!(I2C1->ISR & I2C_ISR_STOPF))
    {
        if ((GetTick() - tickstart) > I2C_TIMEOUT_MS) return -1;
    }
    I2C1->ICR = I2C_ICR_STOPCF;
    return 0;
}

int I2C_WriteReg(uint16_t dev_addr, uint8_t reg, const uint8_t* data, uint16_t len)
{
    uint32_t tickstart = GetTick();

    I2C1->CR2 = (dev_addr << 1) | I2C_CR2_START | I2C_CR2_AUTOEND | ((len + 1) << 16);
    while (!(I2C1->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)))
    {
        if ((GetTick() - tickstart) > I2C_TIMEOUT_MS) return -1;
    }
    if (I2C1->ISR & I2C_ISR_NACKF)
    {
        I2C1->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;
        return -1;
    }
    I2C1->TXDR = reg;
    for (uint16_t i = 0; i < len; i++)
    {
        while (!(I2C1->ISR & I2C_ISR_TXIS))
        {
            if ((GetTick() - tickstart) > I2C_TIMEOUT_MS) return -1;
        }
        I2C1->TXDR = data[i];
    }
    while (!(I2C1->ISR & I2C_ISR_STOPF))
    {
        if ((GetTick() - tickstart) > I2C_TIMEOUT_MS) return -1;
    }
    I2C1->ICR = I2C_ICR_STOPCF;
    return 0;
}

int I2C_ReadReg(uint16_t dev_addr, uint8_t reg, uint8_t* data, uint16_t len)
{
    uint32_t tickstart = GetTick();
    
    /* --- First, write the register address to the device --- */
    /* Configure for write, 1 byte (the register address), and do not auto-end */
    I2C1->CR2 = (dev_addr << 1) | (1 << 16) | I2C_CR2_START;
    
    while (!(I2C1->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)))
    {
        if ((GetTick() - tickstart) > I2C_TIMEOUT_MS) return -1;
    }
    
    if (I2C1->ISR & I2C_ISR_NACKF)
    {
        I2C1->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;
        return -1;
    }
    
    I2C1->TXDR = reg;
    
    /* Wait for the transfer to complete */
    while (!(I2C1->ISR & I2C_ISR_TC))
    {
        if ((GetTick() - tickstart) > I2C_TIMEOUT_MS) return -1;
    }

    /* --- Second, read the data from that register --- */
    /* Configure for read, 'len' bytes, with a repeated start and auto-end */
    tickstart = GetTick();
    I2C_Read(dev_addr, data, len);

    /* Wait for the final stop condition */
    while (!(I2C1->ISR & I2C_ISR_STOPF))
    {
        if ((GetTick() - tickstart) > I2C_TIMEOUT_MS) return -1;
    }
    I2C1->ICR = I2C_ICR_STOPCF;
    return 0;
}

int I2C_IsDeviceReady(uint16_t dev_addr, uint32_t timeout_ms)
{
    uint32_t start = GetTick();
    while ((GetTick() - start) < timeout_ms)
    {
        I2C1->CR2 = (dev_addr << 1) | I2C_CR2_START | I2C_CR2_AUTOEND;
        uint32_t wait = 1000;
        while (wait-- && !(I2C1->ISR & (I2C_ISR_STOPF | I2C_ISR_NACKF)));
        if (I2C1->ISR & I2C_ISR_NACKF)
        {
            I2C1->ICR = I2C_ICR_NACKCF;
            continue;
        }
        I2C1->ICR = I2C_ICR_STOPCF;
        return 0;
    }
    return -1;
}

void I2C_ScanBus(void (*callback)(uint8_t addr))
{
    for (uint8_t addr = 1; addr < 127; addr++)
    {
        if (I2C_IsDeviceReady(addr, 10) == 0)
        {
            if (callback) callback(addr);
        }
    }
}
