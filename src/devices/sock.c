#include <ctype.h>
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

extern double GetTime(void);

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
    dev->server_fd   = T_INVALID_SOCKET;
    dev->host_socket = T_INVALID_SOCKET;
    dev->modem.state = MODEM_STATE_COMMAND;

    Modem_ResetSregs(&dev->modem);
}

void Serial_CloseSockets(TaleaMachine *m)
{
    if (m->terminal.serial.host_socket != INVALID_SOCKET) {
        net_close(m->terminal.serial.host_socket);
        m->terminal.serial.host_socket = INVALID_SOCKET;
    }
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

    int opt = 1;
    if (setsockopt(dev->server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) ==
        T_NET_ERROR) {
        perror("Serial Host: setsockopt failed");
    }

    struct sockaddr_in server_addr = { 0 };
    server_addr.sin_family         = AF_INET;
    server_addr.sin_addr.s_addr    = INADDR_ANY; // Listen on localhost
    server_addr.sin_port           = htons(port);

    if (bind(dev->server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == T_NET_ERROR) {
        perror("Serial Host: Bind failed");
        net_close(dev->server_fd);
        dev->server_fd = T_INVALID_SOCKET;
        return;
    }

    if (listen(dev->server_fd, 1) == T_NET_ERROR) {
        perror("Serial Host: Listen failed");
        return;
    }

    net_set_nonblocking(dev->server_fd);

    printf("Serial Host: Virtual COM port listening on localhost:%d\n", port);
}

void Modem_Update(TaleaMachine *m)
{
    HayesModem *modem = &m->terminal.serial.modem;
    talea_net_t sock  = m->terminal.serial.host_socket;

    switch (modem->state) {
    case MODEM_STATE_COMMAND:
        /* Nothing here */
        break;
    case MODEM_STATE_DIALING: {
        if (sock == T_INVALID_SOCKET) {
            modem->state = MODEM_STATE_COMMAND;
            Modem_SendResponse(m, HAYES_NO_CARRIER);
            return;
        }

        double now = GetTime();
        if (now - modem->dial_start > modem->s_regs[7]) {
            TALEA_LOG_TRACE("Modem: S7 Timeout\n");
            Serial_CloseSockets(m);
            modem->state = MODEM_STATE_COMMAND;
            Modem_SendResponse(m, HAYES_NO_CARRIER);
            return;
        }

        fd_set         wfds, efds;
        struct timeval tv = { 0, 0 }; 

        FD_ZERO(&wfds);
        FD_ZERO(&efds);
        FD_SET(sock, &wfds);
        FD_SET(sock, &efds);

        int res = select((int)sock + 1, NULL, &wfds, &efds, &tv);

        if (res > 0) {
            if (FD_ISSET(sock, &efds)) {
                // Connection Refused (Windows)
                Serial_CloseSockets(m);
                modem->state = MODEM_STATE_COMMAND;
                Modem_SendResponse(m, HAYES_NO_CARRIER);
            } else if (FD_ISSET(sock, &wfds)) {
                // Potential Success
                int       err = 0;
                socklen_t len = sizeof(err);

                // Double check the error state (Required for POSIX)
                if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&err, &len) < 0 || err != 0) {
                    Serial_CloseSockets(m);
                    modem->state = MODEM_STATE_COMMAND;
                    Modem_SendResponse(m, HAYES_NO_CARRIER);
                } else {
                    // SUCCESS!
                    modem->state = MODEM_STATE_DATA;
                    Modem_SendResponse(m, HAYES_CONNECT);
                }
            }
        }
        break;
    }

    case MODEM_STATE_DATA: {
        // Escape sequence logic
        if (modem->waiting_after) {
            double now = GetTime();
            if ((now - modem->last_tx_time) >= GET_GUARD_TIME(modem)) {
                TALEA_LOG_TRACE("Escaped!\n");

                modem->state         = MODEM_STATE_COMMAND;
                modem->waiting_after = false;
                modem->plus_count    = 0;
                Modem_SendResponse(m, HAYES_OK);
            }
        }
        break;
    }

    case MODEM_STATE_DISCONNECTING: break;
    default:
        TALEA_LOG_WARNING("Reached unreachable code: modem state unknown. resetting\n");
        modem->state = MODEM_STATE_COMMAND;
        Modem_ResetSregs(modem);
        break;
    }
}

