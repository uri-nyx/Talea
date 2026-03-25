/* Nanocom -- a simple serial terminal */

#include <akai.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MENU_IMPLEMENTATION
#include <a/menu.h>

/* use the xterm pallete */
#include "xterm_palette.h"

#define NANOVERSION "0.1"

enum ResultModes {
    MODE_UNKNOWN = -1,
    MODE_NUMERIC,
    MODE_VERBOSE,
};

#define HAYES_OK          0
#define HAYES_CONNECT     1
#define HAYES_RING        2
#define HAYES_NO_CARRIER  3
#define HAYES_ERROR       4
#define HAYES_NO_DIALTONE 5
#define HAYES_BUSY        6
#define HAYES_NO_ANSWER   7

#define STR_HELPER(x) #x
#define STR(x)        STR_HELPER(x)
const char *results_numeric[] = {
    STR(HAYES_OK) "\r",         STR(HAYES_CONNECT) "\r",   STR(HAYES_RING) "\r",
    STR(HAYES_NO_CARRIER) "\r", STR(HAYES_ERROR) "\r",     STR(HAYES_NO_DIALTONE) "\r",
    STR(HAYES_BUSY) "\r",       STR(HAYES_NO_ANSWER) "\r",
};
#undef STR
#undef STR_HELPER

const char *results_verbose[] = {
    "OK\n\r",    "CONNECT\n\r",     "RING\n\r", "NO_CARRIER\n\r",
    "ERROR\n\r", "NO_DIALTONE\n\r", "BUSY\n\r", "NO_ANSWER\n\r",
};

static const u32 rates[] = { 300, 1200, 2400, 9600, 19200, 38400, 57600 };

#define CTRL(c) ((c) & 0x1f)

enum NanoMode {
    NANO_MODE_NORMAL,
    NANO_MODE_COMMAND,
    NANO_MODE_QUIT,
    NANO_MODE_MODEM,
    NANO_MODE_TRANSFER,
    NANO_MODE_OPTIONS,
    NANO_MODE_HELP,
    NANO_PROMPT_DIAL     = 0X100,
    NANO_PROMPT_PROTOCOL = 0X101,
    NANO_PROMPT_UPLOAD   = 0X102,
    NANO_PROMPT_DOWNLOAD = 0X103,
    NANO_PROMPT_SPEED    = 0X200,
    NANO_PROMPT_TERMIOS  = 0X201,
};

