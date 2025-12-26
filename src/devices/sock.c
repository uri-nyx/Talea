#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define DO_NOT_INCLUDE_RAYLIB
#include "net_compat.h"
#include "talea.h"
#undef DO_NOT_INCLUDE_RAYLIB
#else
#include "net_compat.h"
#include "talea.h"
#endif

// Initializes the socket API in windows. Returns true on success
bool NetworkInit()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "Failed to initialize WinSock.\n");
        return false;
    }
#else
    return true;
#endif
}

void NetworkDeinit()
{
#ifdef _WIN32
    WSACleanup();
#else
    return;
#endif
}

void Serial_Init(TerminalSerial *dev)
{
    memset(dev, 0, sizeof(TerminalSerial));
    dev->server_fd       = T_INVALID_SOCKET;
    dev->host_socket     = T_INVALID_SOCKET;
    dev->is_command_mode = true;
}

void Serial_HostInit(TerminalSerial *dev, int port)
{
    // Create the socket
    dev->server_fd   = socket(AF_INET, SOCK_STREAM, 0);
    dev->host_socket = T_INVALID_SOCKET;

    if (dev->server_fd == T_INVALID_SOCKET) {
        perror("Serial Host: Could not create socket");
        printf("Socket fd was: %d\n", dev->server_fd);
        return;
    }

    // Set port reuse so you can restart the emulator quickly //TODO:check if
    // this works in linux, setsocopt taks int as opt parameter
    int opt = 1;
    if (setsockopt(dev->server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt,
                   sizeof(opt)) == T_NET_ERROR) {
        perror("Serial Host: setsockopt failed");
    }

    struct sockaddr_in server_addr = { 0 };
    server_addr.sin_family         = AF_INET;
    server_addr.sin_addr.s_addr    = INADDR_ANY; // Listen on localhost
    server_addr.sin_port           = htons(port);

    if (bind(dev->server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == T_NET_ERROR) {
        perror("Serial Host: Bind failed");
        net_close(dev->server_fd);
        dev->server_fd = T_INVALID_SOCKET;
        return;
    }

    if (listen(dev->server_fd, 1) == T_NET_ERROR) {
        perror("Serial Host: Listen failed");
        return;
    }

    // Set to non-blocking so the emulator doesn't freeze while waiting for a
    // user
#if defined(_WIN32)
    u_long mode = 1;
    ioctlsocket(dev->server_fd, FIONBIO, &mode);
#else
    int flags = fcntl(dev->server_fd, F_GETFL, 0);
    fcntl(dev->server_fd, F_SETFL, flags | O_NONBLOCK);
#endif

    printf("Serial Host: Virtual COM port listening on localhost:%d\n", port);
}

void Serial_Update(TaleaMachine *m)
{
    // 1. Check for a new connection if we don't have one
    TerminalSerial *dev = &m->terminal.serial;

    if (dev->host_socket == T_INVALID_SOCKET) {
        talea_net_t new_socket = accept(dev->server_fd, NULL, NULL);

        if (new_socket != T_INVALID_SOCKET) {
            dev->host_socket = new_socket;
            dev->status |= SER_STATUS_CARRIER_DETECT;
            printf("Serial Host: Terminal connected!\n");

// Set the new connection to non-blocking too
#if defined(_WIN32)
            u_long mode = 1;
            ioctlsocket(dev->host_socket, FIONBIO, &mode);
#else
            fcntl(dev->host_socket, F_SETFL, O_NONBLOCK);
#endif
        } else {
            if (!net_would_block())
                TALEA_LOG_ERROR("ERROR connecting to host socket\n");
        }
    }

    // 2. Read incoming data from the Host Terminal
    if (dev->host_socket != T_INVALID_SOCKET) {
        u8  temp_buffer[128];
        int n =
            recv(dev->host_socket, (char *)temp_buffer, sizeof(temp_buffer), 0);

        if (n > 0) {
            for (int i = 0; i < n; i++) {
                // Push to the hardware FIFO
                u8 next = (dev->head + 1) % SERIAL_FIFO_SIZE;
                if (next != dev->tail) {
                    dev->rx_fifo[dev->head] = temp_buffer[i];
                    dev->head               = next;
                }
            }

            // Fire the Priority 4 Interrupt
            dev->status |= SER_STATUS_DATA_AVAILABLE;
            if (dev->control & SER_CONTROL_INT_EN)
                Machine_RaiseInterrupt(m, INT_TTY_TRANSMIT,
                                       PRIORITY_SERIAL_INTERRUPT);
            TALEA_LOG_TRACE("Received serial data: %d bytes, (%x, %c)\n", n,
                            dev->rx_fifo[dev->tail], dev->rx_fifo[dev->tail]);
        } else if (n == 0) {
            // Connection closed by host
            net_close(dev->host_socket);
            dev->host_socket = T_INVALID_SOCKET;
            dev->status &= ~SER_STATUS_CARRIER_DETECT; // Clear Carrier Detect
        } else if (!net_would_block()) {
            // A real network error occurred
            net_close(dev->host_socket);
            dev->host_socket = T_INVALID_SOCKET;
            TALEA_LOG_ERROR("ERROR Host disconnected, net error\n");
        }
    }
}

void Serial_CloseSockets(TaleaMachine *m)
{
    if (m->terminal.serial.host_socket != INVALID_SOCKET) {
        net_close(m->terminal.serial.host_socket);
        m->terminal.serial.host_socket = INVALID_SOCKET;
    }
}

void Serial_SendByte(TaleaMachine *m, u8 byte)
{
    if (m->terminal.serial.host_socket != INVALID_SOCKET) {
        send(m->terminal.serial.host_socket, &byte, 1, 0);
    }
}