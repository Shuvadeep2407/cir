/**
  ******************************************************************************
  * @file    bootloader.h
  * @brief   Bootloader main header - Hardware API table, shared RAM, OTA
  ******************************************************************************
  */
#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*===================================================================*/
/*                        VERSION DEFINITIONS                        */
/*===================================================================*/
#define BOOTLOADER_VERSION_MAJOR    1
#define BOOTLOADER_VERSION_MINOR    0
#define BOOTLOADER_VERSION_PATCH    0
#define BOOTLOADER_VERSION          ((BOOTLOADER_VERSION_MAJOR << 16) | \
                                     (BOOTLOADER_VERSION_MINOR << 8)  | \
                                     BOOTLOADER_VERSION_PATCH)

/*===================================================================*/
/*                     MEMORY MAP CONSTANTS                          */
/*===================================================================*/
#define BOOTLOADER_BASE             0x08000000UL
#define BOOTLOADER_SIZE             (55 * 1024)         /* 55KB */
#define BOOTLOADER_END              (BOOTLOADER_BASE + BOOTLOADER_SIZE)

#define API_TABLE_ADDR              0x0800DB00UL        /* Last 256B of bootloader */
#define API_TABLE_SIZE              256

#define APPLICATION_BASE            0x0800DC00UL        /* Start of app */
#define APPLICATION_MAX_SIZE        (199 * 1024)        /* ~199KB */

#define NVM_BASE                    0x0803F800UL        /* 2KB NVM */
#define NVM_SIZE                    (2 * 1024)

#define SHARED_RAM_BASE             0x20000000UL        /* 256B shared RAM */
#define SHARED_RAM_SIZE             256

#define APP_RAM_BASE                0x20000100UL
#define APP_RAM_SIZE                (30 * 1024 - 256)

/*===================================================================*/
/*                    OTA / BOOT FLAGS (Shared RAM)                   */
/*===================================================================*/
typedef struct {
    /* Bootloader control flags */
    uint32_t magic;                     /* 0xDEADBEEF if valid */
    uint32_t boot_flags;                /* Bitfield of boot options */
    uint32_t ota_firmware_size;         /* Size of received OTA firmware */
    uint32_t ota_crc32;                 /* CRC32 of received firmware */
    uint32_t ota_source;                /* 0=GSM, 1=CAN, 2=SPI(RF) */
    uint32_t app_crc32;                 /* CRC32 of current app (validated) */
    uint32_t app_version;               /* Version of installed app */
    uint32_t boot_attempts;             /* Boot attempt counter */
    uint32_t reserved[6];               /* Future use */
} __attribute__((packed)) SharedRAM_t;

/* Boot flags bit definitions */
#define BOOT_FLAG_OTA_PENDING       (1 << 0)    /* OTA firmware received, pending flash */
#define BOOT_FLAG_APP_VALID         (1 << 1)    /* Application CRC verified */
#define BOOT_FLAG_FORCE_BOOTLOADER  (1 << 2)    /* Force stay in bootloader */
#define BOOT_FLAG_FACTORY_RESET     (1 << 3)    /* Reset to factory firmware */
#define BOOT_FLAG_UPDATE_IN_PROG    (1 << 4)    /* OTA update in progress */
#define BOOT_FLAG_ROLLBACK          (1 << 5)    /* Rollback to previous version */

/* OTA source definitions */
#define OTA_SOURCE_GSM              0
#define OTA_SOURCE_CAN              1
#define OTA_SOURCE_SPI              2

/* Magic number for shared RAM validation */
#define SHARED_RAM_MAGIC            0xDEADBEEFUL

/*===================================================================*/
/*              HARDWARE API JUMP TABLE STRUCTURE                     */
/*===================================================================*/
/* This struct is placed at API_TABLE_ADDR (0x0800DB00) in flash.
   The main application accesses hardware functions through this table,
   avoiding the need to compile driver code into the app. */

