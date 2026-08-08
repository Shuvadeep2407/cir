/**
  ******************************************************************************
  * @file    ota.c
  * @brief   Over-The-Air Firmware Update Module
  *          Supports GSM (primary) and CAN (secondary) channels
  ******************************************************************************
  */
#include "bootloader.h"
#include "ota.h"
#include "blink_driver.h"
#include "gsm_driver.h"
#include "can_driver.h"
#include "led_driver.h"

static OTA_State_t g_ota_state = OTA_STATE_IDLE;
static uint32_t g_ota_received = 0;
static uint32_t g_ota_total = 0;
static uint32_t g_ota_crc32 = 0;
static uint32_t g_ota_version = 0;
static uint8_t g_ota_source = 0;
static uint8_t g_ota_seq = 0;

/* Temporary buffer for OTA data before flashing */
#define OTA_TEMP_BUF_SIZE   1024
static uint8_t g_ota_buf[OTA_TEMP_BUF_SIZE];
static uint32_t g_ota_buf_offset = 0;

/**
  * @brief  Initialize OTA module
  * @param  source: OTA source (GSM, CAN, SPI)
  * @retval OTA_OK
  */
int OTA_Init(uint8_t source)
{
    g_ota_state = OTA_STATE_IDLE;
    g_ota_received = 0;
    g_ota_total = 0;
    g_ota_crc32 = 0;
    g_ota_version = 0;
    g_ota_source = source;
    g_ota_seq = 0;
    g_ota_buf_offset = 0;
    
    return OTA_OK;
}

/**
  * @brief  Start firmware transfer
  * @param  total_size: Total firmware size
  * @param  crc32: Expected CRC32
  * @param  version: Firmware version
  * @retval OTA_OK on success
  */
int OTA_StartTransfer(uint32_t total_size, uint32_t crc32, uint32_t version)
{
    if (total_size == 0 || total_size > APPLICATION_MAX_SIZE)
        return OTA_ERR_SIZE;
    
    g_ota_state = OTA_STATE_RECEIVING;
    g_ota_total = total_size;
    g_ota_crc32 = crc32;
    g_ota_version = version;
    g_ota_received = 0;
    g_ota_seq = 0;
    g_ota_buf_offset = 0;
    
    /* Store OTA info in shared RAM */
    volatile SharedRAM_t* ram = Bootloader_GetSharedRAM();
    ram->magic = SHARED_RAM_MAGIC;
    ram->ota_firmware_size = total_size;
    ram->ota_crc32 = crc32;
    ram->boot_flags |= BOOT_FLAG_UPDATE_IN_PROG;
    
    return OTA_OK;
}

/**
  * @brief  Receive a chunk of firmware data
  * @param  data: Data chunk
  * @param  len: Chunk length
  * @param  offset: Offset in firmware
  * @retval OTA_OK on success
  */
int OTA_ReceiveChunk(const uint8_t* data, uint16_t len, uint32_t offset)
{
    if (g_ota_state != OTA_STATE_RECEIVING)
        return OTA_ERR_INVALID;
    
    if (offset + len > g_ota_total)
        return OTA_ERR_SIZE;
    
    /* Buffer data until we have a full page to flash */
    for (uint16_t i = 0; i < len; i++)
    {
        g_ota_buf[g_ota_buf_offset++] = data[i];
        g_ota_received++;
        
        /* Flash when buffer is full or at end */
        if (g_ota_buf_offset >= OTA_TEMP_BUF_SIZE || g_ota_received >= g_ota_total)
        {
            uint32_t flash_addr = APPLICATION_BASE + (g_ota_received - g_ota_buf_offset);
            
            /* Align to 4 bytes */
            uint32_t aligned_len = (g_ota_buf_offset + 3) & ~3;
            while (g_ota_buf_offset < aligned_len)
                g_ota_buf[g_ota_buf_offset++] = 0xFF;
            
            if (Flash_Write(flash_addr, g_ota_buf, g_ota_buf_offset) != 0)
                return OTA_ERR_FLASH;
            
            g_ota_buf_offset = 0;
        }
    }
    
    return OTA_OK;
}

/**
  * @brief  End firmware transfer
  * @retval OTA_OK on success
  */
