/**
  ******************************************************************************
  * @file    w5100.c
  * @brief   WIZnet W5100 Ethernet controller driver implementation.
  ******************************************************************************
  */

#include "w5100.h"
#include <string.h> // For memcpy

static w5100_io_t w5100_io;

/**
 * @brief Selects the W5100 chip (sets CS low).
 */
static void W5100_Select(void) {
    if (w5100_io.spi_select) {
        w5100_io.spi_select();
    }
}

/**
 * @brief Unselects the W5100 chip (sets CS high).
 */
static void W5100_Unselect(void) {
    if (w5100_io.spi_unselect) {
        w5100_io.spi_unselect();
    }
}

/**
 * @brief Reads a single byte from the W5100.
 * @return The byte read.
 */
static uint8_t W5100_SPI_ReadByte(void) {
    if (w5100_io.spi_read_byte) {
        return w5100_io.spi_read_byte();
    }
    return 0;
}

/**
 * @brief Writes a single byte to the W5100.
 * @param byte The byte to write.
 */
static void W5100_SPI_WriteByte(uint8_t byte) {
    if (w5100_io.spi_write_byte) {
        w5100_io.spi_write_byte(byte);
    }
}

/**
 * @brief Millisecond delay using the HAL-provided delay function.
 */
static void W5100_Delay(uint32_t ms) {
    if (w5100_io.delay_ms) {
        w5100_io.delay_ms(ms);
    }
}

void W5100_WriteByte(uint16_t addr, uint8_t data) {
    W5100_Select();
    W5100_SPI_WriteByte(0xF0); // W5100 write opcode
    W5100_SPI_WriteByte((addr & 0xFF00) >> 8);
    W5100_SPI_WriteByte(addr & 0x00FF);
    W5100_SPI_WriteByte(data);
    W5100_Unselect();
}

uint8_t W5100_ReadByte(uint16_t addr) {
    uint8_t data;
    W5100_Select();
    W5100_SPI_WriteByte(0x0F); // W5100 read opcode
    W5100_SPI_WriteByte((addr & 0xFF00) >> 8);
    W5100_SPI_WriteByte(addr & 0x00FF);
    data = W5100_SPI_ReadByte();
    W5100_Unselect();
    return data;
}

void W5100_WriteBuffer(uint16_t addr, const uint8_t *data, uint16_t len) {
    /* The W5100 accepts one 32-bit SPI command per byte.  Unlike newer
       W5x00 parts, it cannot stream multiple data bytes in one transaction. */
    for (uint16_t i = 0; i < len; i++) {
        W5100_WriteByte((uint16_t)(addr + i), data[i]);
    }
}

void W5100_ReadBuffer(uint16_t addr, uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        data[i] = W5100_ReadByte((uint16_t)(addr + i));
    }
}

int8_t W5100_Init(const w5100_io_t* io_fns) {
    if (io_fns) {
        memcpy(&w5100_io, io_fns, sizeof(w5100_io_t));
    }

    // Ensure delay function is provided
    if (!w5100_io.delay_ms) {
        return -1;
    }

    // Hardware reset if function is provided
    if (w5100_io.reset) {
        w5100_io.reset();
        W5100_Delay(10);
    }

    W5100_Reset();

    // W5100 has no VERSIONR register. Verify the read/write Mode Register.
    uint8_t retries = 3;
    while (retries--) {
        if (W5100_TestCommunication() == 0) {
            W5100_SetTxRxBufferSize(2, 2);
            return 0;
        }
        W5100_Delay(5);
    }
    return -1;
}

void W5100_Reset(void) {
    W5100_WriteByte(W5100_MR, W5100_MR_RST);
    W5100_Delay(10); // Wait for reset to complete
}

int8_t W5100_TestCommunication(void) {
    /* MR is reset to zero. PB is a documented, readable bit and is safe to
       set briefly, so this detects both read and write SPI failures. */
    if (W5100_ReadByte(W5100_MR) != 0x00) {
        return -1;
    }

    W5100_WriteByte(W5100_MR, W5100_MR_PB);
    if (W5100_ReadByte(W5100_MR) != W5100_MR_PB) {
        W5100_Reset();
        return -1;
    }

    W5100_WriteByte(W5100_MR, 0x00);
    return W5100_ReadByte(W5100_MR) == 0x00 ? 0 : -1;
}

void W5100_ConfigureNetwork(const w5100_net_config_t* net_config) {
    W5100_SetGateway(net_config->gateway);
    W5100_SetSubnetMask(net_config->subnet);
    W5100_SetMACAddress(net_config->mac);
    W5100_SetIPAddress(net_config->ip);
}

void W5100_SetMACAddress(const uint8_t *mac) {
    W5100_WriteBuffer(W5100_SHAR, mac, 6);
}

void W5100_SetIPAddress(const uint8_t *ip) {
    W5100_WriteBuffer(W5100_SIPR, ip, 4);
}

void W5100_SetGateway(const uint8_t *gw) {
    W5100_WriteBuffer(W5100_GAR, gw, 4);
}

