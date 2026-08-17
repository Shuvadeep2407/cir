/**
  ******************************************************************************
  * @file    stepper.c
  * @brief   A4988 stepper control using TIM3_CH2 PWM on PA7 as the STEP pulse.
  *
  *          The step frequency is set by arming the timer PWM period. Every
  *          overflow (update event) equals one physical step; the running count
  *          is maintained in HAL_TIM_PeriodElapsedCallback so that absolute
  *          moves (Stepper_MoveTo) work.
  ******************************************************************************
  */
#include "stepper.h"
#include "tim.h"

#define STEPPER_TIM_CLK 48000000u   /* TIM3 clock = PCLK1 = 48 MHz (HAL config) */

static volatile int32_t s_position = 0;
static volatile int32_t s_target   = 0;
static uint8_t s_pwm_running = 0;

void Stepper_Init(void)
{
  __HAL_RCC_TIM3_CLK_ENABLE();

  HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);

  s_position = 0;
  s_target   = 0;
  s_pwm_running = 0;

  Stepper_SetSpeed(400);   /* default speed */
}

void Stepper_Enable(void)
{
  /* A4988 /EN is active-low */
  HAL_GPIO_WritePin(A4988_en_GPIO_Port, A4988_en_Pin, GPIO_PIN_RESET);
}

void Stepper_Disable(void)
{
  HAL_GPIO_WritePin(A4988_en_GPIO_Port, A4988_en_Pin, GPIO_PIN_SET);
}

void Stepper_SetDirection(GPIO_PinState dir)
{
  HAL_GPIO_WritePin(A4988_dir_GPIO_Port, A4988_dir_Pin, dir);
}

/**
  * @brief Program TIM3 for a given step frequency with a 50% duty cycle.
  * @param steps_per_sec Desired step rate in Hz (>= 1).
  */
void Stepper_SetSpeed(uint32_t steps_per_sec)
{
  uint32_t psc;
  uint32_t arr;

  if (steps_per_sec == 0u) steps_per_sec = 1u;

  /* Choose smallest prescaler so that ARR stays within 16 bits:
     psc >= ceil(TIM_CLK / (steps_per_sec * 65535)) - 1 */
  uint32_t need = (STEPPER_TIM_CLK + steps_per_sec * 65535u - 1u)
                  / (steps_per_sec * 65535u);
  psc = (need > 1u) ? (need - 1u) : 0u;

  arr = STEPPER_TIM_CLK / (psc + 1u) / steps_per_sec;
  if (arr == 0u) arr = 1u;

  __HAL_TIM_SET_PRESCALER(&htim3, psc);
  __HAL_TIM_SET_AUTORELOAD(&htim3, arr - 1u);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, (arr - 1u) / 2u);
  __HAL_TIM_SET_COUNTER(&htim3, 0u);

  /* Reload the PSC/ARR shadow registers (update event) */
  htim3.Instance->EGR = TIM_EGR_UG;
}

/**
  * @brief Move the axis to an absolute step position, starting the PWM if idle.
  * @note  Set the desired direction first with Stepper_SetDirection().
  */
void Stepper_MoveTo(int32_t target)
{
  s_target = target;

  if ((int32_t)s_position == target)
  {
    Stepper_Stop();
    return;
  }

  Stepper_Enable();

  if (!s_pwm_running)
  {
    __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);
    if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2) == HAL_OK)
    {
      s_pwm_running = 1;
    }
  }
}

void Stepper_Stop(void)
{
  if (s_pwm_running)
  {
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
    __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE);
    s_pwm_running = 0;
  }
}

int32_t Stepper_GetPosition(void)
{
  return (int32_t)s_position;
}

uint8_t Stepper_IsMoving(void)
{
  return s_pwm_running;
}

/**
  * @brief Called by HAL_TIM_IRQHandler on every TIM3 update (one physical step).
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM3)
  {
    GPIO_PinState dir = HAL_GPIO_ReadPin(A4988_dir_GPIO_Port, A4988_dir_Pin);
    s_position += (dir == GPIO_PIN_SET) ? 1 : -1;

    if ((int32_t)s_position == (int32_t)s_target)
    {
      Stepper_Stop();
    }
  }
}