/**
  ******************************************************************************
  * @file    stepper.h
  * @brief   A4988 stepper driver: STEP = TIM3_CH2 (PA7) PWM, DIR = PA5, /EN = PA6.
  *          Steps are counted in the TIM3 update interrupt so absolute moves work.
  ******************************************************************************
  */
#ifndef __STEPPER_H__
#define __STEPPER_H__

#include "main.h"

#define STEPPER_DIR_CW   GPIO_PIN_SET
#define STEPPER_DIR_CCW  GPIO_PIN_RESET

void Stepper_Init(void);
void Stepper_Enable(void);                       /* /EN low -> motor on   */
void Stepper_Disable(void);                      /* /EN high -> motor off */
void Stepper_SetDirection(GPIO_PinState dir);
void Stepper_SetSpeed(uint32_t steps_per_sec);   /* step frequency (Hz)   */
void Stepper_MoveTo(int32_t target);             /* absolute target (steps) */
void Stepper_Stop(void);
int32_t Stepper_GetPosition(void);
uint8_t Stepper_IsMoving(void);

#endif /* __STEPPER_H__ */