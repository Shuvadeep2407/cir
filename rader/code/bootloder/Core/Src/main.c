/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fdcan.h"
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cc1101.h"
#include "client_node.h"
#include "gps_compass.h"

/* Feature macros - enable specific node capabilities */
//#define HAS_AUDIO_NODE

#ifdef HAS_AUDIO_NODE
#include "i2s_mic.h"
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static char client_id[5];
static char client_pwd[5];
static uint8_t client_addr = 0;

/* GPS + Compass combined sensor data (non-static for extern access) */
Sensor_Data_t sensor_data;

/* Global GPS RX byte (fed from USART1 IRQ) */
volatile uint8_t gps_rx_byte = 0;

/* Current heading of the stepper motor relative to the drone's frame (0-360 deg) */
static float motor_current_heading = 0.0f;

#ifdef HAS_AUDIO_NODE
static uint8_t audio_valid = 0;
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Stepper motor: 200 steps/revolution (1.8 deg/step) */
#define STEPS_PER_REV   200

/* A4988 microstepping multiplier - set to match MS1/MS2/MS3 strapping:
 * MS1 MS2 MS3 | Mode        | Steps/rev
 * L   L   L   | Full        | 200
 * H   L   L   | Half        | 400
 * L   H   L   | Quarter     | 800
 * H   H   L   | Eighth      | 1600
 * H   H   H   | Sixteenth   | 3200   <- common default on many carriers
 * If unsure which mode your A4988 is in, keep STEPPER_MICROSTEPS = 16 */
#define STEPPER_MICROSTEPS   16

/**
  * @brief  Microsecond busy-wait delay (48MHz CPU)
  *         Cortex-M0+ NOP + loop overhead ≈ 4 cycles per iteration,
  *         so use 48/4 = 12 iterations per microsecond.
  * @param  us: Microseconds to wait
  */
static void delay_us(uint32_t us)
{
  uint32_t ticks = us * 12;  /* ~12 iterations per µs at 48MHz (4 cyc/iter) */
  while (ticks--)
  {
    __NOP();
  }
}

/* Obfuscation key for boot pattern (keeps phrase hidden in source) */
#define BOOT_PATTERN_KEY  0x55

/**
  * @brief  Show boot phrase pattern on LEDs (right after boot)
  *         LED1=PC6, LED2=PA10, LED3=PA11, LED4=PA12
  *         Pattern is XOR-encoded so the phrase is not readable in source.
  */
static void Boot_ShowPattern(void)
{
  /* Encoded 4-bit LED patterns (bit3=LED1 ... bit0=LED4), XOR key 0x55 */
  static const uint8_t enc[] = {
    0x5A,0x5F,0x55,0x59,0x5A,0x5F,0x50,0x55,
    0x59,0x5F,0x56,0x5A,0x50,0x5F
  };
  uint8_t i, j, pat;

  /* All LEDs off first */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, LED2_Pin|LED3_Pin|LED4_Pin, GPIO_PIN_SET);
  HAL_Delay(150);

  /* Scroll through boot phrase */
  for (i = 0; i < sizeof(enc); i++)
  {
    pat = enc[i] ^ BOOT_PATTERN_KEY;

    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin,
                      (pat & 0x08) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, LED2_Pin,
                      (pat & 0x04) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, LED3_Pin,
                      (pat & 0x02) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, LED4_Pin,
                      (pat & 0x01) ? GPIO_PIN_RESET : GPIO_PIN_SET);

    HAL_Delay(250);

    /* Blink twice for emphasis */
    for (j = 0; j < 2; j++)
    {
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOA, LED2_Pin|LED3_Pin|LED4_Pin, GPIO_PIN_SET);
      HAL_Delay(70);
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin,
                        (pat & 0x08) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOA, LED2_Pin,
                        (pat & 0x04) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOA, LED3_Pin,
                        (pat & 0x02) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOA, LED4_Pin,
                        (pat & 0x01) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_Delay(70);
    }
  }

  /* Final sweep: all LEDs on then off */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, LED2_Pin|LED3_Pin|LED4_Pin, GPIO_PIN_RESET);
  HAL_Delay(400);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, LED2_Pin|LED3_Pin|LED4_Pin, GPIO_PIN_SET);
  HAL_Delay(150);
}

