#include <string.h>

#include "core/bus.h"
#include "talea.h"

#ifdef DEBUG_LOG_SOCKET
// FIXME: LOG
#define LOG_SOCKET_ERROR(error_text) fprintf(stderr, "SOCKET: %s [%d]\n", error_text, tcs_error);
#else
#define LOG_SOCKET_ERROR(error_text) (void)(error_text)
#endif
// Ports

#define P_SERIAL_DATA    (DEV_TTY_BASE + 0)
#define P_SERIAL_STATUS  (DEV_TTY_BASE + 1)
#define P_SERIAL_CTRL    (DEV_TTY_BASE + 2)
#define P_SERIAL_RXCOUNT (DEV_TTY_BASE + 3)

#define P_TIMER_TIMEOUT   (DEV_TIMER_BASE + 0)
#define P_TIMER_INTERVAL  (DEV_TIMER_BASE + 2)
#define P_TIMER_PRESCALER (DEV_TIMER_BASE + 4)
#define P_TIMER_CSR       (DEV_TIMER_BASE + 5)

#define P_KBD_CSR  (DEV_KEYBOARD_BASE + 0)
#define P_KBD_CHAR (DEV_KEYBOARD_BASE + 1)
#define P_KBD_CODE (DEV_KEYBOARD_BASE + 2)

static u8 term_counter = 0;
static u8 term_lenght  = 0;

void Timer_Update(TaleaMachine *m, u32 cycles)
{
    TerminalTimer *t = &m->terminal.timer;

    if (!(t->csr & TIM_GLOBAL_EN) || t->prescaler == 0) return;

    t->accum_i += cycles;
    while (t->accum_i >= t->prescaler) {
        t->accum_i -= t->prescaler;
        if (t->csr & TIM_INTERVAL_EN) {
            if (t->interval_counter > 0) {
                t->interval_counter--;
            } else {
                t->interval_counter = t->interval_reload;
                Machine_RaiseInterrupt(m, INT_TIMER_INTERVAL, PRIORITY_INTERVAL_INTERRUPT);
            }
        }
    }

    // TODO: DOCUMENT (Fixed x256 multiplier)
    t->accum_t += cycles;
    // We treat 'prescaler' as the high-byte of a 16-bit prescaler here
    u32 timeout_threshold = (u32)t->prescaler << 8;

    while (t->accum_t >= timeout_threshold) {
        t->accum_t -= timeout_threshold;
        if (t->csr & TIM_TIMEOUT_EN) {
            if (t->timeout_counter > 0) {
                t->timeout_counter--;
                if (t->timeout_counter == 0) {
                    t->csr &= ~TIM_TIMEOUT_EN; // Auto-stop
                    Machine_RaiseInterrupt(m, INT_TIMER_TIMEOUT, PRIORITY_INTERVAL_INTERRUPT);
                }
            }
        }
    }
}

static inline void Keyboard_PushEvent(TerminalKeyboard *k, u16 scan, u8 chr, u8 mod, bool is_down)
{
    u8 next_head = (k->head + 1) % KB_QUEUE_LEN;
    if (next_head == k->tail) {
        TALEA_LOG_TRACE("Pushing to keyboard hw queue failed: queue full\n");
        return;
    }

    k->queue[k->head].scancode  = scan;
    k->queue[k->head].character = chr;
    k->queue[k->head].modifiers = mod;
    k->queue[k->head].is_down   = is_down;

    k->head = next_head;
}

static inline void Keyboard_PopEvent(TerminalKeyboard *k)
{
    if (k->head != k->tail) {
        k->tail = (k->tail + 1) % KB_QUEUE_LEN;
    }
}

        // Returns null on empty queue
static inline KbdEvent *Keyboard_PeekEvent(TerminalKeyboard *k)
{
    if (k->head == k->tail) {
        static KbdEvent null_ev = { 0, 0, false };
        return &null_ev;
    }
    return &k->queue[k->tail];
}

void Keyboard_ProcessKeypress(TaleaMachine *m, bool is_down, int key, u8 chr, u16 mod)
{
    u8   csr  = m->terminal.kb.csr;
    bool fire = false;

    if (csr & KB_GLOBAL_EN) {
        bool want_down   = (csr & KB_IE_DOWN) && is_down;
        bool want_up     = (csr & KB_IE_UP) && !is_down;
        bool char_filter = (csr & KB_IE_CHAR);

        if (want_down || want_up) {
            // If filter is on, only fire if it's a character
            if (char_filter) {
                if (chr != 0) {
                    Keyboard_PushEvent(&m->terminal.kb, key, chr, mod, is_down);
                    Machine_RaiseInterrupt(m, INT_KBD_CHAR, PRIORITY_KEYBOARD_INTERRUPT);
                }
            } else {
                Keyboard_PushEvent(&m->terminal.kb, key, chr, mod, is_down);
                Machine_RaiseInterrupt(m, INT_KBD_SCAN, PRIORITY_KEYBOARD_INTERRUPT);
            }
        }
    }
}

