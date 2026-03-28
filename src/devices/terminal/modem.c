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

#include <ctype.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/
/* STATIC GLOBALS                                                             */
/*----------------------------------------------------------------------------*/

// Put statics here

/*----------------------------------------------------------------------------*/
/* INTERNAL DEVICE FUNCIONS: declare as static inline                         */
/*----------------------------------------------------------------------------*/
static inline void sendStr(TaleaMachine *m, const char *s);

static enum TerminalHayesResponse answerCall(TaleaMachine *m)
{
    TerminalHayesModem *modem   = &m->terminal.serial.modem;
    talea_net_t        *sock    = &m->terminal.serial.hostSocket;
    talea_net_t        *pending = &m->terminal.serial.pendingSocket;

    if (*pending == T_INVALID_SOCKET) {
        return TERMINAL_HAYES_ERROR;
    }

    *sock    = *pending;
    *pending = T_INVALID_SOCKET;

    net_set_nonblocking(*sock);
    m->terminal.serial.status |= TERMINAL_SER_STATUS_CARRIER_DETECT;

    modem->isRinging                  = false;
    modem->ringCount                  = 0;
    modem->sRegs[1]                   = 0;
    modem->state                      = TERMINAL_MODEM_STATE_DATA;
    m->terminal.serial.lastUpdateTime = GetTime();

    modem->cmdPos   = 0;
    modem->lastWasA = false;
    memset(modem->cmdBuffer, 0, sizeof(modem->cmdBuffer));

    TALEA_LOG_TRACE("Modem call answered, line active\n");
    return TERMINAL_HAYES_CONNECT;
}

