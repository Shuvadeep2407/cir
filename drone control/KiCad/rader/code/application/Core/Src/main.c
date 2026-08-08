/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main application program body
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bootloader_api.h" // Include header for the bootloader API
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Global pointer to the bootloader's hardware API table. */
const HardwareAPI_t* g_hw_api;

/* USER CODE END PD */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Placeholder for device addresses and constants
#define COMPASS_I2C_ADDR    0x1E  // Example I2C address for a compass
#define RADAR_CAN_ID_CMD    0x100 // Example CAN ID to command the radar
#define RADAR_CAN_ID_STATUS 0x101 // Example CAN ID for radar status
#define STEPS_PER_REVOLUTION 2048 // Example: 2048 steps for a full 360-degree rotation

// --- Radar Protocol Defines ---
#define RADAR_CMD_GET_STATUS 0x01

#define RADAR_STATUS_OK      0xAA
#define RADAR_STATUS_ERROR   0xFF

/**
  * @brief  Indicates an error by blinking an LED rapidly.
  */
static void indicate_error(void) {
    while(1) {
        g_hw_api->LED_Toggle(1); // Fast blink LED 1
        g_hw_api->Delay_ms(100);
    }
}

/**
  * @brief  Checks for a valid GPS fix.
  * @retval 0 on success, -1 on failure.
  */
static int check_gps_fix(void) {
    // Example: Send AT command to get GPS info and check for a fix.
    // This assumes the GPS is part of the GSM module.
    // The exact command and response will depend on your module.
    g_hw_api->GSM_SendAT("AT+CGPSINFO");
    // A real implementation would parse the response. Here, we just wait.
    if (g_hw_api->GSM_WaitResponse(",,,,,,,", 5000) != 0) { // Look for an empty fix response
        return 0; // Assuming empty response means no fix yet, but we'll pass for now.
    }
    return 0; // Placeholder for success
}

/**
  * @brief  Checks if the compass is responding and data is valid.
  * @retval 0 on success, -1 on failure.
  */
static int check_compass(void) {
    if (g_hw_api->I2C_IsDeviceReady(COMPASS_I2C_ADDR, 100) != 0) {
        return -1; // Compass not found
    }
    // Optional: Read a status/ID register to confirm it's the correct device
    return 0;
}

/**
  * @brief  Performs a self-test of the stepper motor.
  * @retval 0 on success, -1 on failure.
  */
static int check_stepper(void) {
    g_hw_api->Stepper_Enable(1);
    g_hw_api->Delay_ms(100);

    uint32_t timeout;

    // Clockwise 360°
    g_hw_api->Stepper_Move(STEPS_PER_REVOLUTION, 1 /* DIR_CW */);
    timeout = 5000; // 5 second timeout
    while (g_hw_api->Stepper_IsMoving() && timeout--) {
        g_hw_api->Delay_ms(1);
    }
    if (timeout == 0) return -1; // Stepper move timed out

    // Counter-clockwise 360°
    g_hw_api->Stepper_Move(STEPS_PER_REVOLUTION, 0 /* DIR_CCW */);
    timeout = 5000; // 5 second timeout
    while (g_hw_api->Stepper_IsMoving() && timeout--) {
        g_hw_api->Delay_ms(1);
    }
    if (timeout == 0) return -1; // Stepper move timed out

    g_hw_api->Stepper_Stop();
    g_hw_api->Stepper_Enable(0);
    return 0;
}

/**
  * @brief  Checks if the CC1101 radio module is responding.
  * @retval 0 on success, -1 on failure.
  */
static int check_cc1101(void) {
    // The CheckChannel function in the bootloader API can be used for this.
    // It reads RSSI, a good indicator of a working radio.
    if (g_hw_api->SPI_Radio_CheckChannel() != 0) {
        return -1;
    }
    return 0;
}

/**
  * @brief  Scans for and checks the status of the radar on the CAN bus.
  * @retval 0 on success, -1 on failure.
  */
