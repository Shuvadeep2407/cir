/**
  ******************************************************************************
  * @file    bootloader.c
  * @brief   Main Bootloader Logic - Startup decision, OTA, app jump
  ******************************************************************************
  */
#include "bootloader.h"
#include "led_driver.h"
#include "blink_driver.h"
#include "ota.h"
#include "gsm_driver.h"
#include "can_driver.h"
#include "spi_radio_driver.h"
#include "stepper_driver.h"
#include "i2c_driver.h"
#include "i2s_audio_driver.h"
#include "stm32c0xx.h"

static int Bootloader_VerifyApplicationCRC(uint32_t address,
                                           uint32_t size,
                                           uint32_t expected_crc)
{
    uint32_t calculated = CRC32_CalculateFlash(address, size);
    return (calculated == expected_crc) ? BL_OK : BL_ERR_CRC;
}

/* External API table - placed at fixed address */
__attribute__((section(".api_table"))) const HardwareAPI_t g_hw_api = {
    .version                = BOOTLOADER_VERSION,
    .api_size               = sizeof(HardwareAPI_t),
    
    /* GSM */
    .GSM_Init               = &GSM_Init,
    .GSM_SendData           = &GSM_SendData,
    .GSM_ReceiveData        = &GSM_ReceiveData,
    .GSM_SendAT             = &GSM_SendAT,
    .GSM_WaitResponse       = &GSM_WaitResponse,
    .GSM_SendPayload        = &GSM_SendPayload,
    .GSM_CheckNetwork       = &GSM_CheckNetwork,
    .GSM_PowerOff           = &GSM_PowerOff,
    
    /* CAN */
    .CAN_Init               = &CAN_Init,
    .CAN_Transmit           = &CAN_Transmit,
    .CAN_Receive            = &CAN_Receive,
    .CAN_SetFilter          = &CAN_SetFilter,
    .CAN_CheckBusOff        = &CAN_CheckBusOff,
    .CAN_ResetBus           = &CAN_ResetBus,
    
    /* SPI Radio */
    .SPI_Radio_Init         = &SPI_Radio_Init,
    .SPI_Radio_WriteReg     = &SPI_Radio_WriteReg,
    .SPI_Radio_ReadReg      = &SPI_Radio_ReadReg,
    .SPI_Radio_SendPacket   = &SPI_Radio_SendPacket,
    .SPI_Radio_ReceivePacket= &SPI_Radio_ReceivePacket,
    .SPI_Radio_SetFrequency = &SPI_Radio_SetFrequency,
    .SPI_Radio_CheckChannel = &SPI_Radio_CheckChannel,
    
    /* Stepper */
    .Stepper_Init           = &Stepper_Init,
    .Stepper_Move           = &Stepper_Move,
    .Stepper_SetSpeed       = &Stepper_SetSpeed,
    .Stepper_Stop           = &Stepper_Stop,
    .Stepper_Enable         = &Stepper_Enable,
    .Stepper_GetPosition    = &Stepper_GetPosition,
    .Stepper_SetPosition    = &Stepper_SetPosition,
    .Stepper_IsMoving       = &Stepper_IsMoving,
    
    /* I2C */
    .I2C_Init               = &I2C_Init,
    .I2C_Write              = &I2C_Write,
    .I2C_Read               = &I2C_Read,
    .I2C_WriteReg           = &I2C_WriteReg,
    .I2C_ReadReg            = &I2C_ReadReg,
    .I2C_IsDeviceReady      = &I2C_IsDeviceReady,
    .I2C_ScanBus            = &I2C_ScanBus,
    
    /* I2S Audio */
    .I2S_Audio_Init         = &I2S_Audio_Init,
    .I2S_Audio_StartDMA     = &I2S_Audio_StartDMA,
    .I2S_Audio_StopDMA      = &I2S_Audio_StopDMA,
    .I2S_Audio_SetVolume    = &I2S_Audio_SetVolume,
    .I2S_Audio_IsPlaying    = &I2S_Audio_IsPlaying,
    
    /* LED */
    .LED_Init               = &LED_Init,
    .LED_On                 = &LED_On,
    .LED_Off                = &LED_Off,
    .LED_Toggle             = &LED_Toggle,
    .LED_SetPattern         = &LED_SetPattern,
    .LED_Blink              = &LED_Blink,
    
    /* Timing */
    .Delay_ms               = &Delay_ms,
    .GetTick                = &GetTick,
    .Watchdog_Refresh       = &Watchdog_Refresh,
    
    /* Flash/NVM */
    .Flash_ErasePage        = &Flash_ErasePage,
    .Flash_Write            = &Flash_Write,
    .Flash_Read             = &Flash_Read,
    .NVM_WriteConfig        = &NVM_WriteConfig,
    .NVM_ReadConfig         = &NVM_ReadConfig,
    
    /* System */
    .System_Reset           = &System_Reset,
    .JumpToApplication      = &JumpToApplication,
    .VerifyApplicationCRC   = &Bootloader_VerifyApplicationCRC,
    .EnterBootloader        = &System_Reset
};

