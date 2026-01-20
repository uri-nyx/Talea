/*

A simple BIOS, the firmware of your Taleä System (TM).

All fictional rights belong to The Patron of the House of Taleä.

All earthly and real rigths belong to its respective autors, find the licenses
in LICENSE-THIRD-PARTY. This project is licensed under the MIT License.

*/

// #include "freestanding/ctype.h"
#include "isr.h"
#include "libsirius/devices.h"
#include "libsirius/keys.h"
#include "libsirius/types.h"
#include "ushell.h"

#define TTTY_IMPLEMENTATION
#define TTTY_USE_PRINTF
#include "tiolib/ttty.h"

#define TINS_IMPLEMENTATION
#include "tiolib/tins.h"

/*
 THIS IS STILL BUGGY
#define TLINE_IMPLEMENTATION
#include "tiolib/tline.h"
*/

/* -------------------CONSTANTS-----------------------------------------------*/

#define BIOS_NAME     "Basic BIOS"
#define BIOS_VERSION  "0.5 beta"
#define BIOS_CHR      'T'
#define GET_SYSTEM(r) _lbud(DEVICE_SYSTEM + (r))

#define STACK_SIZE (64 * 1024)

#define KBD_EVENT_QUEUE_SIZE 32

/* ------------------DECLARATIONS-------------------------------------------- */

/* Defined in entry.s */
extern u8   _lbud(u16 addr);
extern u16  _lhud(u16 addr);
extern u32  _lwd(u16 addr);
extern void _sbd(u16 addr, u8 value);
extern void _shd(u16 addr, u16 value);
extern void _swd(u16 addr, u32 value);

struct KbdEvent {
    u16  scancode;
    u8   character;
    u8   modifiers;
    bool is_keydown;
};

typedef struct KbdEventQueue {
    struct KbdEvent events[KBD_EVENT_QUEUE_SIZE];
    u8              head, tail;
} KbdEventQueue;

/* -------------------- GLOBALS ----------------------------------------------*/

struct device_map Devices;
TttyMux           Con;
Ttty              Video_tty, Serial_tty;

u32        Memsize;
enum TpsId CurrentTps;

void *Stack_top;
void *Stack_base;

u32 Charbuff_size;
u8 *Charbuff;

KbdEventQueue KeyboardEvents;
Tins          Keyboard_in;
static char   command[160];
/* ---------------------------------------------------------------------------*/
/* ------------------------KEYBOARD DRIVER------------------------------------*/

/* 1. Queue management */

/* returns true on success, false on queue full */
static bool kbd_event_push(KbdEventQueue *q, struct KbdEvent *e)
{
    u8 next_head = (q->head + 1) % KBD_EVENT_QUEUE_SIZE;
    /* If the next position is where the tail is, we are full */
    if (next_head == q->tail) {
        /* Optional: Signal a beep or flash a debug LED here */
        return false;
    }

    q->events[q->head].scancode   = e->scancode;
    q->events[q->head].character  = e->character;
    q->events[q->head].modifiers  = e->modifiers;
    q->events[q->head].is_keydown = e->is_keydown;

    q->head = next_head;
    return true;
}

/* Returns NULL on empty queue */
static struct KbdEvent *kbd_event_peek(KbdEventQueue *q)
{
    /* If the head is where the tail is, we are full */
    if (q->head == q->tail) {
        return NULL;
    }

    return &q->events[q->tail];
}

/* Returns NULL on empty queue */
static struct KbdEvent *kbd_event_pop(KbdEventQueue *q)
{
    struct KbdEvent *k;

    /* If the head is where the tail is, we are full */
    if (q->head == q->tail) {
        return NULL;
    }

    k       = &q->events[q->tail];
    q->tail = (q->tail + 1) % KBD_EVENT_QUEUE_SIZE;
    return k;
}

/* 2. ISR */

/* Interrupt service routine for the keyboard interrupt */
void bios_kbd_handler(void)
{
    static struct KbdEvent e;
    bool                   success  = false;
    u32                    keypress = _lwd(Devices.keyboard + KBD_CSR);

    _trace(0xdead, keypress, (keypress >> 16) & 0xff, keypress & (~0x8000U));

    e.modifiers = keypress >> 24;
    e.character = (keypress >> 16) & 0xff;
    /* TODO: check that this does not conflict with raylib keymap */
    e.scancode   = (keypress & (~0x8000U)); /* remove is_down flag */
    e.is_keydown = (keypress & 0x8000U) >> 15;

    success = kbd_event_push(&KeyboardEvents, &e);
}

/* 3. ANSI compatibility */

static struct tins_ansi *kbd_event_to_ansi(struct KbdEvent *e)
{
    static struct tins_ansi trans;
    trans.seq            = NULL;
    trans.printable_char = 0;

