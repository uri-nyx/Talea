#define INCLUDE_ANSI_IN
#include "ansi.h"
#include "hw.h"
#include "kernel.h"
#include "libsirius/keys.h"

#define err A.pr.curr->last_error

static bool kbd_event_push(u32 e)
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
static u32 kbd_event_peek(void)
{
    /* If the head is where the tail is, we are full */
    if (A.kb.events.head == A.kb.events.tail) {
        return 0;
    }

    return (signed)A.kb.events.data[A.kb.events.tail];
}

/* Returns 0 on empty queue */
static u32 kbd_event_pop(void)
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

static u8 kbd_event_to_ansi()
{
    u32 event, scancode;
    u8  character, modifiers;

send_byte:
    if (A.kb.ansi_seq) {
        u8 c = *A.kb.ansi_seq++;
        _trace(0xDADAE, 1, c);
        if (*A.kb.ansi_seq == '\0') A.kb.ansi_seq = NULL;
        return c;
    } else if (A.kb.printable != 0) {
        u8 c = A.kb.printable;
        _trace(0xDADAE, 2, c);

        A.kb.printable = 0;
        return c;
    }

another:
    event = kbd_event_pop();
    _trace(0xDADAE, 3, event);

    if (event == 0) return 0; /* no data in queue */
    if (!(event & 0x8000U)) goto another;

    modifiers = event >> 24;
    character = (event >> 16) & 0xff;
    scancode  = (u16)(event & (~0x8000U)); /* remove is_down flag */

    A.kb.ansi_seq  = NULL;
    A.kb.printable = 0;

    /* clang-format off */
    switch (scancode) {
    case KEY_UP:              A.kb.ansi_seq = ANSI_UP;        break;
    case KEY_DOWN:            A.kb.ansi_seq = ANSI_DOWN;      break;
    case KEY_LEFT:            A.kb.ansi_seq = ANSI_LEFT;      break;
    case KEY_RIGHT:           A.kb.ansi_seq = ANSI_RIGHT;     break;
    case KEY_HOME:            A.kb.ansi_seq = ANSI_HOME;      break;
    case KEY_END:             A.kb.ansi_seq = ANSI_END;       break;
    case KEY_DELETE:          A.kb.ansi_seq = ANSI_DELETE;    break;
    case KEY_PAGE_UP:         A.kb.ansi_seq = ANSI_PGUP;      break;
    case KEY_PAGE_DOWN:       A.kb.ansi_seq = ANSI_PGDOWN;    break;
    case KEY_F1:              A.kb.ansi_seq = ANSI_F1;        break;
    case KEY_F2:              A.kb.ansi_seq = ANSI_F2;        break;
    case KEY_F3:              A.kb.ansi_seq = ANSI_F3;        break;
    case KEY_F4:              A.kb.ansi_seq = ANSI_F4;        break;
    }
    /* clang-format on */

    if (A.kb.ansi_seq == NULL && (modifiers & LALT) && character != 0) {
        A.kb.alt_buf[0] = 0x1B;
        A.kb.alt_buf[1] = character;
        A.kb.alt_buf[2] = '\0';
        A.kb.ansi_seq   = A.kb.alt_buf;
        A.kb.printable  = 0;
    } else if (A.kb.ansi_seq == NULL) {
        A.kb.printable = character;
    }

    if (A.kb.ansi_seq || A.kb.printable) goto send_byte;

    goto another;
}