static inline void sendStr(TaleaMachine *m, const char *s)
{
    TerminalHayesModem *modem = &m->terminal.serial.modem;

    u8 s3 = modem->sRegs[3];
    u8 s4 = modem->sRegs[4];

    if (modem->verboseMode) {
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

static inline int executeATCommand(TaleaMachine *m, char cmd, u8 value)
{
    TerminalHayesModem *modem    = &m->terminal.serial.modem;
    int  response = TERMINAL_HAYES_OK;

    // TODO: implement some cool easter eggs on ATI

    switch (cmd) {
    case 'A':
        /* Answer */
        if (modem->isRinging) {
            response = answerCall(m); // THROWS RESPONSE
        } else {
            response = TERMINAL_HAYES_ERROR;
        }
        break;
    case 'E':
        /* Echo enable */
        modem->echoEnabled = (value != 0);
        break;
    case 'H':
        /* Hang up*/
        Serial_CloseSockets(m);
        modem->state = TERMINAL_MODEM_STATE_COMMAND;
        break;
    case 'I':
        if (value == 0)
            sendStr(m, "103");
        else if (value == 1)
            break;
        else if (value == 2)
            break;
        else if (value == 3)
            sendStr(m, "The House of Talea [#]1");
        else if (value == 4) {
            char str[16] = { 0 };

            for (u8 i = 0; i <= 12; i++) {
                snprintf(str, sizeof(str), "S%02d=%03u", i, modem->sRegs[i]);
                sendStr(m, str);
                memset(str, 0, sizeof(str));
            }

            snprintf(str, sizeof(str), "S%02d=%03u", 37, modem->sRegs[37]);
            sendStr(m, str);
            memset(str, 0, sizeof(str));

        } else
            response = TERMINAL_HAYES_ERROR;
        break;
    case 'O':
        /* Go back online (to data mode) */
        if (m->terminal.serial.hostSocket != T_INVALID_SOCKET) {
            modem->state = TERMINAL_MODEM_STATE_DATA;
        } else {
            response = TERMINAL_HAYES_ERROR;
        }
        break;
    case 'Q': modem->quietMode = (value != 0); break;
    case 'S':
        /* Select S register*/
        if (value >= 50) {
            TALEA_LOG_TRACE("Illegal reg: %d\n", value);
            response = TERMINAL_HAYES_ERROR;
            break;
        }
        modem->currentSReg = value;
        break;
    case 'X': {
        if (value > 2) {
            response = TERMINAL_HAYES_ERROR;
            break;
        }
        modem->xLevel = value;
        break;
    }
    case '=':
        /* Write to selected register */
        modem->sRegs[modem->currentSReg] = value;
        break;
    case '?': {
        /* Return the selected register value */
        char n[4] = { 0 }; // We need three digits and a null
        snprintf(n, sizeof(n), "%03u", modem->sRegs[modem->currentSReg]);
        sendStr(m, n);
        break;
    }
    case 'V':
        /* Verbose mode enable */
        modem->verboseMode = (value != 0);
        break;
    case 'Z':
        /* Reset */
        Modem_ResetSregs(modem);
        break;

    default: response = TERMINAL_HAYES_ERROR; break;
    }

    return response;
}

static bool initiateConnection(TaleaMachine *m, const char *address, bool need_tls)
{
    TerminalHayesModem *modem = &m->terminal.serial.modem;

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
    if (m->terminal.serial.hostSocket != T_INVALID_SOCKET) {
        net_close(m->terminal.serial.hostSocket);
        m->terminal.serial.hostSocket = T_INVALID_SOCKET;
    }

    // Resolve address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(addr_copy, port, &hints, &res) != 0) {
        Modem_SendResponse(m, TERMINAL_HAYES_NO_DIALTONE); // TODO: handle more errors
        return false;
    }

    // new socket
    m->terminal.serial.hostSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (m->terminal.serial.hostSocket == T_INVALID_SOCKET) {
        freeaddrinfo(res);
        return false;
    }

    net_set_nonblocking(m->terminal.serial.hostSocket);

    int nodelay = 1;
    setsockopt(m->terminal.serial.hostSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay,
               sizeof(nodelay));

    result = connect(m->terminal.serial.hostSocket, res->ai_addr, (int)res->ai_addrlen);
    freeaddrinfo(res);

    m->terminal.serial.lastUpdateTime = GetTime();

    if (result == T_NET_ERROR) {
        if (net_would_block()) {
	  TALEA_LOG_TRACE("DIALING!\n");
            modem->state     = TERMINAL_MODEM_STATE_DIALING;
            modem->dialStart = GetTime();
            return true;
        } else {
            // Network unreachable
	  TALEA_LOG_TRACE("Network unreacheable!\n");
            net_close(m->terminal.serial.hostSocket);
            m->terminal.serial.hostSocket = T_INVALID_SOCKET;
            Modem_SendResponse(m, TERMINAL_HAYES_NO_CARRIER);
            return false;
        }
    }

    modem->state = TERMINAL_MODEM_STATE_DATA;
    Modem_SendResponse(m, TERMINAL_HAYES_CONNECT);
    m->terminal.serial.status |= TERMINAL_SER_STATUS_CARRIER_DETECT;
    return true;
}

static bool dial(TaleaMachine *m, char *address)
{
    TerminalHayesModem *modem = &m->terminal.serial.modem;

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
    char san[TERMINAL_MODEM_CMD_BUFFER_SIZE] = { 0 };

    for (size_t i = 0, j = 0; i < TERMINAL_MODEM_CMD_BUFFER_SIZE - 1; i++) {
        if (p[i] == modem->sRegs[3]) {
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

    return initiateConnection(m, san, need_tls);
}

static inline void parseATCommand(TaleaMachine *m, char *cmd_buff)
{
    TerminalHayesModem *modem    = &m->terminal.serial.modem;
    int  response = TERMINAL_HAYES_DEFER;

    if (!(cmd_buff[0] == 'A' && cmd_buff[1] == 'T')) {
        response = TERMINAL_HAYES_ERROR;
        goto end;
    } else if (cmd_buff[0] == 'A' && cmd_buff[1] == 'T' && cmd_buff[2] == modem->sRegs[3]) {
        // Just AT. custom is to say OK
        response = TERMINAL_HAYES_OK;
        goto end;
    }

    char *p = cmd_buff + 2;

    while (*p && *p != modem->sRegs[3]) {
        char  cmd = *p++;
        char *endp;
        u32   val = 0;

        if (cmd == 'D') {
            dial(m, p);
            response = TERMINAL_HAYES_DEFER;
            goto end;
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
            response = TERMINAL_HAYES_ERROR;
            goto end;
        }

        response = executeATCommand(m, cmd, (u8)val);
        if (response == TERMINAL_HAYES_ERROR) goto end;
    }

    memcpy(modem->lastValidCommand, cmd_buff, sizeof(modem->lastValidCommand));
end:
    if (response != TERMINAL_HAYES_DEFER) Modem_SendResponse(m, response);
}

/*----------------------------------------------------------------------------*/
/* INTERFACE IMPLEMENTATION                                                   */
/*----------------------------------------------------------------------------*/

void Modem_ResetSregs(TerminalHayesModem *modem)
{
    memset(modem->sRegs, 0, sizeof(modem->sRegs));
    modem->sRegs[0]  = 0;    // AUTO ANSWER
    modem->sRegs[1]  = 0;    // RING count
    modem->sRegs[2]  = '+';  // Escape char
    modem->sRegs[3]  = '\r'; // CR (cmd terminator)
    modem->sRegs[4]  = '\n'; // LF
    modem->sRegs[5]  = '\b'; // BACSKPACE
    modem->sRegs[7]  = 30;   // WAIT FOR CARRIER in seconds
    modem->sRegs[12] = 50;   // GUARD TIME in 1/50th of a second

    // Non standard S regs
    modem->sRegs[37] = TERMINAL_HAYES_BAUD_9600; // Desired baud rate

    modem->echoEnabled = true;
    modem->verboseMode = true;
    modem->plusCount   = 0;
}

void Modem_ProcessCommand(TaleaMachine *m, u8 byte)
{
    // Called on byte receipt in COMMAND state
    TerminalHayesModem *modem = &m->terminal.serial.modem;

    /* Echo character if ATE1 is set */
    if (modem->echoEnabled) {
        Serial_PushByte(m, byte);
    }

    if (modem->lastWasA && byte == '/') {
        modem->lastWasA = false;
        parseATCommand(m, modem->lastValidCommand);
        return;
    }

    // Cannot use a switch 'cause sRegs are not constant

    // Terminate the command and execute it
    /* S3 holds the CR character, used as command terminator */
    if (byte == modem->sRegs[3]) {
        modem->cmdBuffer[modem->cmdPos++] = modem->sRegs[3];
        modem->cmdBuffer[modem->cmdPos]   = '\0';
        parseATCommand(m, modem->cmdBuffer);
        modem->cmdPos = 0;
    }

    // Command correction
    /* S5 holds the backspace character */
    else if (byte == modem->sRegs[5] && modem->cmdPos > 0) {
        modem->lastWasA = false;
        modem->cmdPos--;
        if (modem->echoEnabled) {
            Serial_PushByte(m, ' ');
            Serial_PushByte(m, modem->sRegs[5]);
            // Serial_PushByte(m, modem->sRegs[5]);
        }
    }

    // Append byte to command buffer
    // -2 because we allways want to be able to append CR and null
    else if (modem->cmdPos < TERMINAL_MODEM_CMD_BUFFER_SIZE - 2) {
        modem->lastWasA                   = (toupper(byte) == 'A');
        modem->cmdBuffer[modem->cmdPos++] = toupper(byte);
    }

    // If here, buffer overrun has happened
    else {
        // TODO: signal buffer overrun? with a bell?
    }
}

void Modem_ProcessData(TaleaMachine *m, u8 byte)
{
    if (m->terminal.serial.hostSocket != T_INVALID_SOCKET) {
        send(m->terminal.serial.hostSocket, &byte, 1, 0);
    }
}

void Modem_SendResponse(TaleaMachine *m, enum TerminalHayesResponse code)
{
    TerminalHayesModem *modem = &m->terminal.serial.modem;

    const char *code_str[] = {
        [TERMINAL_HAYES_OK] = "OK",       [TERMINAL_HAYES_CONNECT] = "CONNECT",
        [TERMINAL_HAYES_RING] = "RING",   [TERMINAL_HAYES_NO_CARRIER] = "NO CARRIER",
        [TERMINAL_HAYES_ERROR] = "ERROR", [TERMINAL_HAYES_NO_DIALTONE] = "NO DIALTONE",
        [TERMINAL_HAYES_BUSY] = "BUSY",   [TERMINAL_HAYES_NO_ANSWER] = "NO ANSWER",
    };

    if (modem->quietMode) return;

    u8 s3 = modem->sRegs[3];
    u8 s4 = modem->sRegs[4];

    // maybe use snprintf

    if (modem->verboseMode) {
        /* Formats as: <CR><LF>OK<CR><LF> */
        Serial_PushByte(m, s3);
        Serial_PushByte(m, s4);

        if (code == TERMINAL_HAYES_CONNECT && modem->xLevel >= 1) {
            char str[16];
            modem->sRegs[37] = MIN(modem->sRegs[37], TERMINAL_HAYES_BAUD_TOTAL - 1);
            snprintf(str, sizeof(str), "CONNECT %d", TerminalHayesBaudRateLookup[modem->sRegs[37]]);
            Serial_PushString(m, str);
            memset(str, 0, sizeof(str));
        } else if (code == TERMINAL_HAYES_NO_DIALTONE && modem->xLevel < 2) {
            Serial_PushString(m, code_str[TERMINAL_HAYES_ERROR]);
        } else {
            Serial_PushString(m, code_str[code]);
        }
        Serial_PushByte(m, s3);
        Serial_PushByte(m, s4);
    } else {
        /* Formats as: 0<CR> */
        if (code == TERMINAL_HAYES_NO_DIALTONE && modem->xLevel < 2) {
            Serial_PushByte(m, TERMINAL_HAYES_ERROR + '0');
        } else {
            Serial_PushByte(m, code + '0');
        }
        // TODO: This only works now that codes are single number
        Serial_PushByte(m, s3);
    }
}

void Modem_CheckEscapeSequence(TaleaMachine *m, u8 byte)
{
    TerminalHayesModem *modem      = &m->terminal.serial.modem;
    double              now        = GetTime();
    double              elapsed    = now - modem->lastTxTime;
    double              guard_time = TERMINAL_GET_GUARD_TIME(modem);

    // TALEA_LOG_TRACE("Checking escape sequence (%c) [%d] %d\n", byte, modem->sRegs[2],
    //                 modem->sRegs[2] == '+');

    if (modem->waitingAfter) {
        /* Any character sent during this window invalidates the sequence */
        // TALEA_LOG_TRACE("Invalidadated sequence\n");
        modem->waitingAfter = false;
        modem->plusCount    = 0;
    }

    if (byte == modem->sRegs[2]) {
        // TALEA_LOG_TRACE("It is the character!\n");
        if (modem->plusCount == 0) {
            /* First '+': Must follow a period of silence */
            if (elapsed >= guard_time) {
                // TALEA_LOG_TRACE("Got 1st +\n");
                modem->plusCount = 1;
            }
        } else if (modem->plusCount < 3) {
            if (elapsed < guard_time) {
                // TALEA_LOG_TRACE("Got another +\n");
                modem->plusCount++;
            } else {
                // TALEA_LOG_TRACE("too much time passed\n");
                modem->plusCount = (elapsed >= guard_time) ? 1 : 0;
            }
        }
    } else {
        modem->plusCount = 0;
    }

    modem->lastTxTime = now;

    if (modem->plusCount == 3) {
        // TALEA_LOG_TRACE("Got all 3! waiting\n");
        modem->waitingAfter = true;
    }
}

// DEVICE INITIALIZATION

// DEVICE UPDATE (on vblank)

void Modem_Update(TaleaMachine *m)
{
    TerminalHayesModem *modem   = &m->terminal.serial.modem;
    talea_net_t         sock    = m->terminal.serial.hostSocket;
    talea_net_t        *pending = &m->terminal.serial.pendingSocket;

    if (modem->isRinging) {
        char buf[1];
        int  connected = recv(*pending, buf, 1, MSG_PEEK);

        if (!connected) {
            net_close(*pending);
            *pending         = T_INVALID_SOCKET;
            modem->isRinging = false;
            modem->ringCount = 0;
            modem->sRegs[1]  = 0;
            goto skip_ring;
        }

        double now = GetTime();

        if (now - modem->lastRingTime > 2.0) {
            sendStr(m, "\r\nRING\r\n");
            modem->ringCount++;
            modem->sRegs[1]     = modem->ringCount;
            modem->lastRingTime = now;

            if (modem->sRegs[0] > 0 && modem->ringCount >= modem->sRegs[0]) {
                answerCall(m);
            }
        }
    }
skip_ring:

    switch (modem->state) {
    case TERMINAL_MODEM_STATE_COMMAND:
        /* Nothing here */
        break;
    case TERMINAL_MODEM_STATE_DIALING: {
        if (sock == T_INVALID_SOCKET) {
	  TALEA_LOG_TRACE("CANCELLED DIALING!\n");
            modem->state = TERMINAL_MODEM_STATE_COMMAND;
            Modem_SendResponse(m, TERMINAL_HAYES_NO_CARRIER);
            return;
        }

        double now = GetTime();
        if (now - modem->dialStart > modem->sRegs[7]) {
            TALEA_LOG_TRACE("Modem: S7 Timeout\n");
            Serial_CloseSockets(m);
            modem->state = TERMINAL_MODEM_STATE_COMMAND;
            Modem_SendResponse(m, TERMINAL_HAYES_NO_CARRIER);
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
	      TALEA_LOG_TRACE("CONNECTION REFUSED (WINDOWS)\n");
                Serial_CloseSockets(m);
                modem->state = TERMINAL_MODEM_STATE_COMMAND;
                Modem_SendResponse(m, TERMINAL_HAYES_NO_CARRIER);
            } else if (FD_ISSET(sock, &wfds)) {
                // Potential Success
                int       err = 0;
                socklen_t len = sizeof(err);

                // Double check the error state (Required for POSIX)
                if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&err, &len) < 0 || err != 0) {
		  TALEA_LOG_TRACE("CONNECTION REFUSED (POSIX)\n");
                    Serial_CloseSockets(m);
                    modem->state = TERMINAL_MODEM_STATE_COMMAND;
                    Modem_SendResponse(m, TERMINAL_HAYES_NO_CARRIER);
                } else {
                    // SUCCESS!
		  TALEA_LOG_TRACE("CONNECTED!\n");
                    modem->state = TERMINAL_MODEM_STATE_DATA;
                    m->terminal.serial.status |= TERMINAL_SER_STATUS_CARRIER_DETECT;
                    Modem_SendResponse(m, TERMINAL_HAYES_CONNECT);
                }
            }
        }
        break;
    }

    case TERMINAL_MODEM_STATE_DATA: {
        // Escape sequence logic
        if (modem->waitingAfter) {
            double now = GetTime();
            if ((now - modem->lastTxTime) >= TERMINAL_GET_GUARD_TIME(modem)) {
                TALEA_LOG_TRACE("Escaped!\n");

                modem->state        = TERMINAL_MODEM_STATE_COMMAND;
                modem->waitingAfter = false;
                modem->plusCount    = 0;
                Modem_SendResponse(m, TERMINAL_HAYES_OK);
            }
        }
        break;
    }

    case TERMINAL_MODEM_STATE_DISCONNECTING: break;
    default:
        TALEA_LOG_WARNING("Reached unreachable code: modem state unknown. resetting\n");
        modem->state = TERMINAL_MODEM_STATE_COMMAND;
        Modem_ResetSregs(modem);
        break;
    }
}

// DEVICE UPDATE (on cpu tick)

// DEVICE PORT IO HANDLERS