void W5100_SetSubnetMask(const uint8_t *sub) {
    W5100_WriteBuffer(W5100_SUBR, sub, 4);
}

void W5100_GetNetworkInfo(w5100_net_config_t* net_config) {
    W5100_ReadBuffer(W5100_SHAR, net_config->mac, 6);
    W5100_ReadBuffer(W5100_SIPR, net_config->ip, 4);
    W5100_ReadBuffer(W5100_GAR, net_config->gateway, 4);
    W5100_ReadBuffer(W5100_SUBR, net_config->subnet, 4);
}

void W5100_SetTxRxBufferSize(uint8_t tx_size, uint8_t rx_size) {
    /* This driver uses fixed 2 KB circular buffers for all four sockets.
       In the W5100, buffer allocation is configured in the common TMSR/RMSR
       registers, not in per-socket registers. 0x55 = 2 KB per socket. */
    if (tx_size != 2 || rx_size != 2) {
        return;
    }

    W5100_WriteByte(W5100_TMSR, 0x55);
    W5100_WriteByte(W5100_RMSR, 0x55);
}

static uint16_t get_socket_reg_addr(uint8_t sock, uint16_t reg_offset) {
    return W5100_SOCKET_BASE + (sock * W5100_SOCKET_STRIDE) + reg_offset;
}

/**
 * @brief Waits for a socket command to complete with timeout.
 * @param sock Socket number.
 * @return 0 on success, -1 on timeout.
 */
static int8_t wait_for_cmd(uint8_t sock) {
    uint32_t timeout = W5100_CMD_TIMEOUT;
    while (W5100_ReadByte(get_socket_reg_addr(sock, W5100_Sn_CR)) != 0) {
        if (--timeout == 0) {
            return -1; // Command timed out
        }
    }
    return 0;
}

uint8_t W5100_SocketInit(uint8_t sock, uint8_t protocol, uint16_t port, uint8_t flag) {
    if (sock >= W5100_MAX_SOCKETS) return 0;

    // Set socket mode and flags
    W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_MR), protocol | flag);

    // Set socket port
    if (port != 0) {
        W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_PORT), (uint8_t)(port >> 8));
        W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_PORT) + 1, (uint8_t)(port));
    }

    // Issue OPEN command
    W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_CR), W5100_SNCR_OPEN);

    // Wait for command to complete with timeout
    if (wait_for_cmd(sock) != 0) {
        return 0;
    }

    // Check status
    if (W5100_SocketGetStatus(sock) != W5100_SNSR_INIT) {
        W5100_SocketClose(sock);
        return 0;
    }

    return 1;
}

uint8_t W5100_SocketConnect(uint8_t sock, const uint8_t *ip, uint16_t port) {
    if (sock >= W5100_MAX_SOCKETS) return 0;

    // Set destination IP and port
    W5100_WriteBuffer(get_socket_reg_addr(sock, W5100_Sn_DIPR), ip, 4);
    W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_DPORT), (uint8_t)(port >> 8));
    W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_DPORT) + 1, (uint8_t)(port));

    // Issue CONNECT command
    W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_CR), W5100_SNCR_CONNECT);

    // Wait for command to complete with timeout
    if (wait_for_cmd(sock) != 0) {
        return 0;
    }

    // Check for timeout
    if (W5100_ReadByte(get_socket_reg_addr(sock, W5100_Sn_IR)) & W5100_SNIR_TIMEOUT) {
        // Clear timeout interrupt flag
        W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_IR), W5100_SNIR_TIMEOUT);
        return 0; // Connection failed
    }

    return 1; // Connection in progress
}

uint8_t W5100_SocketListen(uint8_t sock) {
    if (sock >= W5100_MAX_SOCKETS) return 0;

    // Socket must be in INIT state first
    if (W5100_SocketGetStatus(sock) != W5100_SNSR_INIT) {
        // Try to open it first if it's not already
        W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_MR), W5100_SNMR_TCP);
        W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_CR), W5100_SNCR_OPEN);
        if (wait_for_cmd(sock) != 0) return 0;
    }

    W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_CR), W5100_SNCR_LISTEN);
    if (wait_for_cmd(sock) != 0) {
        return 0;
    }

    if (W5100_SocketGetStatus(sock) != W5100_SNSR_LISTEN) {
        W5100_SocketClose(sock);
        return 0;
    }
    return 1;
}

uint8_t W5100_SocketDisconnect(uint8_t sock) {
    if (sock >= W5100_MAX_SOCKETS) return 0;

    W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_CR), W5100_SNCR_DISCON);
    wait_for_cmd(sock);

    return 1;
}

uint8_t W5100_SocketClose(uint8_t sock) {
    if (sock >= W5100_MAX_SOCKETS) return 0;

    W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_CR), W5100_SNCR_CLOSE);
    wait_for_cmd(sock);

    // Clear interrupt flags
    W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_IR), 0xFF);

    return 1;
}

uint8_t W5100_SocketGetStatus(uint8_t sock) {
    if (sock >= W5100_MAX_SOCKETS) return W5100_SNSR_CLOSED;
    return W5100_ReadByte(get_socket_reg_addr(sock, W5100_Sn_SR));
}