    /* clang-format off */
    switch (e->scancode) {
    case KEY_UP:              trans.seq = ANSI_UP;        return &trans;
    case KEY_DOWN:            trans.seq = ANSI_DOWN;      return &trans;
    case KEY_LEFT:            trans.seq = ANSI_LEFT;      return &trans;
    case KEY_RIGHT:           trans.seq = ANSI_RIGHT;     return &trans;
    case KEY_HOME:            trans.seq = ANSI_HOME;      return &trans;
    case KEY_END:             trans.seq = ANSI_END;       return &trans;
    case KEY_DELETE:          trans.seq = ANSI_DELETE;    return &trans;
    case KEY_PAGE_UP:         trans.seq = ANSI_PGUP;      return &trans;
    case KEY_PAGE_DOWN:       trans.seq = ANSI_PGDOWN;    return &trans;
    case KEY_F1:              trans.seq = ANSI_F1;        return &trans;
    case KEY_F2:              trans.seq = ANSI_F2;        return &trans;
    case KEY_F3:              trans.seq = ANSI_F3;        return &trans;
    case KEY_F4:              trans.seq = ANSI_F4;        return &trans;
    }
    /* clang-format on */

    trans.printable_char = e->character;
    /* TODO: handle ALT- key sequences (ESC-key)*/
    return &trans;
}

int kbd_get_translated(KbdEventQueue *q, bool *peeked)
{
    static struct tins_ansi *pending;
    static bool              has_pending = false;
    int                      c;

    if (!has_pending) {
        /* we need an event to translate */
        struct KbdEvent *e = kbd_event_peek(q);
        *peeked            = true;
        if (!e) return TINS_NO_DATA;

        pending = kbd_event_to_ansi(e);
        if (pending->seq == NULL) {
            if (pending->printable_char)
                return pending->printable_char;
            else
                return TINS_NO_DATA; /* how to handle this */
        } else {
            has_pending = true;
            c           = *pending->seq++;
            return c;
        }
    } else {
        *peeked = false;
        if (c = *pending->seq++)
            return c;
        else
            has_pending = false;
        return TINS_NO_DATA;
    }
}

/* 4. Integration with tins.h */

static int kbd_get_byte(void *priv)
{
    int            c;
    bool           peeked = false;
    KbdEventQueue *q      = (KbdEventQueue *)priv;

    c = kbd_get_translated(q, &peeked);
    if (peeked) kbd_event_pop(q);

    return (c == '\r') ? '\n' : c;
}

/* Creating the actual interface instance */
void kbd_init_tins(Tins *t)
{
    t->priv     = &KeyboardEvents;
    t->get_byte = kbd_get_byte;
}

/*----------------------------------------------------------------------------*/
/*----------------------SERIAL DRIVER-----------------------------------------*/

/* 1. Status */

bool serial_is_connected()
{
    u8 status = _lbud(Devices.tty + SER_STATUS);
    return status & SER_STATUS_CARRIER_DETECT;
}

bool serial_available()
{
    u8 status = _lbud(Devices.tty + SER_STATUS);
    return status & SER_STATUS_DATA_AVAILABLE;
}

/* 2. TX/RX */

#define ser_rx(t)    _lbud((t) + SER_RX)
#define ser_tx(t, v) _sbd((t) + SER_TX)

/* 3. Interface with ttty.h */

void serial_outc(Ttty *tty, char c)
{
    u8 dev_tty = *(u8 *)tty->priv;
    u8 status  = _lbud(dev_tty + SER_STATUS);
    if (status & SER_STATUS_CARRIER_DETECT) {
        _sbd(dev_tty + SER_TX, c);
    };
}

/*----------------------------------------------------------------------------*/
/*----------------------VIDEO DRIVER------------------------------------------*/

/* 1. info */

/* Get the text buffer sizes (cols*rows*bpc) */
void video_text_screen_size(u8 *out_cols, u8 *out_rows, u8 *out_bpc)
{
    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_SYS_INFO);

    *out_bpc  = _lbud(Devices.video + VIDEO_GPU3);
    *out_cols = _lbud(Devices.video + VIDEO_GPU4);
    *out_rows = _lbud(Devices.video + VIDEO_GPU5);
}

void video_set_cursor(u8 x, u8 y)
{
    _sbd(Devices.video + VIDEO_CUR_X, x);
    _sbd(Devices.video + VIDEO_CUR_Y, y);
}

void video_set_csr(u8 csr)
{
    _sbd(Devices.video + VIDEO_CSR, csr);
}

void video_get_cursor(u8 *outx, u8 *outy)
{
    *outx = _lbud(Devices.video + VIDEO_CUR_X);
    *outy = _lbud(Devices.video + VIDEO_CUR_Y);
}

u8 video_get_csr(void)
{
    return _lbud(Devices.video + VIDEO_CSR);
}

/* 2. Interface with ttty.h */

