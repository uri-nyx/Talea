#ifndef TERMINAL_H
#define TERMINAL_H

#include "frontend/config.h"
#include "logging.h"
#include "machine_description.h"
#include "types.h"

#ifndef DO_NOT_INCLUDE_RAYLIB
// Due to confilcts with the WinAPI
#include "raylib.h"
#else
// definitions we need from raylib
#include "need_from_raylib.h"
#endif

struct TaleaMachine;

/*----------------------------------------------------------------------------*/
/* DEVICE MAGIC ID                                                            */
/*----------------------------------------------------------------------------*/

#define DEVICE_TERMINAL_MAGIC (1413829197) // 'TERM'

/*----------------------------------------------------------------------------*/
/* DEVICE PORTS                                                               */
/*----------------------------------------------------------------------------*/

#define P_TERMINAL_SERIAL_DATA     0x0
#define P_TERMINAL_SERIAL_STATUS   0x1
#define P_TERMINAL_SERIAL_CTRL     0x2
#define P_TERMINAL_SERIAL_RXCOUNT  0x3
#define P_TERMINAL_TIMER_TIMEOUT   0x6
#define P_TERMINAL_TIMER_INTERVAL  0x8
#define P_TERMINAL_TIMER_PRESCALER 0xa
#define P_TERMINAL_TIMER_CSR       0xb
#define P_TERMINAL_KBD_CSR         0xc
#define P_TERMINAL_KBD_CHAR        0xd
#define P_TERMINAL_KBD_CODE        0xe

/*----------------------------------------------------------------------------*/
/* DEVICE CONSTANTS                                                           */
/* ---------------------------------------------------------------------------*/

#ifdef DEBUG_LOG_SOCKET
// FIXME: LOG
#define LOG_SOCKET_ERROR(error_text) fprintf(stderr, "SOCKET: %s [%d]\n", error_text, tcs_error);
#else
#define LOG_SOCKET_ERROR(error_text) (void)(error_text)
#endif

#define TERMINAL_CONTROL    0x01
#define TERMINAL_SHIFT      0x02
#define TERMINAL_LALT       0x04
#define TERMINAL_RGUI       0x08
#define TERMINAL_CAPSLOCK   0x10
#define TERMINAL_SCROLLLOCK 0x20
#define TERMINAL_RALT       0x40

#define TERMINAL_KB_QUEUE_LEN 32

enum TerminalKeyboardCSR {
    TERMINAL_KB_IE_DOWN = 0x01,   // Fire interrupt when a key is pressed.
    TERMINAL_KB_IE_UP   = 0x02,   // Fire interrupt when a key is released.
    TERMINAL_KB_IE_CHAR = 0x04,   // Filter: Only fire if the CHARACTER register is
                                  //    non-zero (ASCII).
    TERMINAL_KB_GLOBAL_EN = 0x80, // Master switch for Keyboard Interrupts.
    TERMINAL_KB_GET_CSR   = 0x10, // If set, when read, CSR register returns the status of
                                  // the Interrupt enable flags and this flag is cleared
                                  // //TODO: Document this
};

enum TerminalTimerCSR {
    TERMINAL_TIM_TIMEOUT_EN  = 0x01, // start/stop timeout
    TERMINAL_TIM_INTERVAL_EN = 0x02, // start/stop interval
    TERMINAL_TIM_GLOBAL_EN   = 0x80, // global enable
};

enum TerminalSerialStatus {
    TERMINAL_SER_STATUS_DATA_AVAILABLE = 0x1, // RX FIFO not empty
    TERMINAL_SER_STATUS_CARRIER_DETECT = 0x2, // Socket is connected
    TERMINAL_SER_STATUS_BUFFER_OVERRUN = 0x4,
};

enum TerminalSerialControl {
    TERMINAL_SER_CONTROL_INT_EN       = 0x1,  // Enable interrupt upon receiving data
    TERMINAL_SER_CONTROL_MASTER_RESET = 0x80, // Master reset (clear all buffers)
};

enum TerminalHayesBaudRate {
    TERMINAL_HAYES_BAUD_300,
    TERMINAL_HAYES_BAUD_1200,
    TERMINAL_HAYES_BAUD_2400,
    TERMINAL_HAYES_BAUD_9600,
    TERMINAL_HAYES_BAUD_19200,
    TERMINAL_HAYES_BAUD_38400,
    TERMINAL_HAYES_BAUD_57600,
    TERMINAL_HAYES_BAUD_TOTAL,
};

enum TerminalHayesResponse {
    TERMINAL_HAYES_DEFER = -1,
    TERMINAL_HAYES_OK,
    TERMINAL_HAYES_CONNECT,
    TERMINAL_HAYES_RING,
    TERMINAL_HAYES_NO_CARRIER,
    TERMINAL_HAYES_ERROR,
    TERMINAL_HAYES_NO_DIALTONE,
    TERMINAL_HAYES_BUSY,
    TERMINAL_HAYES_NO_ANSWER,
};

enum TerminalModemState {
    TERMINAL_MODEM_STATE_COMMAND,      /* Accepting AT commands */
    TERMINAL_MODEM_STATE_DIALING,      /* TCP connect() in progress (Non-blocking) */
    TERMINAL_MODEM_STATE_DATA,         /* Transparent pass-through mode */
    TERMINAL_MODEM_STATE_DISCONNECTING /* Closing socket/Cleaning up */
};