/*
STATUS MENU:
NORMAL MODE:
[NORMAL][C-a: menu]             [HOST:20chars max    ][CARRIER][RING][57600baud]
*/
#define STATUS_NORMAL_A "[NORMAL][C-a: menu]"
#define STATUS_NORMAL_B "[%*s:%d][%s][%d baud]"
/*
COMMAND MODE:
[COMMAND][C-a: back]       [q: quit][m: modem][t: transfer][o: options][h: help]
*/
#define STATUS_COMMAND_A "[COMMAND][C-a: back]"
#define STATUS_COMMAND_B "[q: quit][m: modem][t: transfer][o: options][h: help]"
/*
QUIT prompt:
[QUIT][C-a: back]                            [x: hang up and quit][q: just quit]
*/
#define STATUS_QUIT_A "[QUIT][C-a: back]"
#define STATUS_QUIT_B "[x: hang up and quit][q: just quit]"
/*
MODEM MENU:
[MODEM][C-a: back]                       [d: dial][x: hang up][m: modem console]
*/
#define STATUS_MODEM_A "[MODEM][C-a: back]"
#define STATUS_MODEM_B "[d: dial][x: hang up][m: modem console]"
/*
DIAL prompt:
[DIAL][_cursor here and prompt for host:port]
(goes back to NORMAL mode)
*/
#define STATUS_DIAL_A "[DIAL][C-a: cancel]: "
/*
TRANSFER MENU:
[TRANS][C-a: back]                                      [d: download][u: upload]
*/
#define STATUS_TRANS_A "[TRANS][C-a: back]"
#define STATUS_TRANS_B "[d: download][u: upload]"
/*
TRANSFER prompt:
[PROTOCOL][C-a: cancel]                                   [y: ymodem][x: xmodem]
*/
#define STATUS_PROTOCOL_A "[PROTOCOL][C-a: back]"
#define STATUS_PROTOCOL_B "[y: ymodem][x: xmodem]"
/*
UPLOAD/DOWNOLAD progress:
[UPLOAD][C-a: cancel]           [PROTOCOL][FILENAME][xxxx/xxxx KB][xx%][xxxKB/s]
*/
#define STATUS_UPLOAD_A   "[UPLOAD][C-a: cancel]"
#define STATUS_UPLOAD_B   "[%s][%s][%d/%d KB][%3d][%4d KB/s]"
#define STATUS_DOWNLOAD_A "[DOWNLOAD][C-a: cancel]"
#define STATUS_DOWNLOAD_B "[%s][%s][%d/%d KB][%3d][%4d KB/s]"
/*
OPTIONS MENU:
[OPTIONS][C-a: back]                     [s: set speed][t: set terminal options]
*/
#define STATUS_OPTIONS_A "[OPTIONS][C-a: back]"
#define STATUS_OPTIONS_B "[s: set speed][t: set terminal options]"
/*
SPEED prompt:
[SPEED][C-a: back][57000 baud] [1:300|2:1200|3:2400|4:9600|5:19K|6:38K|7:57K]
*/
#define STATUS_SPEED_A "[SPEED][C-a: back][%d baud]"
#define STATUS_SPEED_B "[1:300|2:1200|3:2400|4:9600|5:19K|6:38K|7:57K]"
/*
TERMINAL prompt:
[TERMIOS][C-a: back]  [1:NORWAP X|2:NOSCROLL X|3:ECHO -|4:ROWS 30|5:COLS 80]
*/
#define STATUS_TERMIOS_A "[TERMIOS][C-a: back]"
#define STATUS_TERMIOS_B "[1:NORWAP %c|2:NOSCROLL %c|3:ECHO %c|4:ROWS %3d|5:COLS %3d]"

static int nano_mode = NANO_MODE_NORMAL;

struct {
    int   baud;
    char *host;
    int   port;
    char  escape;
    char *startup_cmd;
    u8    term_w, term_h;
    u8    imode, omode;
} nano_ops;

bool serial_is_connected()
{
    int  status = ak_dev_in(DEV_SERIAL, TERMINAL_SERIAL_STATUS);
    bool ret    = (status >= 0 && (status & SER_STATUS_CARRIER_DETECT));
    if (!ret) nano_ops.host = NULL;
    return ret;
}

bool serial_available()
{
    int status = ak_dev_in(DEV_SERIAL, TERMINAL_SERIAL_STATUS);
    if (status < 0) fprintf(stderr, "Error querying: %d\r\n", status);
    return (status >= 0 && (status & SER_STATUS_DATA_AVAILABLE));
}

static void cursor_show(void)
{
    ak_dev_ctl(PDEV_STDOUT, PX_WRITE, "\x1b[?25h", 6);
}

static void cursor_hide(void)
{
    ak_dev_ctl(PDEV_STDOUT, PX_WRITE, "\x1b[?25l", 6);
}
#define ser_rx    ak_dev_in(DEV_SERIAL, TERMINAL_SERIAL_DATA)
#define ser_tx(v) ak_dev_out(DEV_SERIAL, TERMINAL_SERIAL_DATA, (v))

static int old_imode;
static int old_omode;

static void send_modem(const char *s, bool exec)
{
    char *p = (char *)s;

    while (*p) ak_dev_out(DEV_SERIAL, TERMINAL_SERIAL_DATA, *p++);
    if (exec) ak_dev_out(DEV_SERIAL, TERMINAL_SERIAL_DATA, '\r');
}