static u8 video_text_mode_default_attr[3] = {
    0xf, // fg white
    0x0, // bg black
    0x4, // att transparent
};

/* Outputs a character to the screen  */
/* Checks have already been performed */
void video_outc(Ttty *tty, char c)
{
    u8 *vram  = (u8 *)tty->priv;
    u16 index = (((u16)tty->y * (u16)tty->w) + (u16)tty->x) * tty->bpc;

    vram[index] = c;

    if (tty->bpc > 1) {
        usize i;
        for (i = 1; i < tty->bpc; i++) vram[index + i] = tty->default_att[i - 1];
    }
}

void video_setpos(Ttty *tty, u8 x, u8 y)
{
    video_set_cursor(x, y);
}

/*----------------------------------------------------------------------------*/
/*---------------------DISCOVERY AND INITIALIZATION---------------------------*/

static void find_devices(void)
{
    u8 i, dev_id, audio;
    u8 tty, video, storage, mouse;
    for (i = 0; i < 16; i++) {
        dev_id = _lbud(DEVICE_MAP + i);

        switch (dev_id) {
        case DEVICE_ID_TTY: tty = i * 0x10; break;
        case DEVICE_ID_VIDEO: video = i * 0x10; break;
        case DEVICE_ID_STORAGE: storage = i * 0x10; break;
        case DEVICE_ID_AUDIO: audio = i * 0x10; break;
        case DEVICE_ID_MOUSE: mouse = i * 0x10; break;
        default: break;
        }
    }

    Devices.tty      = tty;
    Devices.timer    = tty + 6;
    Devices.keyboard = tty + 12;
    Devices.video    = video;
    Devices.tps      = storage;
    Devices.hcs      = storage + 6;
    Devices.audio    = audio;
    Devices.mouse    = mouse;
}

static void bios_init(void)
{
    usize i;
    u8    rows, cols, bpc;

    KeyboardEvents.head = 0;
    KeyboardEvents.tail = 0;

    Memsize    = (u32)_lbud(DEVICE_SYSTEM + PORTS_SYSTEM_MEMSIZE) * 0x100000;
    Stack_top  = (void *)(Memsize - (32 * 1024));
    Stack_base = (void *)((u32)Stack_top - STACK_SIZE);

    find_devices();

    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_BEGIN_DRAWING);
    _sbd(Devices.video + VIDEO_GPU0, VIDEO_MODE_TEXT_AND_GRAPHIC);
    _sbd(Devices.video + VIDEO_GPU7, 0); // latch
    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_SET_MODE);
    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_END_DRAWING);

    // wait for vblank
    {
        usize i, wait;
        for (i = 0; i < 100000; i++) {
            wait += i;
        }
    }

    video_text_screen_size(&cols, &rows, &bpc);
    Charbuff_size = rows * cols * bpc;
    Charbuff      = (u8 *)((u32)Stack_base - 1024 - Charbuff_size);

    _trace(0x1, cols, rows, bpc);

    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_BEGIN_DRAWING);

    _swd(Devices.video + VIDEO_GPU0, 0xffffffff);
    _swd(Devices.video + VIDEO_GPU4, (u32)Charbuff);
    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_SET_ADDR);

    _swd(Devices.video + VIDEO_GPU0, 0x20000000);
    _sbd(Devices.video + VIDEO_GPU4, 0x0);
    _sbd(Devices.video + VIDEO_GPU7, 0); // latch
    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_CLEAR);

    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_END_DRAWING);

    // wait for vblank
    {
        usize i, wait;
        for (i = 0; i < 1000090; i++) {
            wait += i;
        }
    }

    /* TODO: use a proper driver for the video */
    ttty_init(&Video_tty, "System Console", TTTY_XYLF | TTTY_ANSI, cols, rows, bpc, (void *)Charbuff,
              video_text_mode_default_attr, video_outc, video_setpos);
    ttty_init(&Serial_tty, "Dumb Terminal", TTTY_STREAM, 0xff, 0xff, 1, (void *)&Devices.tty, NULL,
              serial_outc, NULL);

    ttty_mux_subscribe(&Con, &Video_tty, TTTY_INFO);
    ttty_mux_subscribe(&Con, &Serial_tty, TTTY_INFO);
    ttty_mux_set_priority(&Con, TTTY_NONE);

    /* Set keyboard to only report keydown */
    _sbd(Devices.keyboard + KBD_CSR, KB_IE_DOWN | KB_GLOBAL_EN);
    kbd_init_tins(&Keyboard_in);

    CurrentTps = 0;
}
/*----------------------------------------------------------------------------*/
/* ----------------------------- ushell --------------------------------------*/

void ushell_putchar(char chr)
{
    ttty_mux_putc(&Con, chr);
}