/* Forward declaration: Stepper_Rotate is called by Stepper_PointToHeading
 * but is defined further down in this file. */
static void Stepper_Rotate(int32_t degrees, uint32_t max_speed_hz);

/**
  * @brief  Rotates the stepper motor to point to a specific world heading.
  * @param  target_world_heading: The desired absolute heading (0-360, 0=North).
  * @param  current_drone_heading: The current absolute heading of the drone from the compass.
  * @param  p_current_motor_heading: Pointer to the variable holding the motor's current
  *                                  heading relative to the drone frame.
  */
void Stepper_PointToHeading(float target_world_heading, float current_drone_heading, float *p_current_motor_heading)
{
  /* Calculate how the motor should be angled relative to the drone's body
   * to point to the target world heading. */
  float target_relative_heading = target_world_heading - current_drone_heading;

  /* Normalize the angle to the shortest path (-180 to 180 degrees). */
  while (target_relative_heading > 180.0f)
  {
    target_relative_heading -= 360.0f;
  }
  while (target_relative_heading <= -180.0f)
  {
    target_relative_heading += 360.0f;
  }

  /* Calculate the rotation needed from the motor's current position. */
  float rotation_needed = target_relative_heading - *p_current_motor_heading;

  /* Normalize the required rotation to the shortest path. */
  while (rotation_needed > 180.0f)
  {
    rotation_needed -= 360.0f;
  }
  while (rotation_needed <= -180.0f)
  {
    rotation_needed += 360.0f;
  }

  /* Only move if the required rotation is significant (e.g., > 1 degree). */
  if ( (rotation_needed > 1.0f) || (rotation_needed < -1.0f) )
  {
    /* Rotate the motor by the calculated amount.
     * A speed of 1600 Hz is a reasonable default. */
    Stepper_Rotate((int32_t)rotation_needed, 1600);

    /* Update the motor's current heading. */
    *p_current_motor_heading += rotation_needed;

    /* Keep the motor's heading within the 0-360 degree range for consistency. */
    if (*p_current_motor_heading >= 360.0f)
    {
      *p_current_motor_heading -= 360.0f;
    }
    else if (*p_current_motor_heading < 0.0f)
    {
      *p_current_motor_heading += 360.0f;
    }
  }
}

/**
  * @brief  Rotate stepper motor by given degrees
  * @param  degrees: Angle to rotate (positive = CW, negative = CCW)
  * @param  max_speed_hz: The maximum step frequency (speed) in Hz.
  */