#define TERMINAL_MODEM_CMD_BUFFER_SIZE 65
#define TERMINAL_GET_GUARD_TIME(m)     ((double)((m)->sRegs[12]) / 50.0)

// clang-format off
static const int TerminalHayesBaudRateLookup[TERMINAL_HAYES_BAUD_TOTAL] = 
    { 300,  1200,  2400, 9600, 19200, 38400, 57600 };
// clang-format on

#define TERMINAL_DEFAULT_BAUD_RATE 9600
#define TERMINAL_SERIAL_FIFO_SIZE  256

/*----------------------------------------------------------------------------*/
/* DEVICE STRUCTURES                                                          */
/*--------------------------------------------------------------------------- */

typedef struct {
    u16  scancode;
    u8   character;
    u8   modifiers;
    bool isDown;
} TerminalKbdEvent;

typedef struct {
    TerminalKbdEvent queue[TERMINAL_KB_QUEUE_LEN];
    int              head, tail;

    u8  csr;
    u16 modifiers;
} TerminalKeyboard;

typedef struct {
    u8  csr;
    u8  prescaler;
    u16 timeoutCounter;
    u16 intervalCounter;

    u64 cycles;
    u16 intervalReload;

    // Internal cycle accumulators
    u32 accumT;
    u32 accumI;
} TerminalTimer;

typedef struct {
    // --- Internal State Machine ---
    int state;

    char   cmdBuffer[TERMINAL_MODEM_CMD_BUFFER_SIZE];
    char   lastValidCommand[TERMINAL_MODEM_CMD_BUFFER_SIZE];
    size_t cmdPos;

    // Configuration
    bool echoEnabled;
    bool deferResponse;
    bool lastWasA;
    bool verboseMode;
    bool quietMode;

    u8 xLevel;

    u8 sRegs[50];
    u8 currentSReg;

    double lastTxTime;
    int    plusCount;
    bool   waitingAfter;

    // Networking
    double dialStart;
    double lastRingTime;
    bool   isRinging;
    u8     ringCount;
} TerminalHayesModem;

typedef struct {
    // --- Hardware Registers ---
    u8 rxFifo[TERMINAL_SERIAL_FIFO_SIZE];
    u8 head;    // Write pointer (for host)
    u8 tail;    // Read pointer (for guest CPU)
    u8 status;  // Status bits (Data Avail, Carrier)
    u8 control; // Interrupt enable flags

    TerminalHayesModem modem;

    // --- Throttling logic ---
    double lastUpdateTime;
    double byteCredit;

    // --- Host/Networking Layer ---
    talea_net_t hostSocket;    // The "Active Line"
    talea_net_t pendingSocket; // an incoming call
    talea_net_t serverFd;      // The "Wall Jack" (Always listening for incoming)
    bool        isListening;   // State for AT_LISTEN
} TerminalSerial;

/*----------------------------------------------------------------------------*/
/* DEVICE State                                                           */
/*----------------------------------------------------------------------------*/

typedef struct DeviceTerminal {
    TerminalKeyboard kb;
    TerminalTimer    timer;
    TerminalSerial   serial;
} DeviceTerminal;

/*----------------------------------------------------------------------------*/
/* DEVICE INTERFACE                                                           */
/*----------------------------------------------------------------------------*/

// Callbacks for Raylib's AudioStream
// Here put the callbacks you want raylib to execute on the AudioStream thread

// DEVICE INITIALIZATION
void Terminal_Reset(struct TaleaMachine *m, TaleaConfig *conf, bool isRestart);

// DEVICE DEINITIALIZATION
void Devname_Deinit(struct TaleaMachine *m);

// DEVICE UPDATE (on vblank)
void Modem_Update(struct TaleaMachine *m);

// where do I put this?
void Modem_CheckEscapeSequence(struct TaleaMachine *m, u8 b);
void Modem_ResetSregs(TerminalHayesModem *modem);
void Modem_SendResponse(struct TaleaMachine *m, enum TerminalHayesResponse code);
void Serial_PushByte(struct TaleaMachine *m, u8 b);
u8   Serial_PopByte(TerminalSerial *s);
void Serial_PushString(struct TaleaMachine *m, const u8 *str);
void Keyboard_ProcessKeypress(struct TaleaMachine *m, bool isDown, int key, u8 chr, u16 mod);
void Keyboard_PushEvent(TerminalKeyboard *k, u16 scan, u8 chr, u8 mod, bool isDown);
void Keyboard_PopEvent(TerminalKeyboard *k);
TerminalKbdEvent *Keyboard_PeekEvent(TerminalKeyboard *k);

// DEVICE UPDATE (on cpu tick)
void Timer_Update(struct TaleaMachine *m, u32 cycles);

// DEVICE RENDER

// DEVICE PORT IO HANDLERS
u8   Terminal_Read(struct TaleaMachine *m, u8 port);
void Terminal_Write(struct TaleaMachine *m, u8 port, u8 value);

void Serial_CloseSockets(struct TaleaMachine *m);
void Modem_ProcessCommand(struct TaleaMachine *m, u8 byte);
void Modem_ProcessData(struct TaleaMachine *m, u8 byte);

void Serial_Update(struct TaleaMachine *m);
bool NetworkInit(void);
bool NetworkDeinit(void);

#endif