void sysinfo(int argc, char *argv[])
{
    /*
                SYSTEM INFORMATION
    The Talea Computer System. v0.5-beta

    [ CORE ]
    Processor:   Sirius-24 @ 10MHz
    Memory:      16384KiB
    Firmware:    Basic BIOS v0.5 beta
    Uptime:      100 seconds

    [ DEVICES ]
    ADDR  ID    TYPE             STATUS
    ----  ----  ---------------  -------
    0x20  0x05  Storage Ctl      [ONLINE]
    0x23  0x02  Serial Modem     [CARRIER OK]
    */
    u32 mhz, uptime, khz;

    if (argc != 1) {
        ttty_mux_puts(&Con, "Usage: sysinfo\n");
        return;
    }

    ttty_mux_puts(&Con, "\t\t\tSYSTEM INFORMATION\n");
    ttty_mux_puts(&Con, "The Talea Computer System. v0.5-beta\n\n");
    ttty_mux_puts(&Con, "[ CORE ]\n");

    /* Processor */
    /* TODO: Do a bit of fixed point */
    khz = 10000 + (GET_SYSTEM(PORTS_SYSTEM_CLOCK) * 353);
    ttty_mux_printf(&Con, "Processor:\t\t%s (%s) @ %dKhz\n",
                    (GET_SYSTEM(PORTS_SYSTEM_ARCHID) == ARCH_ID) ? "Sirius" : "unknown",
                    (GET_SYSTEM(PORTS_SYSTEM_VENDOR) == VENDOR_ID) ? "of the House of Talea" :
                                                                     "of unknown origin",
                    khz);

    /* Memory */
    ttty_mux_printf(&Con, "Memory:\t\t%dKiB\n", GET_SYSTEM(PORTS_SYSTEM_MEMSIZE) * 1024);

    /* Firmware */
    ttty_mux_puts(&Con, "Firmware:\t" BIOS_NAME " v" BIOS_VERSION "\n");

    /* Uptime */
    _sbd(DEVICE_SYSTEM + PORTS_SYSTEM_MINUTE, 1); /* Set the COUNTER MODE */
    uptime = _lwd(DEVICE_SYSTEM + PORTS_SYSTEM_MINUTE);

    ttty_mux_printf(&Con, "Uptime:\t\t%d seconds\n\n", uptime / 1000); /* to seconds */

    ttty_mux_puts(&Con, "[ DEVICES ]\n");
    ttty_mux_printf(&Con, "0x%x\t%c\tSerial Modem\t[ %s ]\n", Devices.tty, DEVICE_ID_TTY,
                    serial_is_connected() ? "CARRIER OK" : "NO CARRIER");

    ttty_mux_printf(&Con, "0x%x\t%c\tDual Timer\n", Devices.timer, DEVICE_ID_TTY);
    ttty_mux_printf(&Con, "0x%x\t%c\tKeyboard\n", Devices.keyboard, DEVICE_ID_TTY);
    ttty_mux_printf(&Con, "0x%x\t%c\tGraphics and Video Processor\n", Devices.video,
                    DEVICE_ID_VIDEO);
    ttty_mux_printf(&Con, "0x%x\t%c\tTPS Controller\n", Devices.tps, DEVICE_ID_STORAGE);
    ttty_mux_printf(&Con, "0x%x\t%c\tHCS Controller\n", Devices.hcs, DEVICE_ID_STORAGE);
    ttty_mux_printf(&Con, "0x%x\t%c\tAudio Synthesizer\n", Devices.audio, DEVICE_ID_AUDIO);
    ttty_mux_printf(&Con, "0x%x\t%c\tPointer (mouse)\n", Devices.mouse, DEVICE_ID_MOUSE);
}

void echo(int argc, char *argv[])
{
    if (argc >= 2 && argv[1][0] == '-') {
        char sw = argv[1][1];
        switch (sw) {
help:
        case 'h':
            ttty_mux_printf(&Con, "Usage: echo -[h|a|e] <args>\n");
            ttty_mux_printf(&Con, "\t-h\tprint this help\n");
            ttty_mux_printf(&Con, "\t-a\tprepend \\x1b (ESC) to process ANSI escape code\n");
            ttty_mux_printf(&Con, "\t-e\techo args without processing\n");
            return;
        case 'a': {
            int i = 0;
            for (i = 2; i < argc; i++) {
                ttty_mux_printf(&Con, "\x1b%s", argv[i]);
                ttty_mux_putc(&Con, ' ');
            }
            ttty_mux_putc(&Con, '\n');
            return;
        }
        case 'e':
        default: {
            int i = 0;
            for (i = 2; i < argc; i++) {
                ttty_mux_puts(&Con, argv[i]);
                ttty_mux_putc(&Con, ' ');
            }
            ttty_mux_putc(&Con, '\n');
            return;
        }
        }
    } else if (argc > 1) {
        int i = 0;
        for (i = 1; i < argc; i++) {
            ttty_mux_puts(&Con, argv[i]);
            ttty_mux_putc(&Con, ' ');
        }
        ttty_mux_putc(&Con, '\n');
        return;
    } else {
        goto help;
    }
}

