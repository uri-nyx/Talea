#include <ctype.h>
#include <stdio.h>
#include <string.h>

// NOTE: include always before talea.h

#ifdef _WIN32
#define DO_NOT_INCLUDE_RAYLIB
#include "terminal.h"

#include "net_compat.h"
#include "talea.h"
#undef DO_NOT_INCLUDE_RAYLIB
#else
#include "terminal.h"

#include "net_compat.h"
#include "talea.h"
#endif

/*----------------------------------------------------------------------------*/
/* STATIC GLOBALS                                                             */
/*----------------------------------------------------------------------------*/

// Put statics here

/*----------------------------------------------------------------------------*/
/* INTERNAL DEVICE FUNCIONS: declare as static inline                         */
/*----------------------------------------------------------------------------*/

static inline int Serial_GetFifoUsed(TerminalSerial *dev)
{
    if (dev->head >= dev->tail) {
        return dev->head - dev->tail;
    } else {
        return (TERMINAL_SERIAL_FIFO_SIZE - dev->tail) + dev->head;
    }
}

static inline int Serial_GetFifoSpace(TerminalSerial *dev)
{
    return (TERMINAL_SERIAL_FIFO_SIZE - 1) - Serial_GetFifoUsed(dev);
}

void Serial_CloseSockets(TaleaMachine *m)
{
    if (m->terminal.serial.hostSocket != T_INVALID_SOCKET) {
        net_close(m->terminal.serial.hostSocket);
        m->terminal.serial.hostSocket = T_INVALID_SOCKET;
        m->terminal.serial.status &= ~TERMINAL_SER_STATUS_CARRIER_DETECT;
    }
}

void Serial_SendByte(TaleaMachine *m, u8 byte)
{
    TerminalHayesModem *modem = &m->terminal.serial.modem;

    // TALEA_LOG_TRACE("Sending byte %02x\n", byte);

    switch (modem->state) {
    case TERMINAL_MODEM_STATE_COMMAND: Modem_ProcessCommand(m, byte); break;
    case TERMINAL_MODEM_STATE_DIALING: {
        // Abort connection
        Serial_CloseSockets(m);
        Modem_SendResponse(m, TERMINAL_HAYES_ERROR);
        break;
    }
    case TERMINAL_MODEM_STATE_DATA: Modem_ProcessData(m, byte); break;
    case TERMINAL_MODEM_STATE_DISCONNECTING: break;
    }

    Modem_CheckEscapeSequence(m, byte);
}

/*----------------------------------------------------------------------------*/
/* INTERFACE IMPLEMENTATION                                                   */
/*----------------------------------------------------------------------------*/

void Serial_PushByte(TaleaMachine *m, u8 byte)
{
    TerminalSerial *s    = &m->terminal.serial;
    u8              next = (s->head + 1) % TERMINAL_SERIAL_FIFO_SIZE;

    if (next != s->tail) {
        s->rxFifo[s->head] = byte;
        s->head            = next;

        s->status |= TERMINAL_SER_STATUS_DATA_AVAILABLE;

        if (s->control & TERMINAL_SER_CONTROL_INT_EN)
            Machine_RaiseInterrupt(m, INT_SER_RX, PRIORITY_SERIAL_INTERRUPT);
    } else {
        s->status |= TERMINAL_SER_STATUS_BUFFER_OVERRUN;
    }
}

void Serial_PushString(TaleaMachine *m, const u8 *str)
{
    TerminalSerial *s          = &m->terminal.serial;
    bool            data_added = false;

    while (*str) {
        u8 next = (s->head + 1) % TERMINAL_SERIAL_FIFO_SIZE;
        if (next != s->tail) {
            s->rxFifo[s->head] = (u8)(*str);
            s->head            = next;
            data_added         = true;
        } else {
            TALEA_LOG_WARNING("Pushing to Serial buffer: buffer overrun\n");
            s->status |= TERMINAL_SER_STATUS_BUFFER_OVERRUN;
            break;
        }
        str++;
    }

    if (data_added) {
        s->status |= TERMINAL_SER_STATUS_DATA_AVAILABLE;

        if (s->control & TERMINAL_SER_CONTROL_INT_EN) {
            Machine_RaiseInterrupt(m, INT_SER_RX, PRIORITY_SERIAL_INTERRUPT);
        }
    }
}

u8 Serial_PopByte(TerminalSerial *s)
{
    u8 value = 0;

    if (s->head != s->tail) {
        value = s->rxFifo[s->tail++];
        if (s->head == s->tail) s->status &= ~TERMINAL_SER_STATUS_DATA_AVAILABLE;
    }

    return value;
}

static inline u8 Serial_PeekByte(TerminalSerial *s)
{
    if (s->head == s->tail) {
        s->status &= ~TERMINAL_SER_STATUS_DATA_AVAILABLE;
        return 0;
    }
    return s->rxFifo[s->tail];
}

// Callbacks for Raylib's AudioStream

// DEVICE INITIALIZATION

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
    dev->serverFd    = T_INVALID_SOCKET;
    dev->hostSocket  = T_INVALID_SOCKET;
    dev->modem.state = TERMINAL_MODEM_STATE_COMMAND;

    Modem_ResetSregs(&dev->modem);
}