typedef struct {
    /* Version info */
    uint32_t version;                       /* Bootloader version */
    uint32_t api_size;                      /* Size of this struct in bytes */
    
    /*===================================================================*/
    /*                      GSM / UART DRIVER                            */
    /*===================================================================*/
    void     (*GSM_Init)(uint32_t baud);
    void     (*GSM_SendData)(const uint8_t* data, uint16_t len);
    int      (*GSM_ReceiveData)(uint8_t* buffer, uint16_t max_len, uint32_t timeout_ms);
    int      (*GSM_SendAT)(const char* cmd);
    int      (*GSM_WaitResponse)(const char* expected, uint32_t timeout_ms);
    int      (*GSM_SendPayload)(const uint8_t* data, uint32_t len);
    int      (*GSM_CheckNetwork)(void);
    void     (*GSM_PowerOff)(void);
    
    /*===================================================================*/
    /*                      CAN / FDCAN DRIVER                           */
    /*===================================================================*/
    void     (*CAN_Init)(uint32_t bitrate);
    int      (*CAN_Transmit)(uint32_t id, const uint8_t* data, uint8_t len, int is_extended);
    int      (*CAN_Receive)(uint32_t* id, uint8_t* data, uint8_t* len, uint32_t timeout_ms);
    void     (*CAN_SetFilter)(uint32_t id, uint32_t mask);
    int      (*CAN_CheckBusOff)(void);
    void     (*CAN_ResetBus)(void);
    
    /*===================================================================*/
    /*                   SPI / CC1101 RADIO DRIVER                       */
    /*===================================================================*/
    void     (*SPI_Radio_Init)(void);
    int      (*SPI_Radio_WriteReg)(uint8_t reg, uint8_t value);
    int      (*SPI_Radio_ReadReg)(uint8_t reg, uint8_t* value);
    int      (*SPI_Radio_SendPacket)(const uint8_t* data, uint8_t len);
    int      (*SPI_Radio_ReceivePacket)(uint8_t* data, uint8_t* len, uint32_t timeout_ms);
    void     (*SPI_Radio_SetFrequency)(uint32_t freq_hz);
    int      (*SPI_Radio_CheckChannel)(void);
    
    /*===================================================================*/
    /*                    STEPPER MOTOR DRIVER                           */
    /*===================================================================*/
    void     (*Stepper_Init)(void);
    void     (*Stepper_Move)(int32_t steps, uint8_t dir);
    void     (*Stepper_SetSpeed)(uint16_t rpm);
    void     (*Stepper_Stop)(void);
    void     (*Stepper_Enable)(uint8_t enable);
    int32_t  (*Stepper_GetPosition)(void);
    void     (*Stepper_SetPosition)(int32_t pos);
    
    /*===================================================================*/
    /*                       I2C BUS DRIVER                              */
    /*===================================================================*/
    void     (*I2C_Init)(uint32_t frequency);
    int      (*I2C_Write)(uint16_t dev_addr, const uint8_t* data, uint16_t len);
    int      (*I2C_Read)(uint16_t dev_addr, uint8_t* data, uint16_t len);
    int      (*I2C_WriteReg)(uint16_t dev_addr, uint8_t reg, const uint8_t* data, uint16_t len);
    int      (*I2C_ReadReg)(uint16_t dev_addr, uint8_t reg, uint8_t* data, uint16_t len);
    int      (*I2C_IsDeviceReady)(uint16_t dev_addr, uint32_t timeout_ms);
    
    /*===================================================================*/
    /*                      I2S AUDIO DRIVER                             */
    /*===================================================================*/
    void     (*I2S_Audio_Init)(uint32_t sample_rate);
    void     (*I2S_Audio_StartDMA)(uint16_t* buffer, uint16_t size);
    void     (*I2S_Audio_StopDMA)(void);
    void     (*I2S_Audio_SetVolume)(uint8_t volume);
    int      (*I2S_Audio_IsPlaying)(void);
    
    /*===================================================================*/
    /*                      LED STATUS DRIVER                            */
    /*===================================================================*/
    void     (*LED_Init)(void);
    void     (*LED_On)(uint8_t led_id);       /* 1-4 */
    void     (*LED_Off)(uint8_t led_id);
    void     (*LED_Toggle)(uint8_t led_id);
    void     (*LED_SetPattern)(uint8_t pattern);
    void     (*LED_Blink)(uint8_t led_id, uint16_t period_ms, uint8_t count);
    
    /*===================================================================*/
    /*                      TIMING / DELAY                               */
    /*===================================================================*/
    void     (*Delay_ms)(uint32_t ms);
    uint32_t (*GetTick)(void);
    void     (*Watchdog_Refresh)(void);
    
    /*===================================================================*/
    /*                      FLASH / NVM OPERATIONS                       */
    /*===================================================================*/
    int      (*Flash_ErasePage)(uint32_t address);
    int      (*Flash_Write)(uint32_t address, const uint8_t* data, uint32_t len);
    int      (*Flash_Read)(uint32_t address, uint8_t* data, uint32_t len);
    int      (*NVM_WriteConfig)(uint16_t id, const uint8_t* data, uint16_t len);
    int      (*NVM_ReadConfig)(uint16_t id, uint8_t* data, uint16_t len);
    
    /*===================================================================*/
    /*                      BOOTLOADER CONTROL                           */
    /*===================================================================*/
    void     (*System_Reset)(void);
    void     (*JumpToApplication)(uint32_t app_address);
    int      (*VerifyApplicationCRC)(uint32_t app_address, uint32_t size, uint32_t expected_crc);
    void     (*EnterBootloader)(void);
    
} __attribute__((packed)) HardwareAPI_t;

/* Pointer to the API table at its fixed flash address */
#define HW_API     ((const HardwareAPI_t*)API_TABLE_ADDR)

/*===================================================================*/
/*                    BOOTLOADER PUBLIC FUNCTIONS                     */
/*===================================================================*/

/* Main bootloader entry */
void Bootloader_Main(void);

/* Check if we should enter bootloader or jump to app */
int  Bootloader_ShouldStay(void);

/* Jump to application at given address */
void Bootloader_JumpToApp(uint32_t app_address);

/* Perform OTA firmware update */
int  Bootloader_ProcessOTA(void);

/* Verify application CRC */
int  Bootloader_VerifyApp(uint32_t app_address, uint32_t size);

/* Get pointer to shared RAM */
volatile SharedRAM_t* Bootloader_GetSharedRAM(void);

/* Bootloader error codes */
#define BL_OK               0
#define BL_ERR_CRC          -1
#define BL_ERR_FLASH        -2
#define BL_ERR_OTA_TIMEOUT  -3
#define BL_ERR_OTA_SIZE     -4
#define BL_ERR_NO_APP       -5
#define BL_ERR_INVALID      -6

#ifdef __cplusplus
}
#endif

#endif /* __BOOTLOADER_H */