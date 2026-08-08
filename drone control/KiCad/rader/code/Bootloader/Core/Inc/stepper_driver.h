/**
  ******************************************************************************
  * @file    stepper_driver.h
  * @brief   Stepper Motor Driver Header - PA5(DIR), PA6(ENABLE), PA7(STEP/PWM)
  ******************************************************************************
  */
#ifndef __STEPPER_DRIVER_H
#define __STEPPER_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Stepper defaults */
#define STEPPER_DEFAULT_RPM     60
#define STEPPER_MAX_SPEED       300
#define STEPPER_PWM_PERIOD      65535

/* Direction */
#define STEPPER_DIR_CW          1
#define STEPPER_DIR_CCW         0

void    Stepper_Init(void);
void    Stepper_Move(int32_t steps, uint8_t dir);
void    Stepper_SetSpeed(uint16_t rpm);
void    Stepper_Stop(void);
void    Stepper_Enable(uint8_t enable);
int32_t Stepper_GetPosition(void);
void    Stepper_SetPosition(int32_t pos);
void    Stepper_SetMicrostep(uint8_t microstep);

#ifdef __cplusplus
}
#endif

#endif /* __STEPPER_DRIVER_H */