void Serial_PushByte(TaleaMachine *m, u8 byte)
{
    TerminalSerial *s    = &m->terminal.serial;
    u8              next = (s->head + 1) % SERIAL_FIFO_SIZE;

    if (next != s->tail) {
        s->rx_fifo[s->head] = byte;
        s->head             = next;

        s->status |= SER_STATUS_DATA_AVAILABLE;

        if (s->control & SER_CONTROL_INT_EN)
            Machine_RaiseInterrupt(m, INT_TTY_TRANSMIT, PRIORITY_SERIAL_INTERRUPT);
    } else {
        s->status |= SER_STATUS_BUFFER_OVERRUN;
    }
}

void Serial_PushString(TaleaMachine *m, const u8 *str)
{
    TerminalSerial *s          = &m->terminal.serial;
    bool            data_added = false;

    while (*str) {
        u8 next = (s->head + 1) % SERIAL_FIFO_SIZE;
        if (next != s->tail) {
            s->rx_fifo[s->head] = (u8)(*str);
            s->head             = next;
            data_added          = true;
        } else {
            TALEA_LOG_WARNING("Pushing to Serial buffer: buffer overrun\n");
            s->status |= SER_STATUS_BUFFER_OVERRUN;
            break;
        }
        str++;
    }

    if (data_added) {
        s->status |= SER_STATUS_DATA_AVAILABLE;

        if (s->control & SER_CONTROL_INT_EN) {
            Machine_RaiseInterrupt(m, INT_TTY_TRANSMIT, PRIORITY_SERIAL_INTERRUPT);
        }
    }
}

static inline u8 Serial_PopByte(TerminalSerial *s)
{
    u8 value = 0;

    if (s->head != s->tail) {
        value = s->rx_fifo[s->tail++];
        if (s->head == s->tail) s->status &= ~SER_STATUS_DATA_AVAILABLE;
    }

    return value;
}

static inline u8 Serial_PeekByte(TerminalSerial *s)
{
    if (s->head == s->tail) {
        s->status &= ~SER_STATUS_DATA_AVAILABLE;
        return 0;
    }
    return s->rx_fifo[s->tail];
}

void Modem_CheckEscapeSequence(TaleaMachine *m, u8 byte)
{
    HayesModem *modem      = &m->terminal.serial.modem;
    double      now        = GetTime();
    double      elapsed    = now - modem->last_tx_time;
    double      guard_time = GET_GUARD_TIME(modem);

    TALEA_LOG_TRACE("Checking escape sequence (%c) [%d] %d\n", byte, modem->s_regs[2],
                    modem->s_regs[2] == '+');

    if (modem->waiting_after) {
        /* Any character sent during this window invalidates the sequence */
        TALEA_LOG_TRACE("Invalidadated sequence\n");
        modem->waiting_after = false;
        modem->plus_count    = 0;
    }

    if (byte == modem->s_regs[2]) {
        TALEA_LOG_TRACE("It is the character!\n");
        if (modem->plus_count == 0) {
            /* First '+': Must follow a period of silence */
            if (elapsed >= guard_time) {
                TALEA_LOG_TRACE("Got 1st +\n");
                modem->plus_count = 1;
            }
        } else if (modem->plus_count < 3) {
            if (elapsed < guard_time) {
                TALEA_LOG_TRACE("Got another +\n");
                modem->plus_count++;
            } else {
                TALEA_LOG_TRACE("too much time passed\n");
                modem->plus_count = (elapsed >= guard_time) ? 1 : 0;
            }
        }
    } else {
        modem->plus_count = 0;
    }

    modem->last_tx_time = now;

    if (modem->plus_count == 3) {
        TALEA_LOG_TRACE("Got all 3! waiting\n");
        modem->waiting_after = true;
    }
}