static void send_modem_escape(void)
{
    clock_t start = clock();
    int     i     = 0;

    for (i = 0; i < 3; i++) {
        while (clock() - start < CLOCKS_PER_SEC / 2);
        start = clock();
        send_modem("+", false);

        menu_printf("%.*s", i, "..............");
    }

    start = clock();

    i = 0;
    while (clock() - start < CLOCKS_PER_SEC * 4) {
        char c  = '?';
        int  id = i++ % 6;
        // | / - \ | -
        if (id == 0) c = '|';
        if (id == 1) c = '/';
        if (id == 2) c = '-';
        if (id == 3) c = '\\';
        if (id == 4) c = '|';
        if (id == 5) c = '-';

        menu_printf("%c", c);
    }
}

static int result_modem(int mode)
{
    char buf[64];
    int  len, i;

    for (i = 0; i < 3; i++) {
        /* read one line */
        len = 0;
        while (len < (int)sizeof(buf) - 1) {
            int r;
            if (!serial_available()) continue;

            r = ser_rx;
            if (r < 0) return -2;

            buf[len++] = (char)r;
            if (r == '\r') break;
        }
        buf[len] = '\0';

        /* try numeric first */
        if (mode == MODE_UNKNOWN || mode == MODE_NUMERIC) {
            int i;
            for (i = 0; i < 8; i++) {
                if (strcmp(buf, results_numeric[i]) == 0) return i;
            }
            if (mode == MODE_NUMERIC) continue; /* numeric mode only → skip non-matching lines */
        }

        /* try verbose */
        {
            int i;
            for (i = 0; i < 8; i++) {
                if (strncmp(buf, "CONNECT", 7) == 0) return HAYES_CONNECT;
                if (strcmp(buf, results_verbose[i]) == 0) return i;
            }
        }

        /* not a result → echo or noise → read next line */
    }

    return -1;
}

static bool serial_init(void)
{
    int res;
    u8  imode[1] = { 0 };
    u8  omode[1] = { OUT_NOWRAP };

    if (ak_dev_claim(DEV_SERIAL) != A_OK) {
        return false;
    }

    if (ak_dev_claim(DEV_TEXTBUFFER) != A_OK) {
        fprintf(stderr, "Unable to claim text buffer\n");
        return false;
    }

    old_imode = ak_dev_ctl(PDEV_STDIN, PX_GETCANON, NULL, 0);
    old_omode = ak_dev_ctl(PDEV_STDOUT, PX_GETCANON, NULL, 0);

    res = ak_dev_ctl(DEV_TEXTBUFFER, VCTL_LOAD_PALETTE, xterm_palette, sizeof(xterm_palette));
    if (res) {
        fprintf(stderr, "Error loading pallete. press any key to continue: %d\r\n", res);
        getchar();
    }

    res = ak_dev_ctl(PDEV_STDIN, PX_SETCANON, imode, 1);
    if (res) return false;

    res = ak_dev_ctl(PDEV_STDOUT, PX_SETCANON, omode, 1);
    if (res) return false;

    return true;
}

static void serial_deinit(void)
{
    int res;
    // TODO: here, actually close connection with ATH
    u8 imode[1];
    u8 omode[1];

    imode[0] = old_imode;
    omode[0] = old_omode;

    ak_dev_ctl(DEV_TEXTBUFFER, VCTL_LOAD_PALETTE_DEFAULT, NULL, 0);

    res = ak_dev_ctl(PDEV_STDIN, PX_SETCANON, imode, 1);
    res = ak_dev_ctl(PDEV_STDOUT, PX_SETCANON, omode, 1);
    fputs("\x1b[0m\x1b[H", stderr);
}

static void status_line(char *a, int n, char *b)
{
    int w   = nano_ops.term_w;
    int gap = w - (n + strlen(a));
    gap     = gap < 0 ? 0 : gap;
    printf("%s%*s%s", a, gap, "", b);
}

static void on_update(void *s)
{
    int   n;
    char  ss[80];
    bool  is_connected = serial_is_connected();
    char *con          = is_connected ? "CONNECTED" : "NO CARRIER";
    char *host         = !is_connected ? "disconnected" : nano_ops.host ? nano_ops.host : "unknown";
    int   hs           = strlen(host);

    cursor_hide();
    n = sprintf(ss, STATUS_NORMAL_B, hs, host, nano_ops.port, con, rates[nano_ops.baud]);
    status_line(STATUS_NORMAL_A, n, ss);
    cursor_show();
}