char *strchr(const char *s, int c)
{
    do {
        if (*s == c) {
            return (char *)s;
        }
    } while (*s++);
    return (NULL);
}

u8 to_lower(u8 c)
{
    if (c >= 'A' && c <= 'Z') return c + 32;
}

bool is_space(u8 c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

long strtol(const char *s, char **endp, int base)
{
    long        sign = 1, ok = 0;
    char       *p, *digits;
    const char *src;
    long        v = 0;

    if (base < 0 || base > 36) {
        // errno = EINVAL;
        return 0;
    }
    src = s;
    while (is_space(*s)) s++;
    if ('-' == *s)
        sign = -1, s++;
    else if ('+' == *s)
        s++;
    if ((0 == base || 16 == base) && '0' == *s && 'x' == s[1]) {
        base = 16;
        s += 2;
    } else if ((0 == base || 8 == base) && '0' == *s) {
        base = 8;
        s++;
    }
    if (0 == base) base = 10;
    digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    for (;;) {
        p = strchr(digits, to_lower(*s));
        if (!p || p - digits >= base) break;
        v = v * base + p - digits;
        s++;
        ok = 1;
    }
    if (endp) *endp = ok ? (char *)s : (char *)src;
    return v * sign;
}

#define ULONG_MAX 0xffffff

bool is_print(u8 c)
{
    return (c >= ' ' && c != 127);
}

bool is_digit(u8 c)
{
    return (c >= '0' && c <= '9');
}

bool is_alpha(u8 c)
{
    return (c >= 'A' && c <= 'z');
}

bool is_upper(u8 c)
{
    return (c >= 'A' && c <= 'Z');
}

unsigned long strtoul(const char *nptr, char **endptr, int base)
{
    const char   *s = nptr;
    unsigned long acc;
    unsigned char c;
    unsigned long cutoff;
    int           neg = 0, any, cutlim;

    /*
     * See strtol for comments as to the logic used.
     */
    do {
        c = *s++;
    } while (is_space(c));
    if (c == '-') {
        neg = 1;
        c   = *s++;
    } else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) base = c == '0' ? 8 : 10;
    cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
    cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (!is_print(c)) break;
        if (is_digit(c))
            c -= '0';
        else if (is_alpha(c))
            c -= is_upper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base) break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = ULONG_MAX;
    } else if (neg)
        acc = -acc;
    if (endptr != 0) *((const char **)endptr) = any ? s - 1 : nptr;
    return (acc);
}

void peek(int argc, char *argv[])
{
    u32 addr = 0, end_addr, i = 0;

    if (argc != 2) {
        ttty_mux_printf(&Con, "Usage: peek <addr> \n");
        ttty_mux_printf(&Con, "\t<addr>\taddress in hex to peek (128 bytes)\n");
        return;
    }

    addr     = strtoul(argv[1], NULL, 0);
    end_addr = addr + 128;

    for (i = addr & ~0xf; i < end_addr; i += 16 /* 16 bytes per row*/) {
        /* TODO: add padding to printf*/
        u8  j = 0;
        u8 *m = (u8 *)0;
        ttty_mux_printf(&Con, "%x -> %x", addr, *(u8 *)addr);
        ttty_mux_printf(&Con, "%x:", i);
        ttty_mux_printf(&Con, "\t%x %x %x %x %x %x %x %x", m[i], m[i + 1], m[i + 2], m[i + 3],
                        m[i + 4], m[i + 5], m[i + 6], m[i + 7]);
        ttty_mux_printf(&Con, "|%x %x %x %x %x %x %x %x\t", m[i + 8], m[i + 9], m[i + 10],
                        m[i + 11], m[i + 12], m[i + 13], m[i + 14], m[i + 15]);
        ttty_mux_putc(&Con, '|');
        for (j = 0; j < 16; j++) {
            u8 c = m[j + i] ? m[j + i] : '.';
            ttty_mux_putc(&Con, c);
            if (j == 7) ttty_mux_putc(&Con, '|');
        }
        ttty_mux_putc(&Con, '|');
        ttty_mux_putc(&Con, '\n');
    }
}

void poke(int argc, char *argv[])
{
    u32 addr = 0, value = 0;

    if (argc != 3) {
        ttty_mux_printf(&Con, "Usage: poke <addr> <value>\n");
        ttty_mux_printf(&Con, "\t<addr>\taddress in hex to poke\n");
        ttty_mux_printf(&Con, "\t<value>\tvalue to set (a byte)\n");
        return;
    }

    addr        = strtoul(argv[1], NULL, 0);
    value       = strtoul(argv[2], NULL, 0);
    *(u8 *)addr = value & 0xff;
}

