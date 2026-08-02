/**
  ******************************************************************************
  * @file    app_api.h
  * @brief   Application API Header - Copy of Hardware API struct for app use
  *          The application includes THIS header instead of bootloader.h
  *          to access hardware through the bootloader's API table at 0x0800DB00
  ******************************************************************************
  */
#ifndef __APP_API_H
#define __APP_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*===================================================================*/
/*              HARDWARE API TABLE (at fixed flash address)           */
/*===================================================================*/
/* The bootloader places this struct at 0x0800DB00.
   The application reads function pointers from that address. */

typedef struct {
    /* Version info */
    uint32_t version;
    uint32_t api_size;
    
    /* GSM / UART */
    void     (*GSM_Init)(uint32_t baud);
    void     (*GSM_SendData)(const uint8_t* data, uint16_t len);
    int      (*GSM_ReceiveData)(uint8_t* buffer, uint16_t max_len, uint32_t timeout_ms);
    int      (*GSM_SendAT)(const char* cmd);
    int      (*GSM_WaitResponse)(const char* expected, uint32_t timeout_ms);
    int      (*GSM_SendPayload)(const uint8_t* data, uint32_t len);
    int      (*GSM_CheckNetwork)(void);
    void     (*GSM_PowerOff)(void);
    
    /* CAN / FDCAN */
    void     (*CAN_Init)(uint32_t bitrate);
    int      (*CAN_Transmit)(uint32_t id, const uint8_t* data, uint8_t len, int is_extended);
    int      (*CAN_Receive)(uint32_t* id, uint8_t* data, uint8_t* len, uint32_t timeout_ms);
    void     (*CAN_SetFilter)(uint32_t id, uint32_t mask);
    int      (*CAN_CheckBusOff)(void);
    void     (*CAN_ResetBus)(void);
    
    /* SPI / CC1101 Radio */
    void     (*SPI_Radio_Init)(void);
    int      (*SPI_Radio_WriteReg)(uint8_t reg, uint8_t value);
    int      (*SPI_Radio_ReadReg)(uint8_t reg, uint8_t* value);
    int      (*SPI_Radio_SendPacket)(const uint8_t* data, uint8_t len);
    int      (*SPI_Radio_ReceivePacket)(uint8_t* data, uint8_t* len, uint32_t timeout_ms);
    void     (*SPI_Radio_SetFrequency)(uint32_t freq_hz);
    int      (*SPI_Radio_CheckChannel)(void);
    
    /* Stepper Motor */
    void     (*Stepper_Init)(void);
    void     (*Stepper_Move)(int32_t steps, uint8_t dir);
    void     (*Stepper_SetSpeed)(uint16_t rpm);
    void     (*Stepper_Stop)(void);
    void     (*Stepper_Enable)(uint8_t enable);
    int32_t  (*Stepper_GetPosition)(void);
    void     (*Stepper_SetPosition)(int32_t pos);
    
    /* I2C Bus */
    void     (*I2C_Init)(uint32_t frequency);
    int      (*I2C_Write)(uint16_t dev_addr, const uint8_t* data, uint16_t len);
    int      (*I2C_Read)(uint16_t dev_addr, uint8_t* data, uint16_t len);
    int      (*I2C_WriteReg)(uint16_t dev_addr, uint8_t reg, const uint8_t* data, uint16_t len);
    int      (*I2C_ReadReg)(uint16_t dev_addr, uint8_t reg, uint8_t* data, uint16_t len);
    int      (*I2C_IsDeviceReady)(uint16_t dev_addr, uint32_t timeout_ms);
    
    /* I2S Audio */
    void     (*I2S_Audio_Init)(uint32_t sample_rate);
    void     (*I2S_Audio_StartDMA)(uint16_t* buffer, uint16_t size);
    void     (*I2S_Audio_StopDMA)(void);
    void     (*I2S_Audio_SetVolume)(uint8_t volume);
    int      (*I2S_Audio_IsPlaying)(void);
    
    /* LED Status */
    void     (*LED_Init)(void);
    void     (*LED_On)(uint8_t led_id);
    void     (*LED_Off)(uint8_t led_id);
    void     (*LED_Toggle)(uint8_t led_id);
    void     (*LED_SetPattern)(uint8_t pattern);
    void     (*LED_Blink)(uint8_t led_id, uint16_t period_ms, uint8_t count);
    
    /* Timing */
    void     (*Delay_ms)(uint32_t ms);
    uint32_t (*GetTick)(void);
    void     (*Watchdog_Refresh)(void);
    
    /* Flash / NVM */
    int      (*Flash_ErasePage)(uint32_t address);
    int      (*Flash_Write)(uint32_t address, const uint8_t* data, uint32_t len);
    int      (*Flash_Read)(uint32_t address, uint8_t* data, uint32_t len);
    int      (*NVM_WriteConfig)(uint16_t id, const uint8_t* data, uint16_t len);
    int      (*NVM_ReadConfig)(uint16_t id, uint8_t* data, uint16_t len);
    
    /* System Control */
    void     (*System_Reset)(void);
    void     (*JumpToApplication)(uint32_t app_address);
    int      (*VerifyApplicationCRC)(uint32_t app_address, uint32_t size, uint32_t expected_crc);
    void     (*EnterBootloader)(void);
    
} HardwareAPI_t;

/*===================================================================*/
/*                      API TABLE ACCESS MACRO                       */
/*===================================================================*/
/* The bootloader places its API table at 0x0800DB00.
   The application uses this macro to call hardware functions. */
#define HW    ((const HardwareAPI_t*)0x0800DB00)

/*===================================================================*/
/*                      MEMORY CONSTANTS                             */
/*===================================================================*/
#define APPLICATION_BASE            0x0800DC00UL
#define SHARED_RAM_BASE             0x20000000UL

/*===================================================================*/
/*                      SHARED RAM STRUCTURE                         */
/*===================================================================*/
typedef struct {
    uint32_t magic;
    uint32_t boot_flags;
    uint32_t ota_firmware_size;
    uint32_t ota_crc32;
    uint32_t ota_source;
    uint32_t app_crc32;
    uint32_t app_version;
    uint32_t boot_attempts;
    uint32_t reserved[6];
} SharedRAM_t;

#define SHARED_RAM_MAGIC            0xDEADBEEFUL
#define BOOT_FLAG_OTA_PENDING       (1 << 0)
#define BOOT_FLAG_FORCE_BOOTLOADER  (1 << 2)

/* Get shared RAM pointer */
#define SHARED_RAM  ((volatile SharedRAM_t*)SHARED_RAM_BASE)

#ifdef __cplusplus
}
#endif

#endif /* __APP_API_H */