static inline int Serial_GetFifoUsed(TerminalSerial *dev)
{
    if (dev->head >= dev->tail) {
        return dev->head - dev->tail;
    } else {
        return (SERIAL_FIFO_SIZE - dev->tail) + dev->head;
    }
}

static inline int Serial_GetFifoSpace(TerminalSerial *dev)
{
    return (SERIAL_FIFO_SIZE - 1) - Serial_GetFifoUsed(dev);
}

void Serial_Update(TaleaMachine *m)
{
    TerminalSerial *dev   = &m->terminal.serial;
    HayesModem     *modem = &dev->modem;

    // Accept new connections
    if (modem->state == MODEM_STATE_COMMAND && dev->host_socket == T_INVALID_SOCKET) {
        talea_net_t new_socket = accept(dev->server_fd, NULL, NULL);

        if (new_socket != T_INVALID_SOCKET) {
            dev->host_socket = new_socket;
            dev->status |= SER_STATUS_CARRIER_DETECT;
            printf("Serial Host: Terminal connected!\n");
            net_set_nonblocking(dev->host_socket);
        } else if (!net_would_block()) {
            TALEA_LOG_ERROR("ERROR connecting to host socket\n");
        }
    }

    // Read incoming data from terminal or network
    if (dev->host_socket != T_INVALID_SOCKET && modem->state != MODEM_STATE_DIALING) {
        double now            = GetTime();
        double delta_time     = now - dev->last_update_time;
        dev->last_update_time = now;

        // Clamp to baud rate / 10 bits per byte
        dev->byte_credit += (SERIAL_BAUD_RATE / 10.0) * delta_time;

        if (dev->byte_credit >= 1.0) {
            int fifo_space  = Serial_GetFifoSpace(dev);
            int max_to_read = (int)dev->byte_credit;
            if (max_to_read > fifo_space) max_to_read = fifo_space;
            if (max_to_read > 128) max_to_read = 128;

            if (max_to_read > 0) {
                u8  buffer[128];
                int n = recv(dev->host_socket, (char *)buffer, max_to_read, 0);
                if (n > 0) {
                    for (int i = 0; i < n; i++) {
                        u8 next = (dev->head + 1) % SERIAL_FIFO_SIZE;
                        if (next != dev->tail) {
                            dev->rx_fifo[dev->head] = buffer[i];
                            dev->head               = next;
                        } else {
                            dev->status |= SER_STATUS_BUFFER_OVERRUN;
                            break;
                        }
                    }
                    dev->byte_credit -= n;

                    // Fire the Priority 4 Interrupt
                    dev->status |= SER_STATUS_DATA_AVAILABLE;
                    if (dev->control & SER_CONTROL_INT_EN)
                        Machine_RaiseInterrupt(m, INT_TTY_TRANSMIT, PRIORITY_SERIAL_INTERRUPT);

                } else if (n == 0 || (n < 0 && !net_would_block())) {
                    // Disconnect

                    net_close(dev->host_socket);
                    dev->host_socket = T_INVALID_SOCKET;
                    dev->status &= ~SER_STATUS_CARRIER_DETECT;

                    if (modem->state == MODEM_STATE_DATA) {
                        // The BBS hung up
                        modem->state = MODEM_STATE_COMMAND;
                        Modem_SendResponse(m, HAYES_NO_CARRIER);
                    }

                    TALEA_LOG_TRACE("Serial: Remote connection closed.\n");
                }
            }
        }
    }

    Modem_Update(m);
}