static void on_activation(void *a)
{
    int n = strlen(STATUS_COMMAND_B);
    status_line(STATUS_COMMAND_A, n, STATUS_COMMAND_B);
    cursor_hide();
}

static void on_menu(void *a)
{
    char **v = (char **)a;
    int    n = strlen(v[1]);
    status_line(v[0], n, v[1]);
}

#define MENU_ENTER(s)                   \
    do {                                \
        char *__v[] = { s##_A, s##_B }; \
        menu_update(on_menu, __v);      \
    } while (0);

static void m_command(int c)
{
    menu_update(on_activation, NULL);

    switch (c) {
    case CTRL('A'):
        menu_deactivate(on_update, NULL);
        nano_mode = NANO_MODE_NORMAL;
        break;

    case 'q': {
        MENU_ENTER(STATUS_QUIT);
        nano_mode = NANO_MODE_QUIT;
        break;
    }
    case 'm': {
        MENU_ENTER(STATUS_MODEM);
        nano_mode = NANO_MODE_MODEM;
        break;
    }
    case 't': {
        MENU_ENTER(STATUS_TRANS);
        nano_mode = NANO_MODE_TRANSFER;
        break;
    }
    case 'o': {
        MENU_ENTER(STATUS_OPTIONS);
        nano_mode = NANO_MODE_OPTIONS;
        break;
    }
    case 'h': nano_mode = NANO_MODE_HELP; break;
    default: break;
    }
}

static bool m_quit(int c)
{
    bool quit = false;
    switch (c) {
    case CTRL('A'):
        menu_update(on_activation, NULL);
        nano_mode = NANO_MODE_COMMAND;
        break;
    case 'x':
        send_modem_escape();
        send_modem("ATH", true);
        quit = true;
        break;
    case 'q': quit = true; break;

    default: break;
    }

    return quit;
}

static char dial_host[81];
static void do_dial(void)
{
    usize pos = 0;
    int   c;

    memset(dial_host, 0, sizeof(dial_host));

    menu_printf(STATUS_DIAL_A);
    cursor_show();

    while ((c = ak_dev_in(PDEV_STDIN, 0)) != '\r') {
        if (c == '\b' && pos) {
            dial_host[pos--] = 0;
            menu_printf(STATUS_DIAL_A "%s\b \b", dial_host);
            continue;
        }
        if (c == CTRL('A')) {
            nano_mode = NANO_MODE_MODEM;
            MENU_ENTER(STATUS_MODEM);
            return;
        }

        if (c <= 0 || !isprint(c) || pos > 79) continue;
        dial_host[pos++] = c;
        menu_printf(STATUS_DIAL_A "%s", dial_host);
    }

    dial_host[pos]     = 0;
    nano_ops.host = dial_host;
    cursor_hide();

    send_modem_escape();
    send_modem("ATH", true);
    send_modem("ATDT ", false);
    send_modem(nano_ops.host, true);

    if (result_modem(MODE_UNKNOWN) != HAYES_CONNECT) {
        menu_printf("ERROR DIALING! [C-a to continue]");
    } else {
        menu_deactivate(on_update, NULL);
        nano_mode = NANO_MODE_NORMAL;
    }
}

static void m_modem(int c)
{
    switch (c) {
    case CTRL('A'):
        menu_update(on_activation, NULL);
        nano_mode = NANO_MODE_COMMAND;
        break;
    case 'd': /* Dial */ do_dial(); break;
    case 'x': /* hang up */
        send_modem_escape();
        send_modem("ATH", true);
        menu_update(on_activation, NULL);
        nano_mode = NANO_MODE_COMMAND;
        break;
    case 'm': /* drop to modem terminal */
        send_modem_escape();
        menu_deactivate(on_update, NULL);
        nano_mode = NANO_MODE_NORMAL;
        break;
    default: break;
    }
}

static void do_upload(char p)
{
    if (p == 'x') {
        // xmodem upload
    } else if (p == 'y') {
        // ymodem upload
    } else {
        MENU_ENTER(STATUS_TRANS);
    }
}

static void do_download(char p)
{
    if (p == 'x') {
        // xmodem download
    } else if (p == 'y') {
        // ymodem download
    } else {
        MENU_ENTER(STATUS_TRANS);
    }
}

static void m_trans(int c)
{
    switch (c) {
    case CTRL('A'):
        menu_update(on_activation, NULL);
        nano_mode = NANO_MODE_COMMAND;
        break;
    case 'd':
    case 'u': {
        int p;

        MENU_ENTER(STATUS_PROTOCOL);
again:
        p = ak_dev_in(PDEV_STDIN, 0);
        if (p == CTRL('A')) {
            MENU_ENTER(STATUS_TRANS);
        } else if (p == 'y' || p == 'x') {
            if (c == 'u')
                do_upload(p);
            else
                do_download(p);
        } else
            goto again;

        break;
    }
    default: break;
    }
}

static void do_set_speed(void)
{
    int c;

    MENU_ENTER(STATUS_SPEED);
again:
    c = ak_dev_in(PDEV_STDIN, 0);
    if (c == CTRL('A')) {
        MENU_ENTER(STATUS_OPTIONS);
        return;
    } else if (c >= '1' && c <= '7') {
        char s[2];
        nano_ops.baud = c - '0' - 1;
        send_modem_escape();
        send_modem("ATS37=", false);
        s[0] = c - 1;
        s[1] = 0;
        send_modem(s, true);
        send_modem("ATO", true);
        MENU_ENTER(STATUS_OPTIONS);
        return;
    } else
        goto again;
}

static void do_terminal_options(void)
{
    int  c;
    char ss[80];

    while (true) {
        u8    imode = nano_ops.imode;
        u8    omode = nano_ops.omode;
        char  nw    = omode & OUT_NOWRAP ? 'X' : '-';
        char  ns    = omode & OUT_NOSCROLL ? 'X' : '-';
        char  e     = imode & IN_ECHO ? 'X' : '-';
        int   rows  = nano_ops.term_h;
        int   cols  = nano_ops.term_w;
        char *m[2];

        sprintf(ss, STATUS_TERMIOS_B, nw, ns, e, rows, cols);
        m[0] = STATUS_TERMIOS_A;
        m[1] = ss;
        menu_update(on_menu, m);

another:
        c = ak_dev_in(PDEV_STDIN, 0);
        if (c <= 0) goto another;

        switch (c) {
        case CTRL('A'): {
            MENU_ENTER(STATUS_OPTIONS);
            return;
        }
        case '1': {
            if (nw == 'X') {
                nano_ops.omode &= ~OUT_NOWRAP;
            } else {
                nano_ops.omode |= OUT_NOWRAP;
            }
            break;
        }
        case '2': {
            if (nw == 'X') {
                nano_ops.omode &= ~OUT_NOSCROLL;
            } else {
                nano_ops.omode |= OUT_NOSCROLL;
            }
            break;
        }
        case '3': {
            if (nw == 'X') {
                nano_ops.imode &= ~IN_ECHO;
            } else {
                nano_ops.imode |= IN_ECHO;
            }
            break;
        }
        case '4': break; // XXX
        case '5': break;
        default: break;
        }
    }
}

static void m_opt(int c)
{
    switch (c) {
    case CTRL('A'):
        menu_update(on_activation, NULL);
        nano_mode = NANO_MODE_COMMAND;
        break;
    case 's': do_set_speed(); break;
    case 't': do_terminal_options(); break;
    default: break;
    }
}

static void m_help(int c)
{
    switch (c) {
    case CTRL('A'):
        menu_update(on_activation, NULL);
        nano_mode = NANO_MODE_COMMAND;
        break;
    default: break;
    }
}

static bool command(void)
{
    int  c;
    bool quit = false;

    while (!quit && nano_mode != NANO_MODE_NORMAL) {
        c = ak_dev_in(PDEV_STDIN, 0);
        if (c <= 0) continue;
        c = tolower(c);

        switch (nano_mode) {
        case NANO_MODE_COMMAND: m_command(c); break;
        case NANO_MODE_QUIT: quit = m_quit(c); break;
        case NANO_MODE_MODEM: m_modem(c); break;
        case NANO_MODE_TRANSFER: m_trans(c); break;
        case NANO_MODE_OPTIONS: m_opt(c); break;
        case NANO_MODE_HELP: m_help(c); break;
        default: break;
        }
    }

    ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &nano_ops.imode, 1);
    ak_dev_ctl(PDEV_STDOUT, PX_SETCANON, &nano_ops.omode, 1);

    return quit;
}