/* Shared RAM pointer at fixed address */
volatile SharedRAM_t* const g_shared_ram = (volatile SharedRAM_t*)SHARED_RAM_BASE;

/**
  * @brief  Get pointer to shared RAM
  * @retval Pointer to shared RAM structure
  */
volatile SharedRAM_t* Bootloader_GetSharedRAM(void)
{
    return g_shared_ram;
}

/**
  * @brief  Check if bootloader should stay (or jump to app)
  * @retval 1 if stay, 0 if jump to app
  */
int Bootloader_ShouldStay(void)
{
    /* Check shared RAM for forced bootloader entry */
    if (g_shared_ram->magic == SHARED_RAM_MAGIC)
    {
        if (g_shared_ram->boot_flags & BOOT_FLAG_FORCE_BOOTLOADER)
        {
            g_shared_ram->boot_flags &= ~BOOT_FLAG_FORCE_BOOTLOADER;
            return 1;
        }
        if (g_shared_ram->boot_flags & BOOT_FLAG_OTA_PENDING)
        {
            return 1;
        }
        if (g_shared_ram->boot_flags & BOOT_FLAG_FACTORY_RESET)
        {
            return 1;
        }
    }
    
    /* Check if OTA update flag was set in shared RAM before reset */
    if (g_shared_ram->magic == SHARED_RAM_MAGIC)
    {
        if (g_shared_ram->boot_flags & BOOT_FLAG_UPDATE_IN_PROG)
        {
            return 1;
        }
    }
    
    /* Check for valid application at APPLICATION_BASE */
    uint32_t* app_stack = (uint32_t*)APPLICATION_BASE;
    uint32_t* app_reset = (uint32_t*)(APPLICATION_BASE + 4);
    
    /* Stack pointer should be in RAM range, reset vector should be in flash */
    if (*app_stack < 0x20000000 || *app_stack > 0x20010000)
        return 1;
    if (*app_reset < APPLICATION_BASE || *app_reset > (APPLICATION_BASE + APPLICATION_MAX_SIZE))
        return 1;
    
    /* Verify application CRC if available */
    if (g_shared_ram->magic == SHARED_RAM_MAGIC)
    {
        if (g_shared_ram->boot_flags & BOOT_FLAG_APP_VALID)
            return 0;
    }
    
    /* Application looks valid, jump to it */
    return 0;
}

/**
  * @brief  Verify application CRC in flash
  * @param  app_address: Start address of application
  * @param  size: Size of application in bytes
  * @retval BL_OK if CRC matches, error otherwise
  */
int Bootloader_VerifyApp(uint32_t app_address, uint32_t size)
{
    if (size == 0 || size > APPLICATION_MAX_SIZE)
        return BL_ERR_INVALID;
    
    uint32_t expected_crc = 0;
    
    /* Check if we have a stored CRC from OTA */
    if (g_shared_ram->magic == SHARED_RAM_MAGIC)
    {
        expected_crc = g_shared_ram->app_crc32;
    }
    
    /* Calculate CRC from flash */
    uint32_t calculated = CRC32_CalculateFlash(app_address, size);
    
    if (expected_crc != 0 && calculated != expected_crc)
        return BL_ERR_CRC;
    
    /* Mark app as valid in shared RAM */
    g_shared_ram->magic = SHARED_RAM_MAGIC;
    g_shared_ram->boot_flags |= BOOT_FLAG_APP_VALID;
    g_shared_ram->app_crc32 = calculated;
    
    return BL_OK;
}

/**
  * @brief  Jump to application at given address
  * @param  app_address: Address of application
  */