void clear(int argc, char *argv[])
{
    ttty_mux_puts(&Con, "\x1b[2J\x1b[H"); /*BUG: resets propmtp at last column of first row*/
}

/* SERIAL MODEM TESTS */

void send_escape(int argc, char *argv[])
{
    int i     = 0;
    u32 times = 0;
    for (i = 0; i < 1000000; i++) times += i;
    _sbd(Devices.tty + SER_TX, '+');
    for (i = 0; i < 10000; i++) times += i;
    _sbd(Devices.tty + SER_TX, '+');
    for (i = 0; i < 10000; i++) times += i;
    _sbd(Devices.tty + SER_TX, '+');
    for (i = 0; i < 1000000; i++) times += i;
}

void ser(int argc, char *argv[])
{
    struct KbdEvent *e;

    while (true) {
        e = kbd_event_pop(&KeyboardEvents);
        if (e != NULL) {
            if (e->character != 0) _sbd(Devices.tty + SER_TX, e->character);
            if (e->scancode == KEY_ESCAPE) break;
        }
        if (serial_available()) {
            u8 i;
            u8 count = _lbud(Devices.tty + SER_RXCOUNT);
            for (i = 0; i < count; i++) ttty_putc(&Video_tty, _lbud(Devices.tty + SER_RX));
        }
    };
}

static synth_write(u8 addr, u8 data)
{
    _sbd(Devices.audio + AUDIO_ADDR, addr);
    _sbd(Devices.audio + AUDIO_DATA, data);
}

#define OCTAVE 4
#define MASK   (AUDIO_GLOB_NOTE_ENDED0 | AUDIO_GLOB_NOTE_ENDED1 | AUDIO_GLOB_NOTE_ENDED2)

#define DUR     800
#define BASE_CH 0

static bool vol_control(void)
{
    // controls volume and retruns true on esc pressed
    struct KbdEvent *e;

    static u8 vol = 5;

    e = kbd_event_pop(&KeyboardEvents);
    if (e != NULL) {
        if (e->scancode == KEY_ESCAPE)
            return true;
        else if (e->scancode == KEY_UP && vol < 15) {
            vol += 1;
            _sbd(Devices.audio + AUDIO_MASTER_VOL, vol);
        } else if (e->scancode == KEY_DOWN && vol > 0) {
            vol -= 1;
            _sbd(Devices.audio + AUDIO_MASTER_VOL, vol);
        }
    }

    return false;
}

/* AUDIO TESTS */

#define F_C 0x110
#define F_D 0x132
#define F_E 0x157
#define F_F 0x16B
#define F_G 0x197
#define F_A 0x1C9
#define F_B 0x1F0

#define NOTE(blk, f) (((blk) << 9) | (f))

void play_chord(u16 f1, u16 f2, u16 f3)
{
    u32 status;
    int i;
    u16 freqs[3];
    freqs[0] = f1;
    freqs[1] = f2;
    freqs[2] = f3;

    for (i = 0; i < 3; i++) {
        _sbd(Devices.audio + AUDIO_CHANNEL_SELECT, BASE_CH + i);
        _shd(Devices.audio + AUDIO_FNUMH, freqs[i]);
        _shd(Devices.audio + AUDIO_DURH, DUR);
        _sbd(Devices.audio + AUDIO_CSR, AUDIO_CSR_TRIGGER);
    }

    /* Wait for the most significant byte to show the flags (Big Endian) */
    do {
        if (vol_control()) return;
        status = _lwd(Devices.audio + AUDIO_GLOBAL_STATUS0);
    } while ((status & MASK) != MASK);

    /* Acknowledge and clear the flags */
    _sbd(Devices.audio + AUDIO_GLOBAL_STATUS0, MASK);
}

void fanfare(int argc, char *argv[])
{
    /* Set Instruments: Brass (7) and Horn (9) */
    synth_write(0x30, 0x70); /* Ch 0: Trumpet */
    synth_write(0x31, 0x72); /* Ch 1: Trumpet (slightly quieter) */
    synth_write(0x32, 0x94); /* Ch 2: Horn */

    while (1) {
        if (vol_control()) break;
        // C Major
        play_chord(NOTE(4, F_C), NOTE(4, F_E), NOTE(4, F_G));
        // F Major
        play_chord(NOTE(4, F_F), NOTE(4, F_A), NOTE(5, F_C));
        // G Major
        play_chord(NOTE(3, F_G), NOTE(3, F_B), NOTE(4, F_D));
    }
}