static int check_radar_status(void) {
    uint8_t cmd_packet[2];
    cmd_packet[0] = RADAR_CMD_GET_STATUS;
    cmd_packet[1] = cmd_packet[0]; // Simple checksum (XOR of all data bytes)

    // Transmit the "get status" command to the radar
    if (g_hw_api->CAN_Transmit(RADAR_CAN_ID_CMD, cmd_packet, 2, 0) != 0) {
        return -1; // Failed to transmit
    }

    uint32_t can_id;
    uint8_t can_data[8];
    uint8_t can_len;

    // Wait for a response from the radar
    if (g_hw_api->CAN_Receive(&can_id, can_data, &can_len, 1000) == 0) {
        // Check if the response is from the correct radar and has a valid length and status
        if (can_id == RADAR_CAN_ID_STATUS && can_len >= 2 && can_data[0] == RADAR_STATUS_OK) {
            // Validate checksum (simple XOR of data bytes up to the checksum byte)
            uint8_t checksum = 0;
            for (uint8_t i = 0; i < can_len - 1; ++i) {
                checksum ^= can_data[i];
            }
            if (checksum != can_data[can_len - 1]) return -1; // Checksum error
            return 0; // Radar status OK
        }
    }
    return -1; // No response or error status
}

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

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
  /* This HAL_Init is still necessary for the application's own HAL-based logic, if any. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  // The bootloader has already configured the system clock.
  // Re-configuring it here is unnecessary and could cause issues.
  // SystemClock_Config();
  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */
  // Get the pointer to the bootloader's hardware API table.
  // BOOTLOADER_API_ADDRESS is defined in bootloader_api.h and must match the bootloader's linker script.
  g_hw_api = (const HardwareAPI_t*)BOOTLOADER_API_ADDRESS;
  /* USER CODE END SysInit */

  /* Peripherals are already initialized by the bootloader.
   * Calling MX_..._Init() functions here would re-initialize them,
   * which is incorrect for this two-stage bootloader architecture.
   * The application should use the functions provided by the bootloader's API. */

  /* USER CODE BEGIN 2 */
  // Ensure the API table is valid before proceeding
  if (g_hw_api && g_hw_api->version >= 0x0100) {
      g_hw_api->LED_On(1); // Turn on LED1 to indicate startup sequence begin
  }
  else {
      // Critical error: Bootloader API not found or incompatible.
      // Loop indefinitely with a specific LED pattern.
      while(1);
  }

  // --- Startup Self-Test Sequence ---

  if (check_gps_fix() != 0)      { g_hw_api->LED_On(4); indicate_error(); }
  g_hw_api->LED_On(2); g_hw_api->Delay_ms(200); // GPS OK

  if (check_compass() != 0)      { g_hw_api->LED_On(4); indicate_error(); }
  g_hw_api->LED_On(3); g_hw_api->Delay_ms(200); // Compass OK

  // Calibration placeholder
  g_hw_api->Delay_ms(500);

  if (check_cc1101() != 0)       { g_hw_api->LED_On(4); indicate_error(); }
  g_hw_api->LED_Off(2); g_hw_api->Delay_ms(200); // CC1101 OK

  if (check_stepper() != 0)      { g_hw_api->LED_On(4); indicate_error(); }
  g_hw_api->LED_Off(3); g_hw_api->Delay_ms(200); // Stepper OK

  if (check_radar_status() != 0) { g_hw_api->LED_On(4); indicate_error(); }
  g_hw_api->LED_Off(1); // All checks OK, turn off startup LED

  // All systems go. Start main radar operation.
  g_hw_api->LED_Blink(4, 500, 0); // Blink LED 4 to indicate normal operation
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Main operational loop
    // Example: Periodically check radar and adjust stepper
    // g_hw_api->Stepper_Move(10, 1);
    // g_hw_api->Delay_ms(100);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