void Modem_SendResponse(TaleaMachine *m, enum HayesResponse code)
{
    HayesModem *modem = &m->terminal.serial.modem;

    const char *code_str[] = {
        [HAYES_OK] = "OK",       [HAYES_CONNECT] = "CONNECT",
        [HAYES_RING] = "RING",   [HAYES_NO_CARRIER] = "NO CARRIER",
        [HAYES_ERROR] = "ERROR",
    };

    u8 s3 = modem->s_regs[3];
    u8 s4 = modem->s_regs[4];

    // maybe use snprintf

    if (modem->verbose_mode) {
        /* Formats as: <CR><LF>OK<CR><LF> */
        Serial_PushByte(m, s3);
        Serial_PushByte(m, s4);
        Serial_PushString(m, code_str[code]);
        Serial_PushByte(m, s3);
        Serial_PushByte(m, s4);
    } else {
        /* Formats as: 0<CR> */
        Serial_PushByte(m, code + '0'); // TODO: This only works now that codes are single
                                        // number
        Serial_PushByte(m, s3);
    }
}

static inline void Modem_SendStr(TaleaMachine *m, const char *s)
{
    HayesModem *modem = &m->terminal.serial.modem;

    u8 s3 = modem->s_regs[3];
    u8 s4 = modem->s_regs[4];

    if (modem->verbose_mode) {
        /* Formats as: <CR><LF>OK<CR><LF> */
        Serial_PushByte(m, s3);
        Serial_PushByte(m, s4);
        Serial_PushString(m, s);
        Serial_PushByte(m, s3);
        Serial_PushByte(m, s4);
    } else {
        /* Formats as: 0<CR> */
        Serial_PushString(m, s);
        Serial_PushByte(m, s3);
    }
}

void Modem_ResetSregs(HayesModem *modem)
{
    memset(modem->s_regs, 0, sizeof(modem->s_regs));
    modem->s_regs[0]  = 0; // AUTO ANSWER
    modem->s_regs[2]  = '+';
    modem->s_regs[3]  = '\r'; // CR (cmd terminator)
    modem->s_regs[4]  = '\n'; // LF
    modem->s_regs[5]  = '\b'; // backspace
    modem->s_regs[7]  = 30;   // WAIT FOR CARRIER
    modem->s_regs[12] = 50;   // default 1s guard time

    modem->echo_enabled = true;
    modem->verbose_mode = true;
    modem->plus_count   = 0;
}

static inline bool Modem_ExecuteATCommand(TaleaMachine *m, char cmd, u8 value)
{
    HayesModem *modem   = &m->terminal.serial.modem;
    bool        success = true;

    // TODO: implement some cool easter eggs on ATI

    switch (cmd) {
    case 'E':
        /* Echo enable */
        modem->echo_enabled = (value != 0);
        break;
    case 'H':
        /* Hang up*/
        Serial_CloseSockets(m);
        modem->state = MODEM_STATE_COMMAND;
        Modem_SendResponse(m, HAYES_OK);
        break;
    case 'O':
        /* Go back online (to data mode) */
        if (m->terminal.serial.host_socket != T_INVALID_SOCKET) {
            modem->state = MODEM_STATE_DATA;
        } else {
            Modem_SendResponse(m, HAYES_ERROR);
        }
        break;
    case 'S':
        /* Select S register*/
        modem->current_s_reg = value & 0xF;
        break;
    case '=':
        /* Write to selected register */
        modem->s_regs[modem->current_s_reg] = value;
        break;
    case '?': {
        /* Return the selected register value */
        char n[4] = { 0 }; // We need three digits and a null
        snprintf(n, sizeof(n), "%03u", modem->s_regs[modem->current_s_reg]);
        Modem_SendStr(m, n);
        break;
    }
    case 'V':
        /* Verbose mode enable */
        modem->verbose_mode = (value != 0);
        break;
    case 'Z':
        /* Reset */
        Modem_ResetSregs(modem);
        break;

    default: success = false; break;
    }
    return success;
}