void kbd_scan_hook(u32 *win, struct IPCMessage *msg_out)
{
    usize           i;
    u8             *wait_queue;
    ProcessPID      kb_owner;
    struct Process *owner;
    bool            dance;
    u32             event, sreg;

    sreg       = _disable_interrupts();
    wait_queue = A.kb.wait_queue;
    kb_owner   = DEED_OWNER(&A.devices[DEV_KEYBOARD].deed);
    owner      = &A.pr.proc[kb_owner];
    dance      = false;
    event      = _lwd(A.info.terminal + TERMINAL_KBD_CSR);

    // KILL SWITCH
    if ((u16)(event & (~0x8000U)) == AKAI_KILL_SWITCH && kb_owner > 1) {
        miniprint("Kill switch pressed, owner %d (%s)\n", kb_owner, owner->name);
        process_terminate(kb_owner, WARRANT_ABORT);
        kbd_reset();
        process_yield();
    } else if ((u16)(event & (~0x8000U)) == AKAI_KILL_SWITCH && kb_owner <= 1) {
        miniprint("Kill switch pressed, owner %d (%s). Yielding\n", kb_owner, owner->name);
        process_yield();
    }

    // TODO: define messages internal protocol for interrupts
    msg_out->sender  = KERNEL_PID;
    msg_out->type    = 1; // I dont know, KB, or something
    msg_out->msgid   = 0;
    msg_out->subject = event;
    msg_out->content = NULL;

    if (A.kb.canonical & IN_CANON) {
        u8 c;

        kbd_event_push(event);
get_ansi:
        c = kbd_event_to_ansi();

        _trace(0xDAFB0, 1, c);
        if (c == 0) {
            _restore_interrupts(sreg);
            return;
        }

        switch (c) {
        case '\n':
        case '\r':
            A.kb.line_ready = true;
            if (A.kb.pos < LINE_BUFFER_LEN) {
                c = (A.kb.canonical & IN_CRNL) ? '\n' : c;

                A.kb.line[A.kb.pos++] = c;
            }
            break;
        case '\b':
            if (A.kb.pos > 0) {
                A.kb.pos--;
                dance = true;
            }
            break;
        default:
            if (A.kb.pos < LINE_BUFFER_LEN) {
                A.kb.line[A.kb.pos++] = c;
            } else {
                A.kb.line_ready = true;
            }
            break;
        }

        _trace(0xDAFB11, A.kb.canonical & IN_ECHO, owner->stdout.res_type == HW,
               owner->stdout.res.hw->num == DEV_TEXTBUFFER);
        _trace(0xDAFB11, A.kb.canonical);
        if ((A.kb.canonical & IN_ECHO) && owner->stdout.res_type == HW &&
            owner->stdout.res.hw->num == DEV_TEXTBUFFER) {
            _trace(0xABCD, c);
            video_emit(c);
            if (dance) {
                video_emit(' ');
                video_emit(c);
            }
        }

        // maybe rollong back the syscall is not the worst idea
        if (A.kb.line_ready && BIT_TEST(wait_queue, kb_owner)) {
            u8 *buff = (u8 *)owner->ctx.regs[15];
            u32 len  = owner->ctx.regs[16];

            _trace(0xDAFB0, 2, kb_owner);

            len = len > A.kb.pos ? A.kb.pos : len;

            _trace(0xDAFB0, 3, buff, len);
            copy_to_user(owner, buff, A.kb.line, len);
            _trace(0xDAFB0, 4, len);

            BIT_CLR(wait_queue, kb_owner);
            process_set_ready(kb_owner);

            _trace(0xDAFB12, owner->ctx.pc, owner->ctx.status, owner->ctx.usp);
            _trace(0xDAFB12, owner->ctx.wp, owner->ctx.regs[1]);
            _trace(0xDAFB0, 3, kb_owner);
            owner->ctx.regs[10] = len;
            A.kb.line_ready     = false;
            A.kb.pos            = 0;
            _restore_interrupts(sreg);
            return;
        } else if (!A.kb.line_ready && A.kb.ansi_seq != NULL) {
            goto get_ansi;
        }

    } else if ((A.kb.canonical & IN_BLOCK) && A.kb.blocking) {
        u8  c;
        u8 *out      = (u8 *)owner->ctx.regs[15];
        u32 expected = owner->ctx.regs[16];

        kbd_event_push(event);
        c = kbd_event_to_ansi();

        do {
            if (c == 0) {
                _restore_interrupts(sreg);
                return;
            }

            if (BIT_TEST(wait_queue, kb_owner) && A.kb.next < expected) {
                copy_to_user(owner, out + A.kb.next++, &c, 1);
                if (A.kb.next == expected) {
                    BIT_CLR(wait_queue, kb_owner);
                    process_set_ready(kb_owner);
                    owner->ctx.regs[10] = A.kb.next;
                    A.kb.blocking       = false;
                    _restore_interrupts(sreg);
                    return;
                }
            }
        } while ((c = kbd_event_to_ansi()));
    } else {
        if (BIT_TEST(wait_queue, kb_owner)) {
            BIT_CLR(wait_queue, kb_owner);
            process_set_ready(kb_owner);
            owner->ctx.regs[10] = event;
            _restore_interrupts(sreg);
            return;
        } else {
            _trace(0xDACA, event);
            kbd_event_push(event);
        }
    }

    _restore_interrupts(sreg);
}

