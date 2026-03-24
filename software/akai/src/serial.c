#include "hw.h"
#include "kernel.h"
#include "libsirius/keys.h"

#define err A.pr.curr->last_error

#if 0
// TODO: implement proper serial driver
static bool ser_event_push(u32 e)
{
    u8 next_head = (A.kb.events.head + 1);
    /* If the next position is where the tail is, we are full */
    if (next_head == A.kb.events.tail) {
        /* Optional: Signal a beep or flash a debug LED here */
        return false;
    }

    A.kb.events.data[A.kb.events.head] = e;

    A.kb.events.head = next_head;
    return true;
}

/* Returns 0 on empty queue */
static u32 ser_event_peek(void)
{
    /* If the head is where the tail is, we are full */
    if (A.kb.events.head == A.kb.events.tail) {
        return 0;
    }

    return (signed)A.kb.events.data[A.kb.events.tail];
}

/* Returns 0 on empty queue */
static u32 ser_event_pop(void)
{
    u32 k;

    /* If the head is where the tail is, we are full */
    if (A.kb.events.head == A.kb.events.tail) {
        return 0;
    }

    k                = A.kb.events.data[A.kb.events.tail];
    A.kb.events.tail = (A.kb.events.tail + 1);
    return k;
}


void ser_scan_hook(u32 *win, struct IPCMessage *msg_out)
{
    usize           i;
    u8             *wait_queue;
    ProcessPID      kb_owner;
    struct Process *owner;
    bool            dance;
    u32             event, sreg;

    sreg       = _disable_interrupts();
    wait_queue = A.kb.wait_queue;
   

    _restore_interrupts(sreg);
}

#endif

u8 ser_in(u8 port)
{
    if (port > A.devices[DEV_SERIAL].ports) {
        err = P_ERROR_NO_PORT;
        return 0;
    }

    return _lbud(A.devices[DEV_SERIAL].base + port);
}

i32 ser_out(u8 port, u8 value)
{
    if (port > A.devices[DEV_SERIAL].ports) {
        err = P_ERROR_NO_PORT;
        return (signed)A_ERROR;
    }

    _sbd(A.devices[DEV_SERIAL].base + port, value);
    return (signed)A_OK;
}

i32 ser_reset(void)
{
    return (signed)A_OK;
}

i32 ser_ctl(u32 command, void *buff, u32 len)
{
    switch (command) {
    case DEV_RESET: return ser_reset();
    case SCTL_READ: {
        // maybe later put to sleep until its filled with a cannon
        u32   count = _lbud(A.info.terminal + TERMINAL_SERIAL_RXCOUNT);
        u8   *out   = (u8 *)buff;
        usize i;

        count = count > len ? len : count;

        for (i = 0; i < count; i++) out[i] = _lbud(A.info.terminal + TERMINAL_SERIAL_DATA);
        return count;
    }
    case SCTL_WRITE: {
        u8   *in   = (u8 *)buff;
        usize i;

        for (i = 0; i < len; i++) _sbd(A.info.terminal + TERMINAL_SERIAL_DATA, in[i]);
        return len;
    }
        /* stream operations */
        /* TODO: IMPLEMENT
    case PX_READ: {
        if (A.kb.canonical & IN_CANON) {
            if (A.kb.line_ready) {
                len = len > A.kb.pos ? A.kb.pos : len;
                memcpy(buff, A.kb.line, len);
                A.kb.line_ready = false;
                A.kb.pos        = 0;
                return len;
            } else {
                process_wait(A.pr.curr->pid);
                BIT_SET(A.kb.wait_queue, A.pr.curr->pid);
                process_yield();
                return (signed)A_OK;
            }

        } else if (A.kb.canonical & IN_BLOCK) {
            u8   *out = (u8 *)buff;
            usize i;
            _trace(0xf0f0A01);
            for (i = 0; i < len; i++) {
                u8 c = ser_event_to_ansi();
                _trace(0xDEFAD01, c, i);
                if (c == 0) {
                    A.kb.blocking = true;
                    A.kb.next     = i;
                    process_wait(A.pr.curr->pid);
                    BIT_SET(A.kb.wait_queue, A.pr.curr->pid);
                    process_yield();
                    return (signed)A_OK;
                };
                out[i] = c;
            }
            return (i32)i;
        } else {
            u8   *out = (u8 *)buff;
            usize i;
            _trace(0xf0f0A);
            for (i = 0; i < len; i++) {
                u8 c = ser_event_to_ansi();
                _trace(0xDEFAD, c, i);
                if (c == 0) break;
                out[i] = c;
            }
            return (i32)i;
        }
    }
    case PX_POLL: {
        u32 head = A.kb.events.head;
        u32 tail = A.kb.events.tail;
        u32 diff = head >= tail ? head - tail : (256 - tail) + head;

        if (A.kb.canonical & IN_CANON) {
            return (signed)A.kb.line_ready;
        } else {
            if (diff > 0 || A.kb.ansi_seq || A.kb.printable)
                return 1;
            else
                return 0;
        }
    }
    case PX_SETCANON: {
        u8 mode;
        if (len != 1) {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }

        mode = *(u8 *)buff;
        if ((mode & IN_CANON) != (A.kb.canonical & IN_CANON)) {
            ser_reset();
        }

        A.kb.canonical = mode;
        A.kb.pos       = 0;

        return (signed)A_OK;
    }

    case PX_GETCANON: return (signed)A.kb.canonical;
        */

    default: err = P_ERROR_NO_CTL_COMMAND; return (signed)A_ERROR_CTL;
    }
}

void ser_driver_init(void)
{
    A.devices[DEV_SERIAL].base  = A.info.terminal + TERMINAL_SERIAL_DATA;
    A.devices[DEV_SERIAL].ports = 4;
    A.devices[DEV_SERIAL].id    = 'S';
    A.devices[DEV_SERIAL].num   = DEV_SERIAL;
    A.devices[DEV_SERIAL].reset = ser_reset;
    A.devices[DEV_SERIAL].in    = ser_in;
    A.devices[DEV_SERIAL].out   = ser_out;
    A.devices[DEV_SERIAL].ctl   = ser_ctl;
    ser_reset();
}