static int Modem_InitiateConnection(TaleaMachine *m, const char *address, bool need_tls)
{
    HayesModem *modem = &m->terminal.serial.modem;

    struct addrinfo hints, *res;
    int             result;

    char  addr_copy[64];
    char *port = "23";
    char *colon;

    strncpy(addr_copy, address, sizeof(addr_copy) - 1);
    addr_copy[sizeof(addr_copy) - 1] = '\0';

    colon = strchr(addr_copy, ':');
    if (colon) {
        *colon = '\0';      
        port   = colon + 1;
    }

    // Disconnect terminal if connected
    if (m->terminal.serial.host_socket != T_INVALID_SOCKET) {
        net_close(m->terminal.serial.host_socket);
        m->terminal.serial.host_socket = T_INVALID_SOCKET;
    }

    // Resolve address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;     
    hints.ai_socktype = SOCK_STREAM; 

    if (getaddrinfo(addr_copy, port, &hints, &res) != 0) {
        // Modem_SendResponse(m, HAYES_NO_DIALTONE); // TODO: handle more errors
        return false;
    }

    // new socket
    m->terminal.serial.host_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (m->terminal.serial.host_socket == T_INVALID_SOCKET) {
        freeaddrinfo(res);
        return false;
    }

    net_set_nonblocking(m->terminal.serial.host_socket);

    int nodelay = 1;
    setsockopt(m->terminal.serial.host_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay,
               sizeof(nodelay));

    result = connect(m->terminal.serial.host_socket, res->ai_addr, (int)res->ai_addrlen);
    freeaddrinfo(res);

    if (result == T_NET_ERROR) {
        if (net_would_block()) {
            modem->state      = MODEM_STATE_DIALING;
            modem->dial_start = GetTime();
            return true;
        } else {
            // Network unreachable
            net_close(m->terminal.serial.host_socket);
            m->terminal.serial.host_socket = T_INVALID_SOCKET;
            Modem_SendResponse(m, HAYES_NO_CARRIER);
            return true;
        }
    }
    
    modem->state = MODEM_STATE_DATA;
    Modem_SendResponse(m, HAYES_CONNECT);
    return true;
}

static bool Modem_Dial(TaleaMachine *m, char *address)
{
    HayesModem *modem = &m->terminal.serial.modem;

    char *p        = address;
    bool  need_tls = false; // TODO: implement secure connections
    // TODO: maybe and only maaaybe implement UDP

    if (*p == 'D' || *p == 'T') p++; // We allow ATDT or ATDD for retro look, but it does nothin'
    if (*p == 'S') {
        need_tls = true;
        p++;
    }

    // Sanitize address: discard ' ', '(' and ')' (specifically space, and not ANY
    // whitespace) allow A-Z 0-9 . - : as they appear in domains and Ip characters. allow @
    // and & for some other purposes like address book, etc.
    char san[MODEM_CMD_BUFFER_SIZE] = { 0 };

    for (size_t i = 0, j = 0; i < MODEM_CMD_BUFFER_SIZE - 1; i++) {
        if (p[i] == modem->s_regs[3]) {
            san[j] = '\0';
            break;
        }

        if (isalnum(p[i]) || p[i] == '.' || p[i] == '-' || p[i] == ':' || p[i] == '@' ||
            p[i] == '&') {
            san[j] = p[i];
            j++;
        } else if (p[i] == ' ' || p[i] == '(' || p[i] == ')') {
            continue;
        } else {
            return false;
        }
    }

    return Modem_InitiateConnection(m, san, need_tls);
}