u8 Terminal_ReadHandler(TaleaMachine *m, u16 addr)
{
    switch (addr) {
    case P_SERIAL_DATA: {
        return Serial_PopByte(&m->terminal.serial);
    }
    case P_SERIAL_STATUS: return m->terminal.serial.status;
    case P_SERIAL_CTRL: return m->terminal.serial.control;
    case P_SERIAL_RXCOUNT:
        if (m->terminal.serial.status & SER_STATUS_DATA_AVAILABLE) {
            return m->terminal.serial.head - m->terminal.serial.tail;
        } else {
            return 0;
        }
    case P_TIMER_TIMEOUT: return m->terminal.timer.timeout_counter >> 8;
    case P_TIMER_TIMEOUT + 1: return m->terminal.timer.timeout_counter;
    case P_TIMER_INTERVAL: return m->terminal.timer.interval_counter >> 8;
    case P_TIMER_INTERVAL + 1: return m->terminal.timer.interval_counter;
    case P_TIMER_PRESCALER: return m->terminal.timer.prescaler;
    case P_TIMER_CSR: return m->terminal.timer.csr; break;
    case P_KBD_CSR: {
        if (m->terminal.kb.csr & KB_GET_CSR) {
            // If bit 4 is set: Return Config (bits 0-3)
            u8 config = m->terminal.kb.csr;
            // Clear the flag (Destructive Read)
            m->terminal.kb.csr &= ~KB_GET_CSR;
            return config;
        } else {
            // If bit 4 is clear: Return Live Modifiers
            KbdEvent *e   = Keyboard_PeekEvent(&m->terminal.kb);
            u8        val = e->modifiers;
            return val;
        }
    }
    case P_KBD_CHAR: {
        // CHARACTER Port: Read char and immediately POP
        KbdEvent *e   = Keyboard_PeekEvent(&m->terminal.kb);
        u8        val = e->character;
        return val;
    }
    case P_KBD_CODE: {
        // SCANCODE High Byte: Peek only, do NOT pop
        KbdEvent *e = Keyboard_PeekEvent(&m->terminal.kb);
        u8        v = (u8)(e->scancode >> 8);
        return (u8)((e->scancode >> 8) | ((e->is_down) ? 0x80 : 0)); // TODO: Document that the high
                                                                     // bit is set if is a keydown
    }
    case P_KBD_CODE + 1: {
        // SCANCODE Low Byte: Read and POP
        KbdEvent *e = Keyboard_PeekEvent(&m->terminal.kb);
        Keyboard_PopEvent(&m->terminal.kb);
        return (u8)(e->scancode & 0xff);
    }
    default: return 0xde;
    }
    return 0xde;
}

void Terminal_WriteHandler(TaleaMachine *m, u16 addr, u8 value)
{
    extern void Serial_SendByte(TaleaMachine * m, u8 byte);

    switch (addr) {
    case P_SERIAL_DATA: {
        // COMMAND and data modes
        Serial_SendByte(m, value);
    }
    case P_SERIAL_STATUS: break; // non writable register
    case P_SERIAL_CTRL: {
        if (value & SER_CONTROL_MASTER_RESET) {
            // clear all buffers
            memset(m->terminal.serial.rx_fifo, 0, sizeof(m->terminal.serial.rx_fifo));
            m->terminal.serial.head = 0;
            m->terminal.serial.tail = 0;
        }

        m->terminal.serial.control |= value;
        m->terminal.serial.control &= ~SER_CONTROL_MASTER_RESET;
    }
    case P_SERIAL_RXCOUNT: break; // non writable register
    case P_TIMER_TIMEOUT: m->terminal.timer.timeout_counter |= (u16)value << 8; break;
    case P_TIMER_TIMEOUT + 1: m->terminal.timer.timeout_counter |= value; break;
    case P_TIMER_INTERVAL:
        m->terminal.timer.interval_counter |= (u16)value << 8;
        m->terminal.timer.interval_reload |= (u16)value << 8;
        break;
    case P_TIMER_INTERVAL + 1:
        m->terminal.timer.interval_counter |= value;
        m->terminal.timer.interval_reload |= value;
        break;
    case P_TIMER_PRESCALER: m->terminal.timer.prescaler = value; break;
    case P_TIMER_CSR: m->terminal.timer.csr = value; break;
    case P_KBD_CSR: m->terminal.kb.csr = value; break;
    default: break;
    }
}

extern void Serial_HostInit(TerminalSerial *dev, int port);
extern void Serial_CloseSockets(TaleaMachine *m);
void        Terminal_Reset(TaleaMachine *m, TaleaConfig *conf, bool is_restart)
{
    DeviceTerminal *t = &m->terminal;

    t->kb.head = 0;
    t->kb.tail = 0;
    memset(t->kb.queue, 0, sizeof(t->kb.queue));

    // Reset timer
    t->kb.csr       = 0;
    t->kb.modifiers = 0;
    t->timer.csr = 0;
     t->timer.timeout_counter  = 0;
    t->timer.interval_counter = 0;
    t->timer.interval_reload  = 0;
    t->timer.prescaler        = 0;
    t->timer.cycles  = 0;
    t->timer.accum_t = 0;
    t->timer.accum_i = 0;

    // Reset Serial and Modem 
    t->serial.head = 0;
    t->serial.tail = 0;
    memset(t->serial.rx_fifo, 0, SERIAL_FIFO_SIZE);

    t->serial.status  = 0; // Carrier Lost, No Data
    t->serial.control = 0; // Interrupts Disabled

    t->serial.modem.state = MODEM_STATE_COMMAND;
    memset(t->serial.modem.cmd_buffer, 0, sizeof(t->serial.modem.cmd_buffer));
    t->serial.modem.cmd_pos = 0;
    Modem_ResetSregs(&t->serial.modem);

    t->serial.modem.last_tx_time = 0.0;
    t->serial.modem.plus_count   = 0;

    if (is_restart) Serial_CloseSockets(m);
    if (!is_restart)
        Serial_HostInit(&m->terminal.serial, 4321); // TODO: PUT the config
                                                    // value for the port here
}