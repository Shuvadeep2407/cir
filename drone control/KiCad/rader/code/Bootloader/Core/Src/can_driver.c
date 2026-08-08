/**
  ******************************************************************************
  * @file    can_driver.c
  * @brief   FDCAN Driver - LL-based, PB0(RX) PB1(TX), 1Mbps
  ******************************************************************************
  */
#include "bootloader.h"
#include "can_driver.h"
#include "stm32c0xx.h"
#include "stm32c092_compat.h"
#include "blink_driver.h"

#define CAN_RXFIFO0_RAM_OFFSET   176U
#define CAN_TXFIFO_RAM_OFFSET    632U
#define CAN_MESSAGE_STRIDE       72U
#define CAN_MESSAGE_RAM          ((volatile uint32_t *)SRAMCAN_BASE)

/**
  * @brief  Initialize FDCAN1
  * @param  bitrate: Bitrate in bps (500000 for LRR220PRO radar, 1000000 for OTA)
  */
void CAN_Init(uint32_t bitrate)
{
    uint32_t prescaler, seg1, seg2, sjw;
    
    /* Enable clocks */
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    RCC->APBENR1 |= RCC_APBENR1_FDCAN1EN;
    
    /* Configure PB0(FDCAN1_RX) AF3, PB1(FDCAN1_TX) AF3 */
    GPIOB->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1);
    GPIOB->MODER |= (GPIO_MODER_AF_MODE0 | GPIO_MODER_AF_MODE1);
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL0 | GPIO_AFRL_AFSEL1);
    GPIOB->AFR[0] |= (3 << (0 * 4)) | (3 << (1 * 4));  /* AF3 */
    
    /* Initialize FDCAN1 */
    FDCAN1->CCCR = FDCAN_CCCR_CSR;  /* Enter init mode */
    while (!(FDCAN1->CCCR & FDCAN_CCCR_INIT));
    
    /* Calculate bit timing for desired bitrate @ 48MHz PCLK */
    /* FDCAN clock = 48MHz. Bit time = (prescaler * (seg1+seg2+1)) */
    /* Target: 87.5% sample point for 1Mbps, 83.3% for 500kbps */
    if (bitrate >= 1000000)
    {
        /* 1Mbps: 48MHz / (3 * (1 + 10 + 5)) = 1Mbps */
        prescaler = 3;
        seg1 = 10;
        seg2 = 5;
        sjw = 5;
    }
    else if (bitrate >= 500000)
    {
        /* 500kbps: 48MHz / (6 * (1 + 10 + 5)) = 500kbps */
        prescaler = 6;
        seg1 = 10;
        seg2 = 5;
        sjw = 5;
    }
    else if (bitrate >= 250000)
    {
        /* 250kbps: 48MHz / (12 * (1 + 10 + 5)) = 250kbps */
        prescaler = 12;
        seg1 = 10;
        seg2 = 5;
        sjw = 5;
    }
    else
    {
        /* Default to 500kbps */
        prescaler = 6;
        seg1 = 10;
        seg2 = 5;
        sjw = 5;
    }
    
    /* Configure Nominal Bit Timing & Prescaler */
    FDCAN1->NBTP = ((sjw - 1) << FDCAN_NBTP_NSJW_Pos) |
                   ((prescaler - 1) << FDCAN_NBTP_NBRP_Pos) |
                   ((seg1 - 1) << FDCAN_NBTP_NTSEG1_Pos) |
                   ((seg2 - 1) << FDCAN_NBTP_NTSEG2_Pos);
    
    /* Configure data bit timing (same as nominal for classic CAN) */
    FDCAN1->DBTP = FDCAN1->NBTP;
    
    /* Configure global filter: accept all */
    FDCAN1->RXGFC = FDCAN_RXGFC_ANFE | FDCAN_RXGFC_ANFS;
    
    /* Set TX FIFO queue mode */
    FDCAN1->TXBC = 0;
    FDCAN1->TXFQS = 0;
    
    /* Clear any pending interrupts */
    FDCAN1->IR = 0xFFFFFFFF;
    
    /* Enable interrupts */
    FDCAN1->IE = FDCAN_IE_RF0NE | FDCAN_IE_RF1NE | FDCAN_IE_BOE;
    
    /* Exit init mode and enter normal operation */
    FDCAN1->CCCR &= ~FDCAN_CCCR_CSR;
    while (FDCAN1->CCCR & FDCAN_CCCR_INIT);
    
    /* Enable NVIC interrupts */
    NVIC_SetPriority(FDCAN1_IT0_IRQn, 0);
    NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    NVIC_SetPriority(FDCAN1_IT1_IRQn, 0);
    NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
}

/**
  * @brief  Transmit CAN message
  * @param  id: CAN ID (11-bit or 29-bit)
  * @param  data: Data payload (max 8 bytes)
  * @param  len: Data length
  * @param  is_extended: 0=standard, 1=extended
  * @retval CAN_OK on success
  */