static inline Modem_ParseATCommand(TaleaMachine *m, char *cmd_buff)
{
    HayesModem *modem = &m->terminal.serial.modem;

    if (!(cmd_buff[0] == 'A' && cmd_buff[1] == 'T')) {
        Modem_SendResponse(m, HAYES_ERROR);
        return;
    }

    char *p = cmd_buff + 2;

    while (*p != modem->s_regs[3]) {
        char  cmd = *p++;
        char *endp;
        u32   val = 0;

        if (cmd == 'D') { // could write this as if(cmd == 'D' && Modem_Dial(m, p))...
            if (!Modem_Dial(m, p)) {
                Modem_SendResponse(m, HAYES_ERROR);
            }
            return;
        }

        while (*p == ' ') {
            p++;
        }

        if (isdigit(*p)) {
            val = strtoul(p, &endp, 10);
            val = (p == endp) ? 0 : val;
            p   = endp;
        }

        if (val > 255) {
            TALEA_LOG_TRACE("V: %d\n", val);
            Modem_SendResponse(m, HAYES_ERROR);
            return;
        }

        if (!Modem_ExecuteATCommand(m, cmd, (u8)val)) {
            Modem_SendResponse(m, HAYES_ERROR);
            return;
        }
    }

    memcpy(modem->last_valid_command, cmd_buff, sizeof(modem->last_valid_command));
    if (!modem->defer_response) Modem_SendResponse(m, HAYES_OK);
}

static void Modem_ProcessCommand(TaleaMachine *m, u8 byte)
{
    // Called on byte receipt in COMMAND state
    HayesModem *modem = &m->terminal.serial.modem;

    /* Echo character if ATE1 is set */
    if (modem->echo_enabled) {
        Serial_PushByte(m, byte);
    }

    if (modem->last_was_A && byte == '/') {
        modem->last_was_A = false;
        Modem_ParseATCommand(m, modem->last_valid_command);
        return;
    }

    // Cannot use a switch 'cause s_regs are not constant

    // Terminate the command and execute it
    /* S3 holds the CR character, used as command terminator */
    if (byte == modem->s_regs[3]) {
        modem->cmd_buffer[modem->cmd_pos++] = modem->s_regs[3];
        modem->cmd_buffer[modem->cmd_pos]   = '\0';
        Modem_ParseATCommand(m, modem->cmd_buffer);
        modem->cmd_pos = 0;
    }

    // Command correction
    /* S5 holds the backspace character */
    else if (byte == modem->s_regs[5] && modem->cmd_pos > 0) {
        modem->last_was_A = false;
        modem->cmd_pos--;
        if (modem->echo_enabled) {
            Serial_PushByte(m, modem->s_regs[5]);
            Serial_PushByte(m, ' ');
            Serial_PushByte(m, modem->s_regs[5]);
        }
    }

    // Append byte to command buffer
    // -2 because we allways want to be able to append CR and null
    else if (modem->cmd_pos < MODEM_CMD_BUFFER_SIZE - 2) {
        modem->last_was_A                   = (toupper(byte) == 'A');
        modem->cmd_buffer[modem->cmd_pos++] = toupper(byte);
    }

    // If here, buffer overrun has happened
    else {
        // TODO: signal buffer overrun? with a bell?
    }
}

static void Modem_ProcessData(TaleaMachine *m, u8 byte)
{
    if (m->terminal.serial.host_socket != INVALID_SOCKET) {
        send(m->terminal.serial.host_socket, &byte, 1, 0);
    }
}

void Serial_SendByte(TaleaMachine *m, u8 byte)
{
    HayesModem *modem = &m->terminal.serial.modem;

    TALEA_LOG_TRACE("Sending byte %02x\n", byte);

    switch (modem->state) {
    case MODEM_STATE_COMMAND: Modem_ProcessCommand(m, byte); break;
    case MODEM_STATE_DIALING: {
        // Abort connection
        Serial_CloseSockets(m);
        Modem_SendResponse(m, HAYES_ERROR);
        break;
    }
    case MODEM_STATE_DATA: /*TODO*/ Modem_ProcessData(m, byte); break;
    case MODEM_STATE_DISCONNECTING: /*TODO*/ break;
    }

    Modem_CheckEscapeSequence(m, byte);
}