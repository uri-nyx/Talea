#include <string.h>

#include "core/bus.h"
#include "terminal.h"

#include "talea.h"

// Ports

u8 Terminal_Read(TaleaMachine *m, u8 port)
{
    switch (port & 0xf) {
    case P_TERMINAL_SERIAL_DATA: {
        u8 c = Serial_PopByte(&m->terminal.serial);
        TALEA_LOG_TRACE("CHARACTER: %d, %c\n", c, c);
        return c;
    }
    case P_TERMINAL_SERIAL_STATUS: return m->terminal.serial.status;
    case P_TERMINAL_SERIAL_CTRL: return m->terminal.serial.control;
    case P_TERMINAL_SERIAL_RXCOUNT:
        if (m->terminal.serial.status & TERMINAL_SER_STATUS_DATA_AVAILABLE) {
            return m->terminal.serial.head - m->terminal.serial.tail;
        } else {
            return 0;
        }
    case P_TERMINAL_TIMER_TIMEOUT: return m->terminal.timer.timeoutCounter >> 8;
    case P_TERMINAL_TIMER_TIMEOUT + 1: return m->terminal.timer.timeoutCounter;
    case P_TERMINAL_TIMER_INTERVAL: return m->terminal.timer.intervalCounter >> 8;
    case P_TERMINAL_TIMER_INTERVAL + 1: return m->terminal.timer.intervalCounter;
    case P_TERMINAL_TIMER_PRESCALER: return m->terminal.timer.prescaler;
    case P_TERMINAL_TIMER_CSR: return m->terminal.timer.csr; break;
    case P_TERMINAL_KBD_CSR: {
        TALEA_LOG_TRACE("READ_FROM_KBD_CSR\n");
        if (m->terminal.kb.csr & TERMINAL_KB_GET_CSR) {
            // If bit 4 is set: Return Config (bits 0-3)
            u8 config = m->terminal.kb.csr;
            // Clear the flag (Destructive Read)
            m->terminal.kb.csr &= ~TERMINAL_KB_GET_CSR;
            return config;
        } else {
            // If bit 4 is clear: Return Live Modifiers
            TerminalKbdEvent *e   = Keyboard_PeekEvent(&m->terminal.kb);
            u8                val = e->modifiers;
            return val;
        }
    }
    case P_TERMINAL_KBD_CHAR: {
        // CHARACTER Port: Read char and immediately POP
        TerminalKbdEvent *e   = Keyboard_PeekEvent(&m->terminal.kb);
        u8                val = e->character;
        return val;
    }
    case P_TERMINAL_KBD_CODE: {
        // SCANCODE High Byte: Peek only, do NOT pop
        TerminalKbdEvent *e = Keyboard_PeekEvent(&m->terminal.kb);
        u8                v = (u8)(e->scancode >> 8);
        return (u8)((e->scancode >> 8) | ((e->isDown) ? 0x80 : 0)); // TODO: Document that the high
                                                                    // bit is set if is a keydown
    }
    case P_TERMINAL_KBD_CODE + 1: {
        // SCANCODE Low Byte: Read and POP
        TerminalKbdEvent *e = Keyboard_PeekEvent(&m->terminal.kb);
        Keyboard_PopEvent(&m->terminal.kb);
        return (u8)(e->scancode & 0xff);
    }
    default: return 0xde;
    }
    return 0xde;
}

