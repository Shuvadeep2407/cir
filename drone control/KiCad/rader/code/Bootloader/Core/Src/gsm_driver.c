/**
  ******************************************************************************
  * @file    gsm_driver.c
  * @brief   GSM Module Driver - USART1 LL-based, PB6(TX) PB7(RX), 115200 baud
  ******************************************************************************
  */
#include "bootloader.h"
#include "gsm_driver.h"
#include "blink_driver.h"
#include "stm32c0xx.h"
#include "stm32c092_compat.h"

/* Circular RX buffer */
static volatile struct {
    uint8_t buffer[GSM_RX_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} g_gsm_rx;

/* TX buffer */
static uint8_t g_gsm_tx_buf[GSM_TX_BUF_SIZE];

/**
  * @brief  Initialize USART1 for GSM communication
  * @param  baud: Baud rate (default 115200)
  */
void GSM_Init(uint32_t baud)
{
    uint32_t usartdiv;
    
    /* Clear RX buffer */
    g_gsm_rx.head = 0;
    g_gsm_rx.tail = 0;
    
    /* Enable clocks */
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN; /* Enable GPIOA clock for PA2 */
    RCC->APBENR2 |= RCC_APBENR2_USART1EN;
    
    /* Configure PB6(TX) as AF0, PB7(RX) as AF0 */
    GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
    GPIOB->MODER |= (GPIO_MODER_AF_MODE6 | GPIO_MODER_AF_MODE7);
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);  /* AF0 */
    
    /* Configure USART1 */
    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    
    /* Baud rate: USARTDIV = fCK / (8 * (2-OVER8) * baud) */
    /* For 48MHz, 115200 baud, OVER8=0: USARTDIV = 48000000/(16*115200) = 26.04 */
    usartdiv = (SystemCoreClock + (baud * 8)) / (baud * 16);
    
    USART1->BRR = usartdiv;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
    USART1->CR1 |= USART_CR1_UE;
    
    /* Enable USART1 interrupt in NVIC */
    NVIC_SetPriority(USART1_IRQn, 3);
    NVIC_EnableIRQ(USART1_IRQn);
    
    /* Power on GSM module via PA2 */
    GPIOA->MODER &= ~GPIO_MODER_MODE2;
    GPIOA->MODER |= GPIO_MODER_OUTPUT_MODE2;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
    
    /* Give GSM module time to boot */
    Delay_ms(2000);
}

/**
  * @brief  USART1 interrupt handler (called from stm32c0xx_it.c)
  */
void USART1_IRQHandler(void)
{
    if (USART1->ISR & USART_ISR_RXNE)
    {
        uint8_t data = (uint8_t)USART1->RDR;
        
        /* Store in circular buffer */
        uint16_t next = (g_gsm_rx.head + 1) % GSM_RX_BUF_SIZE;
        if (next != g_gsm_rx.tail)
        {
            g_gsm_rx.buffer[g_gsm_rx.head] = data;
            g_gsm_rx.head = next;
        }
    }
}

/**
  * @brief  Check if data is available in RX buffer
  * @retval 1 if data available
  */
int GSM_IsDataAvailable(void)
{
    return (g_gsm_rx.head != g_gsm_rx.tail);
}

/**
  * @brief  Get number of bytes in RX buffer
  * @retval Byte count
  */
uint16_t GSM_GetRxCount(void)
{
    if (g_gsm_rx.head >= g_gsm_rx.tail)
        return g_gsm_rx.head - g_gsm_rx.tail;
    else
        return GSM_RX_BUF_SIZE - g_gsm_rx.tail + g_gsm_rx.head;
}

/**
  * @brief  Clear RX buffer
  */
void GSM_ClearRxBuffer(void)
{
    g_gsm_rx.head = 0;
    g_gsm_rx.tail = 0;
}

/**
  * @brief  Send data over UART
  * @param  data: Data buffer
  * @param  len: Data length
  */
void GSM_SendData(const uint8_t* data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        while (!(USART1->ISR & USART_ISR_TXE));
        USART1->TDR = data[i];
    }
}