static void modem(void)
{
    bool    quit = false;
    u8      rx_buf[256];
    u8      tx_buf[17] = { 0 };
    int     tx_ptr     = 0;
    clock_t timer      = clock();
    bool    timeout;

    // clear screen
    ak_dev_ctl(PDEV_STDOUT, PX_WRITE, "\x1b[2J\x1b[H", 7);
    menu_update(on_update, NULL);

    while (!quit) {
        if (nano_mode == NANO_MODE_NORMAL) {
            int n   = ak_dev_ctl(PDEV_STDIN, PX_READ, tx_buf + tx_ptr, 16 - tx_ptr);
            timeout = (clock() - timer) > 2; // 20 ms (CPS is 100)

            if (n > 0) {
                int  old_tx_ptr = tx_ptr, i;
                bool saw_cr = false, saw_c_a = false;
                tx_ptr += n;
                timer = clock();

                for (i = old_tx_ptr; i < tx_ptr; i++) {
                    if (tx_buf[i] == nano_ops.escape) {
                        saw_c_a = true;
                    }
                    if (tx_buf[i] == '\r') saw_cr = true;
                }

                if (saw_c_a) {
                    tx_ptr         = 0;
                    nano_mode      = NANO_MODE_COMMAND;
                    nano_ops.imode = ak_dev_ctl(PDEV_STDIN, PX_GETCANON, NULL, 0);
                    nano_ops.omode = ak_dev_ctl(PDEV_STDOUT, PX_GETCANON, NULL, 0);
                    menu_activate(on_activation, NULL);
                } else if (tx_ptr >= 16 || saw_cr || timeout) {
                    int sent = ak_dev_ctl(DEV_SERIAL, SCTL_WRITE, tx_buf, tx_ptr); // send input
                                                                                   // to serial
                    if (sent != tx_ptr) {
                        fprintf(stderr, "Error sending data: %d\r\n", sent);
                    }
                    memset(tx_buf, 0, 17);
                    tx_ptr = 0;
                }
            }
        } else if (nano_mode == NANO_MODE_COMMAND) {
            quit = command();
        }

        if (serial_available()) {
            // send to console
            int count = ak_dev_in(DEV_SERIAL, TERMINAL_SERIAL_RXCOUNT);
            int res;
            if ((res = ak_dev_ctl(DEV_SERIAL, SCTL_READ, rx_buf, count)) == count) {
                ak_dev_ctl(PDEV_STDOUT, PX_WRITE, rx_buf, count);
                menu_update(on_update, NULL);
            } else {
                menu_printf("Error receiving data: %d\r\n", res);
            }
        }
    }
}