void Terminal_Write(TaleaMachine *m, u8 port, u8 value)
{
    extern void Serial_SendByte(TaleaMachine * m, u8 byte);

    switch (port & 0xf) {
    case P_TERMINAL_SERIAL_DATA: {
        // COMMAND and data modes
        Serial_SendByte(m, value);
        break;
    }
    case P_TERMINAL_SERIAL_STATUS: break; // non writable register
    case P_TERMINAL_SERIAL_CTRL: {
        if (value & TERMINAL_SER_CONTROL_MASTER_RESET) {
            // clear all buffers
            memset(m->terminal.serial.rxFifo, 0, sizeof(m->terminal.serial.rxFifo));
            m->terminal.serial.head = 0;
            m->terminal.serial.tail = 0;
        }

        m->terminal.serial.control = value;
        m->terminal.serial.control &= ~TERMINAL_SER_CONTROL_MASTER_RESET;
        break;
    }
    case P_TERMINAL_SERIAL_RXCOUNT: break; // non writable register
    case P_TERMINAL_TIMER_TIMEOUT:
        m->terminal.timer.timeoutCounter = (m->terminal.timer.timeoutCounter & 0x00FF) | (u16)value
                                                                                             << 8;
        break;
    case P_TERMINAL_TIMER_TIMEOUT + 1:
        m->terminal.timer.timeoutCounter = (m->terminal.timer.timeoutCounter & 0xFF00) | value;
        break;
    case P_TERMINAL_TIMER_INTERVAL:
        m->terminal.timer.intervalCounter = (m->terminal.timer.intervalCounter & 0x00FF) |
                                            (u16)value << 8;
        m->terminal.timer.intervalReload = (m->terminal.timer.intervalReload & 0x00FF) | (u16)value
                                                                                             << 8;
        break;
    case P_TERMINAL_TIMER_INTERVAL + 1:
        m->terminal.timer.intervalCounter = (m->terminal.timer.intervalCounter & 0xFF00) | value;
        m->terminal.timer.intervalReload  = (m->terminal.timer.intervalReload & 0xFF00) | value;
        break;
    case P_TERMINAL_TIMER_PRESCALER: m->terminal.timer.prescaler = value; break;
    case P_TERMINAL_TIMER_CSR: m->terminal.timer.csr = value; break;
    case P_TERMINAL_KBD_CSR:
        TALEA_LOG_TRACE("WRITTEN TO TERMINAL CSR %x\n", value);
        if (value & TERMINAL_KB_GET_CSR)
            m->terminal.kb.csr |= TERMINAL_KB_GET_CSR;
        else
            m->terminal.kb.csr = value;
        break;
    default: break;
    }
}

extern void Serial_HostInit(TerminalSerial *dev, int port);
extern void Serial_CloseSockets(TaleaMachine *m);
void        Terminal_Reset(TaleaMachine *m, TaleaConfig *conf, bool isRestart)
{
    DeviceTerminal *t = &m->terminal;

    t->kb.head = 0;
    t->kb.tail = 0;
    memset(t->kb.queue, 0, sizeof(t->kb.queue));

    // Reset timer
    t->kb.csr                = 0;
    t->kb.modifiers          = 0;
    t->timer.csr             = 0;
    t->timer.timeoutCounter  = 0;
    t->timer.intervalCounter = 0;
    t->timer.intervalReload  = 0;
    t->timer.prescaler       = 0;
    t->timer.cycles          = 0;
    t->timer.accumT          = 0;
    t->timer.accumI          = 0;

    // Reset Serial and Modem
    t->serial.head = 0;
    t->serial.tail = 0;
    memset(t->serial.rxFifo, 0, TERMINAL_SERIAL_FIFO_SIZE);

    t->serial.status  = 0; // Carrier Lost, No Data
    t->serial.control = 0; // Interrupts Disabled

    t->serial.modem.state = TERMINAL_MODEM_STATE_COMMAND;
    memset(t->serial.modem.cmdBuffer, 0, sizeof(t->serial.modem.cmdBuffer));
    t->serial.modem.cmdPos = 0;
    Modem_ResetSregs(&t->serial.modem);

    t->serial.modem.lastTxTime = 0.0;
    t->serial.modem.plusCount  = 0;

    if (isRestart) Serial_CloseSockets(m);
    if (!isRestart)
        Serial_HostInit(&m->terminal.serial, 4321); // TODO: PUT the config
                                                    // value for the port here
}