static void Stepper_Rotate(int32_t degrees, uint32_t max_speed_hz)
{
  int32_t steps;
  int32_t i;

  /* Enable motor driver for the duration of this move */
  HAL_GPIO_WritePin(ENABLE_GPIO_Port, ENABLE_Pin, GPIO_PIN_RESET);
  HAL_Delay(200); /* Let coils energize */

  /* Set direction: A4988 DIR HIGH = CW, DIR LOW = CCW (per typical hookup) */
  if (degrees >= 0)
  {
    HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_SET);   /* CW */
    steps = (degrees * STEPS_PER_REV * STEPPER_MICROSTEPS) / 360;
  }
  else
  {
    HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_RESET); /* CCW */
    steps = (-degrees * STEPS_PER_REV * STEPPER_MICROSTEPS) / 360;
  }

  /* --- Ramping Implementation ---
   * To reach high speeds, the motor must accelerate from a slow start speed
   * and decelerate before stopping. This is called ramping. */
  uint32_t start_speed_hz = 400;  /* A safe speed to start and stop at (e.g., 7.5 RPM) */
  uint32_t accel_steps = 400;     /* Number of steps over which to accelerate/decelerate. */

  /* If the total move is too short, there's no time for a full ramp.
   * In this case, use half the steps for acceleration and half for deceleration. */
  if (steps < (2 * accel_steps)) {
    accel_steps = steps / 2;
  }

  for (i = 0; i < steps; i++)
  {
    uint32_t current_speed_hz;

    /* --- Calculate current speed based on position in the ramp --- */
    // Acceleration phase
    if (i < accel_steps) {
      current_speed_hz = start_speed_hz + ((max_speed_hz - start_speed_hz) * i) / accel_steps;
    }
    // Deceleration phase
    else if (i >= (steps - accel_steps)) {
      current_speed_hz = max_speed_hz - ((max_speed_hz - start_speed_hz) * (i - (steps - accel_steps))) / accel_steps;
    }
    // Constant speed (cruise) phase
    else {
      current_speed_hz = max_speed_hz;
    }

    /* Calculate delay for the current speed */
    uint32_t half_period_us = 500000 / current_speed_hz;

    /* A4988 requires a minimum STEP pulse width of 1us.
       Our delay_us is approximate, so ensure we don't go below a safe value. */
    if (half_period_us < 2)
    {
        half_period_us = 2;
    }

    /* --- Generate one step pulse --- */
    HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_SET);
    delay_us(half_period_us);
    HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_RESET);
    delay_us(half_period_us);
  }

  /* Disable motor driver when move is complete to prevent overheating and save power */
  HAL_GPIO_WritePin(ENABLE_GPIO_Port, ENABLE_Pin, GPIO_PIN_SET);
  HAL_Delay(200); /* Let coils energize */
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FDCAN1_Init();
  MX_I2C1_Init();
  MX_I2S1_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* Show boot phrase pattern on LEDs first (before anything else) */
  Boot_ShowPattern();

  /* Initialize CC1101 transceiver */
  //CC1101_Init();

  /* Initialize GPS + Compass sensors */
  if (!Sensors_Init())
  {
    /* Compass or other sensor init failed.
     * This is a critical error, so we enter the error handler.
     * You could add specific LED blinking here for diagnostics. */
    Error_Handler();
  }

  /* Start GPS UART RX interrupt */
  HAL_UART_Receive_IT(&huart1, (uint8_t *)&gps_rx_byte, 1);

#ifdef HAS_AUDIO_NODE
  /* Initialize dual I2S microphone array with DMA */
  if (I2S_Mic_Init())
  {
    I2S_Mic_Start();
    audio_valid = 1;
  }
#endif

  /* Phase 1: Parse configuration from flash */
  if (Client_ParseFlashConfig(client_id, client_pwd))
  {
    /* Phase 2: Dynamic provisioning (Join Request / Join Accept) */
    if (Client_Provision(client_id, client_pwd, &client_addr))
    {
      /* Provisioning successful - enable GDO0 interrupt */
      CC1101_EnableGdo0Interrupt();
    }
    else
    {
      /* Provisioning failed - stay in broadcast mode */
      CC1101_SetAddress(CC1101_ADDR_BROADCAST);
      CC1101_EnableGdo0Interrupt();
    }
  }
  else
  {
    /* Config parse failed - stay in broadcast mode */
    CC1101_SetAddress(CC1101_ADDR_BROADCAST);
    CC1101_EnableGdo0Interrupt();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* Update GPS + Compass sensor data */
    Sensors_Update(&sensor_data);

    /* If compass data is valid, try to point the motor North.
     * This continuously adjusts the motor to compensate for the drone's rotation. */
    if (sensor_data.compass.valid)
    {
      /* Target heading is 0 degrees (North) */
      Stepper_PointToHeading(0.0f, sensor_data.compass.heading_deg, &motor_current_heading);
    }

    /* Check if a packet was received (GDO0 interrupt) */
    if (client_rx_flag)
    {
      uint8_t rx_len;
      uint8_t rx_buf[PKT_MAX_LEN];

      /* Clear flag */
      client_rx_flag = 0;

      /* Read the RX FIFO */
      rx_len = CC1101_ReadRxFifo(rx_buf, PKT_MAX_LEN);

      /* Process the received command and send response */
      if (rx_len > 0)
      {
        Client_ProcessPacket(rx_buf, rx_len);
      }

      /* Return to RX mode */
      Client_EnterRxMode();
    }

    /* Small delay to prevent busy-looping and give CPU to other tasks if needed */
    HAL_Delay(20);

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