int OTA_EndTransfer(void)
{
    if (g_ota_state != OTA_STATE_RECEIVING)
        return OTA_ERR_INVALID;
    
    g_ota_state = OTA_STATE_VERIFYING;
    
    /* Verify CRC */
    uint32_t calc_crc = CRC32_CalculateFlash(APPLICATION_BASE, g_ota_total);
    if (calc_crc != g_ota_crc32)
    {
        g_ota_state = OTA_STATE_ERROR;
        return OTA_ERR_CRC;
    }
    
    g_ota_state = OTA_STATE_COMPLETE;
    
    /* Update shared RAM */
    volatile SharedRAM_t* ram = Bootloader_GetSharedRAM();
    ram->boot_flags |= BOOT_FLAG_APP_VALID;
    ram->boot_flags &= ~BOOT_FLAG_UPDATE_IN_PROG;
    ram->app_crc32 = g_ota_crc32;
    ram->app_version = g_ota_version;
    
    return OTA_OK;
}

/**
  * @brief  Abort firmware transfer
  * @retval OTA_OK
  */
int OTA_AbortTransfer(void)
{
    g_ota_state = OTA_STATE_ERROR;
    g_ota_received = 0;
    g_ota_total = 0;
    
    volatile SharedRAM_t* ram = Bootloader_GetSharedRAM();
    ram->boot_flags &= ~BOOT_FLAG_UPDATE_IN_PROG;
    
    return OTA_OK;
}

/**
  * @brief  Process an OTA packet
  * @param  pkt: OTA packet
  * @retval OTA_OK on success
  */
int OTA_ProcessPacket(OTA_Packet_t* pkt)
{
    if (!pkt) return OTA_ERR_INVALID;
    
    switch (pkt->type)
    {
        case OTA_PKT_START:
        {
            OTA_FirmwareHeader_t* hdr = (OTA_FirmwareHeader_t*)pkt->payload;
            if (hdr->magic != OTA_MAGIC)
                return OTA_ERR_INVALID;
            return OTA_StartTransfer(hdr->firmware_size, hdr->firmware_crc32, hdr->firmware_version);
        }
        
        case OTA_PKT_DATA:
            return OTA_ReceiveChunk(pkt->payload, pkt->length, pkt->offset);
        
        case OTA_PKT_END:
            return OTA_EndTransfer();
        
        case OTA_PKT_ABORT:
            return OTA_AbortTransfer();
        
        default:
            return OTA_ERR_INVALID;
    }
}

/**
  * @brief  Get OTA transfer progress
  * @param  received: Bytes received
  * @param  total: Total bytes
  * @retval OTA_OK
  */
int OTA_GetProgress(uint32_t* received, uint32_t* total)
{
    if (received) *received = g_ota_received;
    if (total) *total = g_ota_total;
    return OTA_OK;
}

/**
  * @brief  Get OTA state
  * @retval Current OTA state
  */
OTA_State_t OTA_GetState(void)
{
    return g_ota_state;
}

/**
  * @brief  Flash received firmware to application region
  * @retval OTA_OK on success
  */
int OTA_FlashFirmware(void)
{
    /* Already flashed during receive */
    return OTA_OK;
}

/**
  * @brief  Verify flashed firmware
  * @retval OTA_OK on success
  */
int OTA_VerifyFirmware(void)
{
    if (g_ota_total == 0) return OTA_ERR_INVALID;
    
    uint32_t calc = CRC32_CalculateFlash(APPLICATION_BASE, g_ota_total);
    if (calc != g_ota_crc32)
        return OTA_ERR_CRC;
    
    return OTA_OK;
}

/**
  * @brief  Receive firmware update via GSM
  *         Expects data over TCP/UDP connection
  * @retval OTA_OK on success
  */