void pcm(int argc, char *argv[])
{
    /* Simple 8-point sine wave table (scaled to i16) */
    static i16 sine_lut[] = { 0,      8149,   15918,  22958,  28959,  32767,  32767,  32767,
                              28959,  22958,  15918,  8149,   0,      -8149,  -15918, -22958,
                              -28959, -32767, -32767, -32767, -28959, -22958, -15918, -8149 };
    u8         i          = 0;

    while (1) {
        /* 1. Check if FIFO is full by reading the 32-bit status */
        /* (Assuming P_AUDIO_GLOBAL_STATUS0 is the start of the 4-byte block) */
        u32 status = _lwd(Devices.audio + AUDIO_GLOBAL_STATUS0);
        if (vol_control()) break;

        if (!(status & AUDIO_GLOB_PCM_FIFO_FULL)) {
            /* 2. Push the next sample using a Halfword Store */
            /* This triggers P_AUDIO_PCM_FIFOH then P_AUDIO_PCM_FIFOL */
            _shd(Devices.audio + AUDIO_PCM_FIFOH, sine_lut[i]);

            i = (i + 1) % 24;
        }
    }
}

void pcm_synth(int argc, char *argv[])
{
    /* Simple 8-point sine wave table (scaled to i16) */
    static i16 sine_lut[] = { 0,      8149,   15918,  22958,  28959,  32767,  32767,  32767,
                              28959,  22958,  15918,  8149,   0,      -8149,  -15918, -22958,
                              -28959, -32767, -32767, -32767, -28959, -22958, -15918, -8149 };

    u32 status;
    u8  i = 0;

    synth_write(0x30, 0x90); /* Ch 0: horn */

    _sbd(Devices.audio + AUDIO_CHANNEL_SELECT, BASE_CH);
    _shd(Devices.audio + AUDIO_FNUMH, NOTE(4, F_A));
    _shd(Devices.audio + AUDIO_DURH, 5000);
    _sbd(Devices.audio + AUDIO_CSR, AUDIO_CSR_TRIGGER);

    do {
        /* 1. Check if FIFO is full by reading the 32-bit status */
        /* (Assuming P_AUDIO_GLOBAL_STATUS0 is the start of the 4-byte block) */
        status = _lwd(Devices.audio + AUDIO_GLOBAL_STATUS0);
        if (vol_control()) break;

        if (!(status & AUDIO_GLOB_PCM_FIFO_FULL)) {
            /* Wait for the most significant byte to show the flags (Big Endian) */
            /* 2. Push the next sample using a Halfword Store */
            /* This triggers P_AUDIO_PCM_FIFOH then P_AUDIO_PCM_FIFOL */
            _shd(Devices.audio + AUDIO_PCM_FIFOH, sine_lut[i]);

            i = (i + 1) % 24;
        }

    } while ((status & AUDIO_GLOB_NOTE_ENDED0) == 0);
}

void tamlin(int argc, char *argv[])
{
    i16  *samples     = (i16 *)0x1000;
    usize len         = 171902 / 2;
    usize curr_sample = 0;

    if (samples[0] != -7484) {
        ttty_mux_printf(&Con, "Tamlin is not loaded, or data is corrupted\n"
                              "(you must load the tamlin.raw file with the emulator)");
        return;
    }

    while (curr_sample < len) {
        u32 status = _lwd(Devices.audio + AUDIO_GLOBAL_STATUS0);
        if (vol_control()) break;

        if (!(status & AUDIO_GLOB_PCM_FIFO_FULL)) {
            /* 2. Push the next sample using a Halfword Store */
            /* This triggers P_AUDIO_PCM_FIFOH then P_AUDIO_PCM_FIFOL */
            _shd(Devices.audio + AUDIO_PCM_FIFOH, samples[curr_sample]);
            curr_sample++;
        }
    }
}

/* VIDEO TEST */
#define SCREEN_W  640
#define SCREEN_H  480
#define CTR_X     320
#define CTR_Y     240
#define NUM_STARS 100

static u8   video_context    = 0;
static bool video_is_drawing = false;

static void video_begin_drawing(u8 context)
{
    video_context    = context;
    video_is_drawing = true;
    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_BEGIN_DRAWING);
}

static void video_end_drawing(void)
{
    video_is_drawing = false;
    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_END_DRAWING);
}

static void video_clear(u8 color, u8 flags)
{
    if (!video_is_drawing) _sbd(Devices.video + VIDEO_COMMAND, COMMAND_BEGIN_DRAWING);
    _swd(Devices.video + VIDEO_GPU0, 0x20000004);
    _sbd(Devices.video + VIDEO_GPU4, color);
    _sbd(Devices.video + VIDEO_GPU5, flags);
    _sbd(Devices.video + VIDEO_GPU7, 0);
    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_CLEAR);
    if (!video_is_drawing) _sbd(Devices.video + VIDEO_COMMAND, COMMAND_END_DRAWING);
}

static void draw_line(u8 color, i16 x0, i16 y0, i16 x1, i16 y1)
{
    _sbd(Devices.video + VIDEO_GPU0, video_context);
    _sbd(Devices.video + VIDEO_GPU1, color);
    _shd(Devices.video + VIDEO_GPU4, x0);
    _shd(Devices.video + VIDEO_GPU6, y0);
    _shd(Devices.video + VIDEO_GPU0, x1);
    _shd(Devices.video + VIDEO_GPU2, y1);
    _sbd(Devices.video + VIDEO_GPU7, 0);
    _sbd(Devices.video + VIDEO_COMMAND, COMMAND_DRAW_LINE);
}

