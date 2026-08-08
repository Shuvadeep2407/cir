/**
  ******************************************************************************
  * @file    bootloader_api.h
  * @brief   Header for accessing the bootloader's shared API and RAM.
  *          This file must be consistent between the bootloader and application.
  ******************************************************************************
  */

#ifndef BOOTLOADER_API_H
#define BOOTLOADER_API_H

#include <stdint.h>

/* Define the memory layout. These must match the bootloader's layout. */
#define APPLICATION_BASE         0x08008000U  /* Application start address */
#define BOOTLOADER_API_ADDRESS   0x08007F00U  /* Fixed address of the API table */
#define SHARED_RAM_BASE          0x2000FFF0U  /* Fixed address for shared RAM */

/*
 * @brief Hardware API function table provided by the bootloader.
 * The application can call these functions via this struct.
 * The layout of this struct must be IDENTICAL to the one in the bootloader.
 */
typedef struct {
    uint32_t version;
    uint16_t api_size;

    /* GSM */
    void (*GSM_Init)(uint32_t);
    void (*GSM_SendData)(const uint8_t*, uint16_t);
    int (*GSM_ReceiveData)(uint8_t*, uint16_t, uint32_t);
    int (*GSM_SendAT)(const char*);
    int (*GSM_WaitResponse)(const char*, uint32_t);
    int (*GSM_SendPayload)(const uint8_t*, uint32_t);
    int (*GSM_CheckNetwork)(void);
    void (*GSM_PowerOff)(void);

    /* CAN */
    void (*CAN_Init)(uint32_t);
    int (*CAN_Transmit)(uint32_t, const uint8_t*, uint8_t, int);
    int (*CAN_Receive)(uint32_t*, uint8_t*, uint8_t*, uint32_t);
    void (*CAN_SetFilter)(uint32_t, uint32_t);
    int (*CAN_CheckBusOff)(void);
    void (*CAN_ResetBus)(void);

    /* SPI Radio */
    void (*SPI_Radio_Init)(void);
    void (*SPI_Radio_WriteReg)(uint8_t, uint8_t);
    uint8_t (*SPI_Radio_ReadReg)(uint8_t);
    void (*SPI_Radio_SendPacket)(const uint8_t*, uint8_t);
    int (*SPI_Radio_ReceivePacket)(uint8_t*, uint8_t, uint32_t);
    void (*SPI_Radio_SetFrequency)(uint32_t);
    int (*SPI_Radio_CheckChannel)(void);

    /* Stepper */
    void (*Stepper_Init)(void);
    void (*Stepper_Move)(int32_t, uint8_t);
    void (*Stepper_SetSpeed)(uint16_t);
    void (*Stepper_Stop)(void);
    void (*Stepper_Enable)(uint8_t);
    int32_t (*Stepper_GetPosition)(void);
    void (*Stepper_SetPosition)(int32_t);
    int (*Stepper_IsMoving)(void);

    /* I2C */
    void (*I2C_Init)(uint32_t);
    int (*I2C_Write)(uint16_t, const uint8_t*, uint16_t);
    int (*I2C_Read)(uint16_t, uint8_t*, uint16_t);
    int (*I2C_WriteReg)(uint16_t, uint8_t, const uint8_t*, uint16_t);
    int (*I2C_ReadReg)(uint16_t, uint8_t, uint8_t*, uint16_t);
    int (*I2C_IsDeviceReady)(uint16_t, uint32_t);
    void (*I2C_ScanBus)(void (*callback)(uint8_t addr));

    /* I2S Audio */
    void (*I2S_Audio_Init)(uint32_t);
    void (*I2S_Audio_StartDMA)(const uint16_t*, uint32_t);
    void (*I2S_Audio_StopDMA)(void);
    void (*I2S_Audio_SetVolume)(uint8_t);
    int (*I2S_Audio_IsPlaying)(void);

    /* LED */
    void (*LED_Init)(void);
    void (*LED_On)(uint8_t);
    void (*LED_Off)(uint8_t);
    void (*LED_Toggle)(uint8_t);
    void (*LED_SetPattern)(uint8_t);
    void (*LED_Blink)(uint8_t, uint16_t, uint8_t);

    /* Timing */
    void (*Delay_ms)(uint32_t);
    uint32_t (*GetTick)(void);
    void (*Watchdog_Refresh)(void);

    /* Flash/NVM */
    int (*Flash_ErasePage)(uint32_t);
    int (*Flash_Write)(uint32_t, const uint8_t*, uint32_t);
    void (*Flash_Read)(uint32_t, uint8_t*, uint32_t);
    int (*NVM_WriteConfig)(const void*, uint32_t);
    int (*NVM_ReadConfig)(void*, uint32_t);

    /* System */
    void (*System_Reset)(void);
    void (*JumpToApplication)(uint32_t);
    int (*VerifyApplicationCRC)(uint32_t, uint32_t, uint32_t);
    void (*EnterBootloader)(void);
} HardwareAPI_t;

#endif // BOOTLOADER_API_H