int OTA_ReceiveFromGSM(void)
{
    LED_SetPattern(LED_PATTERN_SEQUENCE);
    
    /* Initialize OTA for GSM source */
    OTA_Init(OTA_SOURCE_GSM);
    
    /* Send AT command to start data mode */
    GSM_SendAT("AT+CIPSTART=\"TCP\",\"ota.server.com\",\"8080\"");
    if (GSM_WaitResponse("CONNECT", 30000) != GSM_OK)
        return OTA_ERR_TIMEOUT;
    
    /* Request firmware info */
    GSM_SendAT("AT+CIPSEND=16");
    GSM_WaitResponse(">", 5000);
    GSM_SendData((const uint8_t*)"GET_FW_INFO\r\n", 13);
    
    /* Wait for firmware header */
    uint8_t header_buf[64];
    int hdr_len = GSM_ReceiveData(header_buf, 64, 30000);
    if (hdr_len < (int)sizeof(OTA_FirmwareHeader_t))
        return OTA_ERR_TIMEOUT;
    
    OTA_FirmwareHeader_t* fw_hdr = (OTA_FirmwareHeader_t*)header_buf;
    if (fw_hdr->magic != OTA_MAGIC)
        return OTA_ERR_INVALID;
    
    /* Start transfer */
    int ret = OTA_StartTransfer(fw_hdr->firmware_size, fw_hdr->firmware_crc32, fw_hdr->firmware_version);
    if (ret != OTA_OK) return ret;
    
    /* Receive firmware data in chunks */
    uint8_t data_buf[OTA_MAX_PAYLOAD_SIZE];
    uint32_t remaining = fw_hdr->firmware_size;
    uint32_t offset = 0;
    
    while (remaining > 0)
    {
        LED_Toggle(LED1);
        
        uint16_t chunk = (remaining > OTA_MAX_PAYLOAD_SIZE) ? OTA_MAX_PAYLOAD_SIZE : (uint16_t)remaining;
        
        int recv = GSM_ReceiveData(data_buf, chunk, OTA_RX_TIMEOUT_MS);
        if (recv <= 0) return OTA_ERR_TIMEOUT;
        
        ret = OTA_ReceiveChunk(data_buf, (uint16_t)recv, offset);
        if (ret != OTA_OK) return ret;
        
        offset += recv;
        remaining -= recv;
        
        /* Send ACK */
        GSM_SendAT("AT+CIPSEND=4");
        GSM_WaitResponse(">", 5000);
        GSM_SendData((const uint8_t*)"ACK\r\n", 4);
    }
    
    /* End transfer */
    ret = OTA_EndTransfer();
    
    /* Close connection */
    GSM_SendAT("AT+CIPCLOSE");
    
    return ret;
}

/**
  * @brief  Receive firmware update via CAN bus
  *         Uses OTA protocol over CAN frames
  * @retval OTA_OK on success
  */
int OTA_ReceiveFromCAN(void)
{
    LED_SetPattern(LED_PATTERN_ALTERNATE);
    
    OTA_Init(OTA_SOURCE_CAN);
    
    OTA_Packet_t pkt;
    uint8_t can_data[8];
    uint8_t can_len;
    uint32_t can_id;
    int ret;
    
    /* Wait for OTA start packet */
    uint32_t timeout = 30000;
    uint32_t start = GetTick();
    
    while ((GetTick() - start) < timeout)
    {
        if (CAN_Receive(&can_id, can_data, &can_len, 1000) == CAN_OK)
        {
            if (can_id == CAN_OTA_START_ID && can_len >= 8)
            {
                /* Build OTA packet from CAN data */
                pkt.type = OTA_PKT_START;
                pkt.seq = 0;
                pkt.length = can_len;
                for (int i = 0; i < can_len && i < 8; i++)
                    pkt.payload[i] = can_data[i];
                
                ret = OTA_ProcessPacket(&pkt);
                if (ret != OTA_OK) return ret;
                
                /* Send ACK */
                uint8_t ack = 0x01;
                CAN_Transmit(CAN_OTA_ACK_ID, &ack, 1, 0);
                break;
            }
        }
    }
    
    if (g_ota_state != OTA_STATE_RECEIVING)
        return OTA_ERR_TIMEOUT;
    
    /* Receive data packets */
    while (g_ota_received < g_ota_total)
    {
        if (CAN_Receive(&can_id, can_data, &can_len, 5000) == CAN_OK)
        {
            if (can_id == CAN_OTA_DATA_ID && can_len >= 8)
            {
                pkt.type = OTA_PKT_DATA;
                pkt.seq = g_ota_seq++;
                pkt.length = can_len;
                pkt.offset = g_ota_received;
                for (int i = 0; i < can_len && i < 8; i++)
                    pkt.payload[i] = can_data[i];
                
                ret = OTA_ProcessPacket(&pkt);
                if (ret != OTA_OK) return ret;
                
                /* Send ACK every 16 packets */
                if ((g_ota_seq % 16) == 0)
                {
                    uint8_t ack = g_ota_seq;
                    CAN_Transmit(CAN_OTA_ACK_ID, &ack, 1, 0);
                }
                
                LED_Toggle(LED2);
            }
            else if (can_id == CAN_OTA_END_ID)
            {
                pkt.type = OTA_PKT_END;
                ret = OTA_ProcessPacket(&pkt);
                return ret;
            }
        }
        else
        {
            return OTA_ERR_TIMEOUT;
        }
    }
    
    return OTA_EndTransfer();
}