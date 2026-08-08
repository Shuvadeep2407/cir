/**
  ******************************************************************************
  * @file    stepper_driver.c
  * @brief   Stepper Motor Driver - PA5(DIR) PA6(ENABLE) PA7(STEP/TIM1_CH1N)
  ******************************************************************************
  */
#include "bootloader.h"
#include "stepper_driver.h"
#include "stm32c0xx.h"
#include "stm32c092_compat.h"
#include "blink_driver.h"

static int32_t g_position = 0;
static volatile uint32_t g_steps_remaining = 0;

// Default speed: 1kHz pulse frequency
#define DEFAULT_STEPPER_ARR 100

/**
  * @brief  Initialize GPIO and TIM1 for stepper control
  */
void Stepper_Init(void)
{
    /* Enable clocks */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR2 |= RCC_APBENR2_TIM1EN;
    
    /* PA5(DIR) and PA6(ENABLE) as outputs */
    GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6);
    GPIOA->MODER |= (GPIO_MODER_OUTPUT_MODE5 | GPIO_MODER_OUTPUT_MODE6);
    GPIOA->ODR &= ~(GPIO_ODR_OD5 | GPIO_ODR_OD6);  /* DIR=0, ENABLE=0 (disabled) */
    
    /* PA7(STEP) = TIM1_CH1N as AF2 */
    GPIOA->MODER &= ~GPIO_MODER_MODE7;
    GPIOA->MODER |= GPIO_MODER_AF_MODE7;
    GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL7;
    GPIOA->AFR[0] |= (2 << (7 * 4));  /* AF2 = TIM1 */
    
    /* Configure TIM1 for One-Pulse Mode (OPM) to generate a specific number of pulses */
    TIM1->CR1 = 0; // Ensure timer is disabled
    TIM1->PSC = 47;           // 48MHz / (47+1) = 1MHz timer clock
    TIM1->ARR = DEFAULT_STEPPER_ARR; // Period -> determines pulse frequency
    TIM1->CCR1 = DEFAULT_STEPPER_ARR / 2; // 50% duty cycle
    
    TIM1->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode 1
    TIM1->CCER = TIM_CCER_CC1NE;     // Enable CH1N output
    TIM1->BDTR = TIM_BDTR_MOE;       // Main output enable
    TIM1->RCR = 0;                   // Repetition counter, will be set in Stepper_Move
    TIM1->DIER = TIM_DIER_UIE;       // Enable Update Interrupt
    
    g_position = 0;
    g_steps_remaining = 0;

    NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 2);
    NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
}

/**
  * @brief  Move stepper motor
  * @param  steps: Number of steps (positive = CW, negative = CCW)
  * @param  dir: Direction (STEPPER_DIR_CW or STEPPER_DIR_CCW)
  */
void Stepper_Move(int32_t steps, uint8_t dir)
{
    if (g_steps_remaining > 0) return; // Already moving

    int32_t num_steps = (steps > 0) ? steps : -steps;

    if (dir == STEPPER_DIR_CW)
    {
        GPIOA->ODR |= GPIO_ODR_OD5;     /* DIR = 1 (CW) */
        g_position += num_steps;
    }
    else
    {
        GPIOA->ODR &= ~GPIO_ODR_OD5;    /* DIR = 0 (CCW) */
        g_position -= num_steps;
    }
    
    g_steps_remaining = num_steps;

    // Configure timer for OPM to generate 'num_steps' pulses
    TIM1->CR1 &= ~TIM_CR1_CEN; // Disable timer
    TIM1->RCR = num_steps - 1; // Repetition Counter (N-1 pulses)
    TIM1->CR1 |= TIM_CR1_OPM | TIM_CR1_CEN; // Enable One-Pulse Mode and start timer
}

/**
  * @brief  Set stepper speed
  * @param  rpm: Speed in RPM
  */
void Stepper_SetSpeed(uint16_t rpm)
{
    if (rpm == 0 || rpm > STEPPER_MAX_SPEED) return;
    // F_pulse = (rpm * steps_per_rev) / 60
    // ARR = F_timer / F_pulse
    // Assuming 200 steps/rev for this calculation
    uint32_t steps_per_rev = 200;
    uint32_t f_pulse = (rpm * steps_per_rev) / 60;
    if (f_pulse == 0) return;
    uint32_t arr = (1000000 / f_pulse); // 1MHz timer clock
    if (arr > 0xFFFF) arr = 0xFFFF;
    if (arr < 10) arr = 10; // Limit max speed

    TIM1->ARR = arr;
    TIM1->CCR1 = arr / 2;
}

/**
  * @brief  Stop stepper motor
  */
void Stepper_Stop(void)
{
    TIM1->CR1 &= ~TIM_CR1_CEN; // Disable timer
    g_steps_remaining = 0;
}

/**
  * @brief  Enable or disable stepper motor
  * @param  enable: 1=enable, 0=disable
  */
void Stepper_Enable(uint8_t enable)
{
    if (enable)
        GPIOA->ODR |= GPIO_ODR_OD6;
    else
        GPIOA->ODR &= ~GPIO_ODR_OD6;
}

/**
  * @brief  Get current step position
  * @retval Position in steps
  */
int32_t Stepper_GetPosition(void)
{
    return g_position;
}

/**
  * @brief  Set step position (for calibration)
  * @param  pos: Position to set
  */
void Stepper_SetPosition(int32_t pos)
{
    g_position = pos;
}

/**
  * @brief  Set microstepping mode
  * @param  microstep: 1, 2, 4, 8, 16
  */
void Stepper_SetMicrostep(uint8_t microstep)
{
    (void)microstep;  /* Requires external driver chip support */
}

/**
  * @brief  Check if the stepper is currently moving.
  * @retval 1 if moving, 0 if stopped.
  */
int Stepper_IsMoving(void)
{
    return (g_steps_remaining > 0);
}

/**
  * @brief  TIM1 Update Interrupt Handler
  */
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
    if (TIM1->SR & TIM_SR_UIF) {
        TIM1->SR = ~TIM_SR_UIF; // Clear interrupt flag
        g_steps_remaining = 0; // Pulse train finished
    }
}