/**
  * @brief  Receive data with timeout
  * @param  buffer: Output buffer
  * @param  max_len: Maximum bytes to read
  * @param  timeout_ms: Timeout in milliseconds
  * @retval Number of bytes received, or GSM_TIMEOUT
  */
int GSM_ReceiveData(uint8_t* buffer, uint16_t max_len, uint32_t timeout_ms)
{
    uint32_t start = GetTick();
    uint16_t count = 0;
    
    while (count < max_len)
    {
        if (GetTick() - start > timeout_ms)
            return GSM_TIMEOUT;
        
        if (g_gsm_rx.head != g_gsm_rx.tail)
        {
            buffer[count++] = g_gsm_rx.buffer[g_gsm_rx.tail];
            g_gsm_rx.tail = (g_gsm_rx.tail + 1) % GSM_RX_BUF_SIZE;
            start = GetTick();  /* Reset timeout on new data */
        }
    }
    
    return count;
}

/**
  * @brief  Send AT command
  * @param  cmd: AT command string (null-terminated)
  * @retval GSM_OK or GSM_ERROR
  */
int GSM_SendAT(const char* cmd)
{
    /* Clear any previous response */
    GSM_ClearRxBuffer();
    
    /* Send command */
    while (*cmd)
    {
        while (!(USART1->ISR & USART_ISR_TXE));
        USART1->TDR = *cmd++;
    }
    
    /* Send CR+LF */
    while (!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = '\r';
    while (!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = '\n';
    
    return GSM_OK;
}

/**
  * @brief  Wait for expected response string
  * @param  expected: Expected response (e.g., "OK", "CONNECT")
  * @param  timeout_ms: Timeout
  * @retval GSM_OK if found, GSM_TIMEOUT otherwise
  */
int GSM_WaitResponse(const char* expected, uint32_t timeout_ms)
{
    uint32_t start = GetTick();
    uint16_t match_idx = 0;
    
    while ((GetTick() - start) < timeout_ms)
    {
        if (g_gsm_rx.head != g_gsm_rx.tail)
        {
            uint8_t c = g_gsm_rx.buffer[g_gsm_rx.tail];
            g_gsm_rx.tail = (g_gsm_rx.tail + 1) % GSM_RX_BUF_SIZE;
            
            if (c == expected[match_idx])
            {
                match_idx++;
                if (expected[match_idx] == '\0')
                    return GSM_OK;
            }
            else
            {
                match_idx = 0;
            }
        }
    }
    
    return GSM_TIMEOUT;
}

/**
  * @brief  Check GSM network registration
  * @retval GSM_OK if registered, GSM_ERROR otherwise
  */
int GSM_CheckNetwork(void)
{
    GSM_SendAT("AT+CREG?");
    return GSM_WaitResponse("+CREG: 0,1", GSM_AT_TIMEOUT_MS);
}

/**
  * @brief  Send payload data (for OTA)
  * @param  data: Payload data
  * @param  len: Payload length
  * @retval GSM_OK on success
  */
int GSM_SendPayload(const uint8_t* data, uint32_t len)
{
    /* For OTA, use AT+QISEND or custom TCP/IP send */
    char cmd[32];
    uint32_t remaining = len;
    const uint8_t* ptr = data;
    
    while (remaining > 0)
    {
        uint16_t chunk = (remaining > 1024) ? 1024 : (uint16_t)remaining;
        GSM_SendData(ptr, chunk);
        ptr += chunk;
        remaining -= chunk;
        Delay_ms(10);
    }
    
    return GSM_OK;
}

/**
  * @brief  Power off GSM module
  */
void GSM_PowerOff(void)
{
    /* Power off via PA2 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
    
    /* Disable USART1 */
    USART1->CR1 &= ~USART_CR1_UE;
    NVIC_DisableIRQ(USART1_IRQn);
    
    /* Disable clock */
    RCC->APBENR2 &= ~RCC_APBENR2_USART1EN;
}