void Bootloader_JumpToApp(uint32_t app_address)
{
    /* Disable all interrupts before jumping */
    __disable_irq();
    
    /* Disable SysTick */
    SysTick->CTRL = 0;
    
    /* Clear pending interrupts */
    NVIC->ICPR[0] = 0xFFFFFFFF;
    
    /* Set vector table to application */
    SCB->VTOR = app_address;
    
    /* Set main stack pointer from application vector table */
    __set_MSP(*(volatile uint32_t*)app_address);
    
    /* Get application reset handler address */
    uint32_t app_reset = *(volatile uint32_t*)(app_address + 4);
    
    /* Create function pointer and jump */
    void (*app_main)(void) = (void (*)(void))app_reset;
    app_main();
    
    /* Should never reach here */
    while(1);
}

/**
  * @brief  Main bootloader entry point
  *         Called from Reset_Handler after system init
  */
void Bootloader_Main(void)
{
    /* Initialize hardware */
    Tick_Init();
    LED_Init();
    
    /* Show bootloader indicator sequence */
    LED_BootloaderIndicator();
    
    /* Initialize all hardware peripherals */
    SPI_Radio_Init();
    CAN_Init(CAN_DEFAULT_BITRATE);
    I2C_Init(I2C_DEFAULT_FREQ);
    I2S_Audio_Init(AUDIO_DEFAULT_SAMPLE_RATE);
    Stepper_Init();
    
    /* Initialize GSM (takes time for module to boot) */
    GSM_Init(GSM_DEFAULT_BAUD);
    
    /* Check if we should process OTA update */
    if (Bootloader_ShouldStay())
    {
        LED_SetPattern(LED_PATTERN_SEQUENCE);
        
        int ota_result = Bootloader_ProcessOTA();
        
        if (ota_result == BL_OK)
        {
            /* Verify the flashed application */
            if (g_shared_ram->magic == SHARED_RAM_MAGIC)
            {
                uint32_t app_size = g_shared_ram->ota_firmware_size;
                if (Bootloader_VerifyApp(APPLICATION_BASE, app_size) == BL_OK)
                {
                    g_shared_ram->boot_flags |= BOOT_FLAG_APP_VALID;
                    g_shared_ram->boot_flags &= ~BOOT_FLAG_OTA_PENDING;
                    g_shared_ram->boot_flags &= ~BOOT_FLAG_UPDATE_IN_PROG;
                }
            }
        }
        else
        {
            /* OTA failed - blink error */
            LED_Error((uint8_t)(-ota_result));
            Delay_ms(2000);
        }
    }
    
    /* Verify application integrity */
    if (g_shared_ram->magic == SHARED_RAM_MAGIC)
    {
        if (!(g_shared_ram->boot_flags & BOOT_FLAG_APP_VALID))
        {
            /* Try to verify existing app */
            Bootloader_VerifyApp(APPLICATION_BASE, APPLICATION_MAX_SIZE);
        }
    }
    
    /* All LEDs on briefly before jump */
    LED_On(LED1);
    LED_On(LED2);
    LED_On(LED3);
    LED_On(LED4);
    Delay_ms(100);
    LED_Off(LED1);
    LED_Off(LED2);
    LED_Off(LED3);
    LED_Off(LED4);
    
    /* Jump to main application */
    Bootloader_JumpToApp(APPLICATION_BASE);
}

/**
  * @brief  Process OTA firmware update
  * @retval OTA result code
  */
int Bootloader_ProcessOTA(void)
{
    if (g_shared_ram->magic != SHARED_RAM_MAGIC)
        return BL_ERR_INVALID;
    
    int result = BL_ERR_OTA_TIMEOUT;
    
    /* Determine OTA source */
    switch (g_shared_ram->ota_source)
    {
        case OTA_SOURCE_GSM:
            /* Try GSM first */
            if (GSM_CheckNetwork() == GSM_OK)
            {
                result = OTA_ReceiveFromGSM();
            }
            break;
            
        case OTA_SOURCE_CAN:
            /* Try CAN bus */
            result = OTA_ReceiveFromCAN();
            break;
            
        case OTA_SOURCE_SPI:
            /* SPI/RF OTA not implemented for large firmware */
            result = BL_ERR_INVALID;
            break;
            
        default:
            /* No specific source - try all channels */
            /* First try CAN (fast, local) */
            result = OTA_ReceiveFromCAN();
            if (result == OTA_OK) break;
            
            /* Then try GSM (slow, remote) */
            if (GSM_CheckNetwork() == GSM_OK)
            {
                result = OTA_ReceiveFromGSM();
            }
            break;
    }
    
    if (result == OTA_OK)
    {
        return BL_OK;
    }
    
    return BL_ERR_OTA_TIMEOUT;
}