void Serial_HostInit(TerminalSerial *dev, int port)
{
    // Create the socket
    dev->serverFd   = socket(AF_INET, SOCK_STREAM, 0);
    dev->hostSocket = T_INVALID_SOCKET;

    if (dev->serverFd == T_INVALID_SOCKET) {
        perror("Serial Host: Could not create socket");
        printf("Socket fd was: %d\n", dev->serverFd);
        return;
    }

    int opt = 1;
    if (setsockopt(dev->serverFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) ==
        T_NET_ERROR) {
        perror("Serial Host: setsockopt failed");
    }

    struct sockaddr_in server_addr = { 0 };
    server_addr.sin_family         = AF_INET;
    server_addr.sin_addr.s_addr    = INADDR_ANY; // Listen on localhost
    server_addr.sin_port           = htons(port);

    if (bind(dev->serverFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == T_NET_ERROR) {
        perror("Serial Host: Bind failed");
        net_close(dev->serverFd);
        dev->serverFd = T_INVALID_SOCKET;
        return;
    }

    if (listen(dev->serverFd, 1) == T_NET_ERROR) {
        perror("Serial Host: Listen failed");
        return;
    }

    net_set_nonblocking(dev->serverFd);

    printf("Serial Host: Virtual COM port listening on localhost:%d\n", port);
}

// DEVICE UPDATE (on vblank)

void Serial_Update(TaleaMachine *m)
{
    TerminalSerial     *dev   = &m->terminal.serial;
    TerminalHayesModem *modem = &dev->modem;

    // Accept new connections
    if (modem->state == TERMINAL_MODEM_STATE_COMMAND && dev->hostSocket == T_INVALID_SOCKET) {
        talea_net_t new_socket = accept(dev->serverFd, NULL, NULL);

        if (new_socket != T_INVALID_SOCKET) {
            dev->pendingSocket = new_socket;
            net_set_nonblocking(dev->pendingSocket);
            printf("Serial Host: Incoming call!\n");
            modem->isRinging    = true;
            modem->ringCount    = 0;
            modem->sRegs[1]     = 0; // ring count sreg
            modem->lastRingTime = GetTime();
        } else if (!net_would_block()) {
            TALEA_LOG_ERROR("ERROR connecting to host socket\n");
        }
    }

    // Read incoming data from terminal or network
    if (dev->hostSocket != T_INVALID_SOCKET && modem->state != TERMINAL_MODEM_STATE_DIALING) {
        double now          = GetTime();
        double delta_time   = now - dev->lastUpdateTime;
        dev->lastUpdateTime = now;

        // check if baud rate valid otherwise default to 9600
        int baud_rate = (modem->sRegs[37] >= TERMINAL_HAYES_BAUD_TOTAL) ? TERMINAL_HAYES_BAUD_9600 :
                                                                          modem->sRegs[37];
        // Clamp to baud rate / 10 bits per byte
        dev->byteCredit += (TerminalHayesBaudRateLookup[baud_rate] / 10.0) * delta_time;

        if (dev->byteCredit >= 1.0) {
            int fifo_space  = Serial_GetFifoSpace(dev);
            int max_to_read = (int)dev->byteCredit;
            if (max_to_read > fifo_space) max_to_read = fifo_space;
            if (max_to_read > 128) max_to_read = 128;

            if (max_to_read > 0) {
                u8  buffer[128];
                int n = recv(dev->hostSocket, (char *)buffer, max_to_read, 0);
                if (n > 0) {
                    for (int i = 0; i < n; i++) {
                        u8 next = (dev->head + 1) % TERMINAL_SERIAL_FIFO_SIZE;
                        if (next != dev->tail) {
                            dev->rxFifo[dev->head] = buffer[i];
                            dev->head              = next;
                        } else {
                            dev->status |= TERMINAL_SER_STATUS_BUFFER_OVERRUN;
                            break;
                        }
                    }
                    dev->byteCredit -= n;

                    // Fire the Priority 4 Interrupt
                    dev->status |= TERMINAL_SER_STATUS_DATA_AVAILABLE;
                    if (dev->control & TERMINAL_SER_CONTROL_INT_EN)
                        Machine_RaiseInterrupt(m, INT_SER_RX, PRIORITY_SERIAL_INTERRUPT);

                } else if (n == 0 || (n < 0 && !net_would_block())) {
                    // Disconnect

                    net_close(dev->hostSocket);
                    dev->hostSocket = T_INVALID_SOCKET;
                    dev->status &= ~TERMINAL_SER_STATUS_CARRIER_DETECT;

                    if (modem->state == TERMINAL_MODEM_STATE_DATA) {
                        // The BBS hung up
                        modem->state = TERMINAL_MODEM_STATE_COMMAND;
                        Modem_SendResponse(m, TERMINAL_HAYES_NO_CARRIER);
                    }

                    TALEA_LOG_TRACE("Serial: Remote connection closed.\n");
                }
            }
        }
    }

    Modem_Update(m);
}

// DEVICE UPDATE (on cpu tick)

// DEVICE PORT IO HANDLERS

// Initializes the socket API in windows. Returns true on success
