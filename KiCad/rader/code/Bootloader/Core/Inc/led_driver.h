/**
  ******************************************************************************
  * @file    led_driver.h
  * @brief   LED Status Driver - PC6(LED1), PA10(LED2), PA11(LED3), PA12(LED4)
  ******************************************************************************
  */
#ifndef __LED_DRIVER_H
#define __LED_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* LED IDs */
#define LED1                    1
#define LED2                    2
#define LED3                    3
#define LED4                    4

/* LED Patterns */
#define LED_PATTERN_NONE        0
#define LED_PATTERN_ALL_ON      1
#define LED_PATTERN_ALL_OFF     2
#define LED_PATTERN_ALTERNATE   3
#define LED_PATTERN_SEQUENCE    4
#define LED_PATTERN_ERROR       5    /* Fast blink all */

void    LED_Init(void);
void    LED_On(uint8_t led_id);
void    LED_Off(uint8_t led_id);
void    LED_Toggle(uint8_t led_id);
void    LED_SetPattern(uint8_t pattern);
void    LED_Blink(uint8_t led_id, uint16_t period_ms, uint8_t count);
void    LED_Update(void);  /* Call periodically for patterns */
void    LED_Error(uint8_t error_code);
void    LED_BootloaderIndicator(void);

#ifdef __cplusplus
}
#endif

#endif /* __LED_DRIVER_H */