u8 kbd_in(u8 port)
{
    if (port > A.devices[DEV_KEYBOARD].ports) {
        err = P_ERROR_NO_PORT;
        return 0;
    }

    return _lbud(A.devices[DEV_KEYBOARD].base + port);
}

i32 kbd_out(u8 port, u8 value)
{
    if (port > A.devices[DEV_KEYBOARD].ports) {
        err = P_ERROR_NO_PORT;
        return (signed)A_ERROR;
    }

    _sbd(A.devices[DEV_KEYBOARD].base + port, value);
    return (signed)A_OK;
}

i32 kbd_reset(void)
{
    A.kb.events.tail = A.kb.events.head;
    A.kb.ansi_seq    = NULL;
    A.kb.printable   = 0;
    A.kb.pos         = 0;
    memset(A.kb.alt_buf, 0, sizeof(A.kb.alt_buf));
    memset(A.kb.line, 0, sizeof(A.kb.line));
    memset(A.kb.wait_queue, 0, sizeof(A.kb.wait_queue));
    A.kb.canonical  = 0;
    A.kb.line_ready = false;
    return (signed)A_OK;
}

i32 kbd_ctl(u32 command, void *buff, u32 len)
{
    switch (command) {
    case DEV_RESET: return kbd_reset();
    case KCTL_GETKEY: return (i32)kbd_event_pop();
    case KCTL_WAITKEY: {
        u32 e = kbd_event_pop();
        if (e == 0) {
            process_wait(A.pr.curr->pid);
            BIT_SET(A.kb.wait_queue, A.pr.curr->pid);
            process_yield();
            return (signed)A_OK;
        } else {
            return (i32)e;
        }
    }
    case KCTL_PEEK: return (i32)kbd_event_peek();
    case KCTL_QUEUE_COUNT: {
        u32 head = A.kb.events.head;
        u32 tail = A.kb.events.tail;
        u32 diff = head >= tail ? head - tail : (256 - tail) + head;
        return (i32)diff;
    }
        /* stream operations */
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
                u8 c = kbd_event_to_ansi();
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
                u8 c = kbd_event_to_ansi();
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
            kbd_reset();
        }

        A.kb.canonical = mode;
        A.kb.pos       = 0;

        return (signed)A_OK;
    }

    case PX_GETCANON: return (signed)A.kb.canonical;

    default: err = P_ERROR_NO_CTL_COMMAND; return (signed)A_ERROR_CTL;
    }
}

void kbd_driver_init(void)
{
    A.devices[DEV_KEYBOARD].base  = A.info.terminal + TERMINAL_KBD_CSR;
    A.devices[DEV_KEYBOARD].ports = 3;
    A.devices[DEV_KEYBOARD].id    = 'K';
    A.devices[DEV_KEYBOARD].num   = DEV_KEYBOARD;
    A.devices[DEV_KEYBOARD].reset = kbd_reset;
    A.devices[DEV_KEYBOARD].in    = kbd_in;
    A.devices[DEV_KEYBOARD].out   = kbd_out;
    A.devices[DEV_KEYBOARD].ctl   = kbd_ctl;
    kbd_reset();
}