int CAN_Transmit(uint32_t id, const uint8_t* data, uint8_t len, int is_extended)
{
    /* Wait for TX buffer available */
    uint32_t timeout = 10000;
    uint32_t buf_idx;
    
    do {
        buf_idx = FDCAN1->TXFQS & FDCAN_TXFQS_TFQF;
        if (--timeout == 0) return CAN_TIMEOUT;
    } while (buf_idx);
    
    /* Get next free TX buffer index */
    uint32_t tx_idx = (FDCAN1->TXFQS & FDCAN_TXFQS_TFQPI) >> FDCAN_TXFQS_TFQPI_Pos;
    
    /* Write message to the fixed STM32C092 Tx FIFO message RAM. */
    volatile uint32_t *tx_buf = CAN_MESSAGE_RAM +
                                (CAN_TXFIFO_RAM_OFFSET / 4U) +
                                (tx_idx * (CAN_MESSAGE_STRIDE / 4U));
    tx_buf[0] = is_extended ? (id & 0x1FFFFFFFU)
                            : ((id & 0x7FFU) << 18U);
    tx_buf[1] = ((uint32_t)len << 16U);
    
    /* Write data */
    for (uint8_t i = 0; i < 4 && i < len; i++)
        ((uint8_t*)&tx_buf[2])[i] = data[i];
    for (uint8_t i = 0; i < 4 && (i + 4) < len; i++)
        ((uint8_t*)&tx_buf[3])[i] = data[i + 4];
    
    /* Add to TX queue */
    FDCAN1->TXBAR = (1 << tx_idx);
    
    return CAN_OK;
}

/**
  * @brief  Receive CAN message with timeout
  * @param  id: Output CAN ID
  * @param  data: Output data buffer
  * @param  len: Output data length
  * @param  timeout_ms: Timeout
  * @retval CAN_OK on success
  */
int CAN_Receive(uint32_t* id, uint8_t* data, uint8_t* len, uint32_t timeout_ms)
{
    uint32_t start = GetTick();
    
    while ((GetTick() - start) < timeout_ms)
    {
        /* Check RX FIFO 0 */
        if (FDCAN1->RXF0S & FDCAN_RXF0S_F0FL)
        {
            uint32_t rx_idx = (FDCAN1->RXF0S & FDCAN_RXF0S_F0GI) >> FDCAN_RXF0S_F0GI_Pos;
            volatile uint32_t *rx_buf = CAN_MESSAGE_RAM +
                                        (CAN_RXFIFO0_RAM_OFFSET / 4U) +
                                        (rx_idx * (CAN_MESSAGE_STRIDE / 4U));
            
            /* Parse header */
            uint32_t r0 = rx_buf[0];
            uint32_t r1 = rx_buf[1];

            if (r0 & (1U << 30U)) { // XTD bit for extended ID
                *id = r0 & 0x1FFFFFFFU;
            } else { // Standard ID
                *id = (r0 >> 18U) & 0x7FFU;
            }
            
            *len = (r1 >> 16U) & 0xFU;
            if (*len > 8) {
                *len = 8;
            }
            
            /* Copy data */
            uint8_t* pData = (uint8_t*)&rx_buf[2];
            for (uint8_t i = 0; i < *len; i++)
            {
                data[i] = pData[i];
            }
            /* Acknowledge reception */
            FDCAN1->RXF0A = rx_idx;
            
            return CAN_OK;
        }
    }
    
    return CAN_TIMEOUT;
}

/**
  * @brief  Set CAN filter
  * @param  id: Filter ID
  * @param  mask: Filter mask
  */
void CAN_SetFilter(uint32_t id, uint32_t mask)
{
    /* STM32C092 uses a fixed message-RAM layout; accept-all is configured
       in CAN_Init, so there is no SIDFC/SIDFE register to program here. */
    (void)id;
    (void)mask;
}

/**
  * @brief  Check if CAN bus is off
  * @retval 1 if bus off
  */
int CAN_CheckBusOff(void)
{
    return (FDCAN1->PSR & FDCAN_PSR_BO) ? 1 : 0;
}

/**
  * @brief  Reset CAN bus after bus-off
  */
void CAN_ResetBus(void)
{
    /* Request init mode */
    FDCAN1->CCCR |= FDCAN_CCCR_CSR;
    while (!(FDCAN1->CCCR & FDCAN_CCCR_INIT));
    
    /* Clear protocol status */
    FDCAN1->PSR = 0;
    FDCAN1->ECR = 0;
    
    /* Exit init mode */
    FDCAN1->CCCR &= ~FDCAN_CCCR_CSR;
    while (FDCAN1->CCCR & FDCAN_CCCR_INIT);
}

/**
  * @brief  Get CAN error count
  * @retval Error counter value
  */
uint32_t CAN_GetErrorCount(void)
{
    return FDCAN1->ECR;
}