void bouncing_box(void)
{
    static i32 box_x    = 100 << 16;
    static i32 box_y    = 100 << 16;
    static u32 dx       = 150000;
    static u32 dy       = 120000;
    u16        box_size = 80;
    i16        current_x, current_y, x1, y1;

    box_x += dx;
    box_y += dy;

    current_x = box_x >> 16;
    current_y = box_y >> 16;

    if (current_x < -40 || current_x > 600) dx = -dx;
    if (current_y < -40 || current_y > 440) dy = -dy;


    video_begin_drawing(0);

    video_clear(0x5, VIDEO_CLEAR_FLAG_FB);

    draw_line(255, 0, 240, 639, 240);
    draw_line(255, 320, 0, 320, 479);

    x1 = current_x + box_size;
    y1 = current_y + box_size;

    draw_line(0xb, current_x, current_y, x1, current_y); // Top
    draw_line(0xb, x1, current_y, x1, y1);               // Right
    draw_line(0xb, x1, y1, current_x, y1);               // Bottom
    draw_line(0xb, current_x, y1, current_x, current_y); // Left

    video_end_drawing();

    ttty_printf(&Video_tty, "\x1b[30;61HBox {x: %03d, y: %03d}", current_x + box_size / 2, current_y + box_size / 2);
}

static void (*on_vblank)(void) = NULL;

void test2d(int argc, char *argv[])
{
    bool escape = false;
    u32  vcsr   = _lbud(Devices.video + VIDEO_CSR);

    video_clear(0, VIDEO_CLEAR_FLAG_TB | VIDEO_CLEAR_FLAG_FB);

    on_vblank = &bouncing_box;

    vcsr |= 1; // vblank enable
    _sbd(Devices.video + VIDEO_CSR, vcsr);

    do {
        escape = vol_control();
    } while (!escape);

    vcsr = _lbud(Devices.video + VIDEO_CSR);
    vcsr &= ~1; // vblank enable
    _sbd(Devices.video + VIDEO_CSR, vcsr);

    video_clear(0, VIDEO_CLEAR_FLAG_TB | VIDEO_CLEAR_FLAG_FB);
    on_vblank = NULL;

    ttty_clear(&Video_tty);
}

void bios_vblank_handler(void)
{
    if (on_vblank) on_vblank();
}

static const ushell_command_t commands[] = {
    /* SYSTEM TEST */
    { "sysinfo", "prints system information", &sysinfo },
    { "echo", "echoes its arguments", &echo },
    { "peek", "print a memory dump", &peek },
    { "poke", "set a value in memory", &poke },
    /* SERIAL MODEM TEST */
    { "ser", "drop to a serial terminal to the modem", &ser },
    { "escape", "test the AT escape secuence", &send_escape },
    /* AUDIO TEST */
    { "fanfare", "play a simple fanfare. esc to quit", &fanfare },
    { "pcm", "play a pcm sine wave", &pcm },
    { "audio", "play a pcm sine wave and A4", &pcm_synth },
    { "tamlin", "play a bit of Tam Lin", &tamlin },
    /* VIDEO TEST */
    { "box", "a very simple 2d demo", &test2d },
    // TODO: 3d test
    // TODO: rich text mode test
    /* UI (KEYBOARD AND MOUSE TEST) */
    // TODO: mouse test (click & drag & position (hover))
    // TODO: keyboard tester (scancode & character)
    /* STORAGE TEST */
    // TODO: tps test (load sector & store sector)
    // TODO: hcs test (load sector & store sector)
    /* TIMER TEST*/
    // TODO: set interval timer
    // TODO: set timeout timer
    /* UTILITIES */
    { "clear", "clear the screen", &clear }
};

/**************************************************************************************************/
/***************************** BIOS ENTRY POINT AND MAIN LOOP *************************************/
/**************************************************************************************************/

void bios_start(void)
{
    usize i, wait;
    u8    x, y;

    bios_init();

    for (i = 0; i < 100000; i++) {
        wait += i;
    }

    ttty_mux_clear(&Con);

    ttty_mux_printf(&Con, "%s %s %c\n", BIOS_NAME, BIOS_VERSION, BIOS_CHR);
    ttty_printf(&Video_tty, "Terminal size: %d cols, %d rows, %d bpc\n", Video_tty.w, Video_tty.h,
                Video_tty.bpc);

    ushell_init(commands, sizeof(commands) / sizeof(commands[0]));

    while (true) {
        u8 chr = tins_getc(&Keyboard_in);
        ushell_process(chr);
    }
}