uint16_t W5100_SocketGetTXFreeSize(uint8_t sock) {
    if (sock >= W5100_MAX_SOCKETS) return 0;
    uint16_t val = 0;
    val = W5100_ReadByte(get_socket_reg_addr(sock, W5100_Sn_TX_FSR));
    val = (val << 8) + W5100_ReadByte(get_socket_reg_addr(sock, W5100_Sn_TX_FSR) + 1);
    return val;
}

uint16_t W5100_SocketGetRXReceivedSize(uint8_t sock) {
    if (sock >= W5100_MAX_SOCKETS) return 0;
    uint16_t val = 0;
    val = W5100_ReadByte(get_socket_reg_addr(sock, W5100_Sn_RX_RSR));
    val = (val << 8) + W5100_ReadByte(get_socket_reg_addr(sock, W5100_Sn_RX_RSR) + 1);
    return val;
}

uint16_t W5100_SocketSend(uint8_t sock, const uint8_t *data, uint16_t len) {
    if (sock >= W5100_MAX_SOCKETS || len == 0) return 0;

    // Check free buffer size
    uint16_t free_size = W5100_SocketGetTXFreeSize(sock);
    if (len > free_size) {
        len = free_size; // Truncate data to fit
    }

    if (len > 0) {
        // Get current write pointer
        uint16_t write_ptr = W5100_ReadByte(get_socket_reg_addr(sock, W5100_Sn_TX_WR));
        write_ptr = (write_ptr << 8) + W5100_ReadByte(get_socket_reg_addr(sock, W5100_Sn_TX_WR) + 1);

        // Write data to the circular TX buffer, splitting at the wrap point.
        uint16_t tx_buf_addr = W5100_TX_BASE + (sock * W5100_SOCKET_BUF_SIZE);
        uint16_t tx_offset = write_ptr & W5100_SOCKET_BUF_MASK;
        uint16_t first_len = W5100_SOCKET_BUF_SIZE - tx_offset;
        if (first_len > len) {
            first_len = len;
        }
        W5100_WriteBuffer(tx_buf_addr + tx_offset, data, first_len);
        if (len > first_len) {
            W5100_WriteBuffer(tx_buf_addr, data + first_len, len - first_len);
        }

        // Update write pointer
        write_ptr += len;
        W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_TX_WR), (uint8_t)(write_ptr >> 8));
        W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_TX_WR) + 1, (uint8_t)write_ptr);

        // Issue SEND command
        W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_CR), W5100_SNCR_SEND);
        if (wait_for_cmd(sock) != 0) {
            return 0; // Send command timed out
        }
    }

    return len;
}

uint16_t W5100_SocketRecv(uint8_t sock, uint8_t *data, uint16_t len) {
    if (sock >= W5100_MAX_SOCKETS || len == 0) return 0;

    // Check received data size
    uint16_t recv_size = W5100_SocketGetRXReceivedSize(sock);
    if (recv_size == 0) return 0;

    if (len < recv_size) {
        recv_size = len;
    }

    if (recv_size > 0) {
        // Get current read pointer
        uint16_t read_ptr = W5100_ReadByte(get_socket_reg_addr(sock, W5100_Sn_RX_RD));
        read_ptr = (read_ptr << 8) + W5100_ReadByte(get_socket_reg_addr(sock, W5100_Sn_RX_RD) + 1);

        // Read data from the circular RX buffer, splitting at the wrap point.
        uint16_t rx_buf_addr = W5100_RX_BASE + (sock * W5100_SOCKET_BUF_SIZE);
        uint16_t rx_offset = read_ptr & W5100_SOCKET_BUF_MASK;
        uint16_t first_len = W5100_SOCKET_BUF_SIZE - rx_offset;
        if (first_len > recv_size) {
            first_len = recv_size;
        }
        W5100_ReadBuffer(rx_buf_addr + rx_offset, data, first_len);
        if (recv_size > first_len) {
            W5100_ReadBuffer(rx_buf_addr, data + first_len, recv_size - first_len);
        }

        // Update read pointer
        read_ptr += recv_size;
        W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_RX_RD), (uint8_t)(read_ptr >> 8));
        W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_RX_RD) + 1, (uint8_t)read_ptr);

        // Issue RECV command
        W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_CR), W5100_SNCR_RECV);
        if (wait_for_cmd(sock) != 0) {
            return 0; // RECV command timed out
        }
    }

    return recv_size;
}

void W5100_SocketSetDestIP(uint8_t sock, const uint8_t *ip) {
    if (sock >= W5100_MAX_SOCKETS) return;
    W5100_WriteBuffer(get_socket_reg_addr(sock, W5100_Sn_DIPR), ip, 4);
}

void W5100_SocketSetDestPort(uint8_t sock, uint16_t port) {
    if (sock >= W5100_MAX_SOCKETS) return;
    W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_DPORT), (uint8_t)(port >> 8));
    W5100_WriteByte(get_socket_reg_addr(sock, W5100_Sn_DPORT) + 1, (uint8_t)(port));
}