static void error(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    fputs("nanocom: ", stderr);
    vfprintf(stderr, msg, (char *)ap);
    fputs("\r\n", stderr);
    va_end(ap);
    exit(EXIT_FAILURE);
}

static void parse_args(int argc, char **argv)
{
    int i = 1;

    while (i < argc) {
        char *arg = argv[i];

        if (arg[0] != '-') {
            fprintf(stderr, "Unexpected argument: %s\n", arg);
            exit(EXIT_FAILURE);
        }

        switch (arg[1]) {
        case 'b': {
            u32 rate, j;
            if (++i >= argc) error("-b requires a baud rate");
            rate = strtoul(argv[i], NULL, 0);
            for (j = 0; j < sizeof(rates) / sizeof(rates[0]); j++) {
                if (rate == rates[j]) {
                    nano_ops.baud = j;
                    goto next;
                }
            }

            fprintf(stderr, "'%u' is not a supported baud rate choose from:\r\n", rate);
            for (i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
                fprintf(stderr, "\t+ %u\r\n", rates[i]);
            }

            exit(EXIT_FAILURE);
        }

        case 'c': {
            char *hp, *colon;
            if (++i >= argc) error("-c requires host[:port]");
            hp    = argv[i];
            colon = strchr(hp, ':');
            if (colon) {
                *colon        = '\0';
                nano_ops.host = hp;
                nano_ops.port = atoi(colon + 1);
            } else {
                nano_ops.host = hp;
            }
            break;
        }

        case 'e':
            if (++i >= argc) error("-e requires a character");
            if (argv[i][0] == '^' && argv[i][1])
                nano_ops.escape = argv[i][1] & 0x1F;
            else
                nano_ops.escape = argv[i][0];
            break;

        case 'a':
            if (++i >= argc) error("-a requires a command");
            nano_ops.startup_cmd = argv[i];
            break;

        default: fprintf(stderr, "Unknown option: %s\n", arg); exit(EXIT_FAILURE);
        }

next:
        i++;
    }
}

