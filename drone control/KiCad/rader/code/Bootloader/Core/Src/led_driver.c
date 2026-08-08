/**
  ******************************************************************************
  * @file    led_driver.c
  * @brief   LED Status Driver - PC6(LED1), PA10(LED2), PA11(LED3), PA12(LED4)
  ******************************************************************************
  */
#include "bootloader.h"
#include "led_driver.h"
#include "blink_driver.h"
#include "stm32c0xx.h"
#include "stm32c092_compat.h"

static struct {
    uint8_t  pattern;
    uint16_t timer;
    uint16_t period;
    uint8_t  blink_count[4];
    uint8_t  blink_state[4];
    uint16_t blink_period[4];
    uint16_t blink_timer[4];
} g_led;

void LED_Init(void)
{
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOCEN;
    
    /* PC6(LED1), PA10(LED2), PA11(LED3), PA12(LED4) as outputs */
    GPIOC->MODER &= ~GPIO_MODER_MODE6;
    GPIOC->MODER |= GPIO_MODER_OUTPUT_MODE6;
    
    GPIOA->MODER &= ~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12);
    GPIOA->MODER |= (GPIO_MODER_OUTPUT_MODE10 | GPIO_MODER_OUTPUT_MODE11 | GPIO_MODER_OUTPUT_MODE12);
    
    /* All LEDs off initially */
    GPIOC->ODR &= ~GPIO_ODR_OD6;
    GPIOA->ODR &= ~(GPIO_ODR_OD10 | GPIO_ODR_OD11 | GPIO_ODR_OD12);
    
    g_led.pattern = LED_PATTERN_NONE;
}

void LED_On(uint8_t led_id)
{
    switch(led_id)
    {
        case 1: GPIOC->ODR |= GPIO_ODR_OD6; break;
        case 2: GPIOA->ODR |= GPIO_ODR_OD10; break;
        case 3: GPIOA->ODR |= GPIO_ODR_OD11; break;
        case 4: GPIOA->ODR |= GPIO_ODR_OD12; break;
    }
}

void LED_Off(uint8_t led_id)
{
    switch(led_id)
    {
        case 1: GPIOC->ODR &= ~GPIO_ODR_OD6; break;
        case 2: GPIOA->ODR &= ~GPIO_ODR_OD10; break;
        case 3: GPIOA->ODR &= ~GPIO_ODR_OD11; break;
        case 4: GPIOA->ODR &= ~GPIO_ODR_OD12; break;
    }
}

void LED_Toggle(uint8_t led_id)
{
    switch(led_id)
    {
        case 1: GPIOC->ODR ^= GPIO_ODR_OD6; break;
        case 2: GPIOA->ODR ^= GPIO_ODR_OD10; break;
        case 3: GPIOA->ODR ^= GPIO_ODR_OD11; break;
        case 4: GPIOA->ODR ^= GPIO_ODR_OD12; break;
    }
}

void LED_Blink(uint8_t led_id, uint16_t period_ms, uint8_t count)
{
    if (led_id < 1 || led_id > 4) return;
    g_led.blink_period[led_id-1] = period_ms;
    g_led.blink_count[led_id-1] = count;
    g_led.blink_state[led_id-1] = 0;
    g_led.blink_timer[led_id-1] = 0;
    LED_On(led_id);
}

void LED_SetPattern(uint8_t pattern)
{
    g_led.pattern = pattern;
    g_led.timer = 0;
}

void LED_Update(void)
{
    /* Handle blink timers for individual LEDs */
    for (int i = 0; i < 4; i++)
    {
        if (g_led.blink_count[i] > 0)
        {
            g_led.blink_timer[i]++;
            if (g_led.blink_timer[i] >= g_led.blink_period[i])
            {
                g_led.blink_timer[i] = 0;
                LED_Toggle(i + 1);
                g_led.blink_state[i] ^= 1;
                // If count is not infinite (0), decrement it when a full ON-OFF cycle is complete.
                if (g_led.blink_count[i] != 0 && g_led.blink_state[i] == 0)
                    g_led.blink_count[i]--;
            }
        }
    }
    
    /* Handle global patterns */
    g_led.timer++;
    switch(g_led.pattern)
    {
        case LED_PATTERN_ALL_ON:
            LED_On(1); LED_On(2); LED_On(3); LED_On(4);
            break;
        case LED_PATTERN_ALL_OFF:
            LED_Off(1); LED_Off(2); LED_Off(3); LED_Off(4);
            break;
        case LED_PATTERN_ALTERNATE:
            if (g_led.timer % 50 == 0)
            {
                LED_Toggle(1); LED_Toggle(3);
                LED_Toggle(2); LED_Toggle(4);
            }
            break;
        case LED_PATTERN_SEQUENCE:
            if (g_led.timer % 25 == 0)
            {
                static uint8_t seq = 0;
                LED_Off(1); LED_Off(2); LED_Off(3); LED_Off(4);
                LED_On(seq + 1);
                seq = (seq + 1) % 4;
            }
            break;
    }
}

void LED_Error(uint8_t error_code)
{
    /* Fast blink all LEDs to indicate error */
    for (int i = 0; i < error_code * 3; i++)
    {
        LED_On(1); LED_On(2); LED_On(3); LED_On(4);
        Delay_ms(100);
        LED_Off(1); LED_Off(2); LED_Off(3); LED_Off(4);
        Delay_ms(100);
    }
}

void LED_BootloaderIndicator(void)
{
    /* Fast "Om Namah Shivaya" pattern for bootloader indication */
    uint8_t i;
    for (i = 0; i < 2; i++) {
        // "Om" - All LEDs flash
        LED_On(1); LED_On(2); LED_On(3); LED_On(4);
        Delay_ms(50);
        LED_Off(1); LED_Off(2); LED_Off(3); LED_Off(4);
        Delay_ms(100);

        // "Na-mah"
        LED_On(1); LED_On(2);
        Delay_ms(80);
        LED_Off(1); LED_Off(2);
        LED_On(3); LED_On(4);
        Delay_ms(80);
        LED_Off(3); LED_Off(4);
        Delay_ms(100);

        // "Shi-va-ya"
        LED_On(1); LED_On(3);
        Delay_ms(80);
        LED_Off(1); LED_Off(3);
        LED_On(2); LED_On(4);
        Delay_ms(80);
        LED_Off(2); LED_Off(4);
        Delay_ms(100);
    }
    /* Turn all off to finish */
    LED_Off(1); LED_Off(2); LED_Off(3); LED_Off(4);
}