int main(int argc, char **argv)
{
    char ops_at[40];
    int  result_code, res = 0;
    u32  info;

    nano_ops.baud        = 3; // 9600 bps
    nano_ops.host        = NULL;
    nano_ops.port        = 23;
    nano_ops.escape      = CTRL('A');
    nano_ops.startup_cmd = NULL;

    if (!serial_init()) {
        fputs("Could not initialize serial device.\n", stderr);
        return 1;
    }

    atexit(serial_deinit);

    info = ak_dev_ctl(DEV_TEXTBUFFER, TCTL_GET_INFO, NULL, 0);
    if (info < A_OK) {
        nano_ops.term_w = 80;
        nano_ops.term_h = 25;
    } else {
        nano_ops.term_w = info >> 8;
        nano_ops.term_h = info & 0xFF;
    }

    if (!menu_init(nano_ops.term_w, nano_ops.term_h, 4 /*ANSI BLUE*/)) return 100;
    atexit(menu_deinit);

    if (argc < 2) {
        modem();
        return 0;
    }

    parse_args(argc, argv);

    memset(ops_at, 0, 32);
    // set the baud rate, numeric mode, Xmodem 2, echo on,
    // also set CR to 13 and LF to 10
    sprintf(ops_at, "ATs3=13s4=10s37=%dV0X2E1", nano_ops.baud);
    send_modem(ops_at, true);
    result_code = result_modem(MODE_UNKNOWN);
    if (result_code != HAYES_OK) {
        fprintf(stderr, "Unexpected result: %d\r\n", result_code);
        return EXIT_FAILURE;
    }

    if (nano_ops.startup_cmd) {
        send_modem(nano_ops.startup_cmd, true);
        result_code = result_modem(MODE_NUMERIC);
        if (result_code < 0) {
            fprintf(stderr, "Unexpected result: %d\r\n", result_code);
            return EXIT_FAILURE;
        } else if (result_code != HAYES_OK) {
            menu_printf("\x1b[41m[]", results_verbose[result_code]);
        }
    }

    if (result_code != HAYES_CONNECT && nano_ops.host) {
        sprintf(ops_at, "ATDT %s:%d", nano_ops.host, nano_ops.port);
        send_modem(ops_at, true);
        result_code = result_modem(MODE_NUMERIC);
        if (result_code != HAYES_CONNECT) {
            fprintf(stderr, "Could not connect to: %s:%d (%d)\r\n", nano_ops.host, nano_ops.port,
                    result_code);
            return EXIT_FAILURE;
        }
    }

    modem();
    return 0;
}
