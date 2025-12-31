#include "ushell.h"

#define ASCII_BACKSPACE 0x08
#define ASCII_DEL       0x7f

static struct {
    char rx[USHELL_RX_BUFFER_SIZE];
    int  rx_ptr;

    int   argc;
    char *argv[USHELL_ARGC_MAX];

    const ushell_command_t *commands;
    usize                   commands_cnt;
} ushell;

static int   ushell_strcmp(const char *lhs, const char *rhs);
static usize ushell_strlen(const char *str);
static void  ushell_print(const char *str);
static void  ushell_println(const char *str);
static void  ushell_print_help(void);
static void  ushell_receive(char c);
static void  ushell_parse(void);
static void  ushell_exec(void);

static int ushell_strcmp(const char *lhs, const char *rhs)
{
    while (*lhs && (*lhs == *rhs)) ++lhs, ++rhs;
    return *(const unsigned char *)lhs - *(const unsigned char *)rhs;
}

static usize ushell_strlen(const char *str)
{
    const char *s;
    for (s = str; *s; ++s);
    return (s - str);
}

void ushell_init(const ushell_command_t *commands, usize commands_cnt)
{
    i32 retcode;

    ushell.commands     = commands;
    ushell.commands_cnt = commands_cnt;
    ushell.argc         = 0;
    ushell.rx_ptr       = 0;
    memset(ushell.rx, 0, USHELL_RX_BUFFER_SIZE);

    ushell_print(USHELL_PROMPT);
}

static void ushell_print(const char *str)
{
    unsigned long len = ushell_strlen(str);
    unsigned long i;
    for (i = 0; i < len; ++i) ushell_putchar(str[i]);
}

static void ushell_println(const char *str)
{
    ushell_print(str);
    ushell_print(EOL);
}

static void ushell_print_help(void)
{
    usize i;
    ushell_println("ushell: commands available:");
    for (i = 0; i < ushell.commands_cnt; ++i) {
        ushell_print("\t");
        ushell_print(ushell.commands[i].name);
        ushell_print(" - ");
        ushell_println(ushell.commands[i].description);
    }
    ushell_println("\thelp - list all commands");
}

static void ushell_receive(char chr)
{
    if (ushell.rx_ptr >= USHELL_RX_BUFFER_SIZE) {
        ushell.rx_ptr = 0;
        ushell_println("ushell: command is too long");

        return;
    }

    switch (chr) {
    case '\r':
    case '\n':
        if (ushell.rx_ptr > 0) {
            ushell_print(EOL);
            if (ushell.rx[ushell.rx_ptr - 1] == ' ')
                ushell.rx[ushell.rx_ptr - 1] = '\0';
            ushell.rx[ushell.rx_ptr++] = '\0';
        }
        break;

    case ASCII_BACKSPACE:
    case ASCII_DEL:
        if (ushell.rx_ptr > 0) {
            --ushell.rx_ptr;
            ushell_print("\b \b");
        }
        break;

    case ' ':
        if (ushell.rx_ptr > 0 && ushell.rx[ushell.rx_ptr - 1] != ' ') {
            ushell.rx[ushell.rx_ptr++] = chr;
            ushell_putchar(chr);
        }
        break;

    default:
        ushell.rx[ushell.rx_ptr++] = chr;
        ushell_putchar(chr);
        break;
    }
}

static void ushell_parse(void)
{
    int           i;
    unsigned long arg_pos;

    if (ushell.rx_ptr == 0) return;

    if ((ushell.rx_ptr > 0) && (ushell.rx[ushell.rx_ptr - 1] != '\0')) return;

    arg_pos = 0;
    for (i = 0; i < ushell.rx_ptr; ++i) {
        if (ushell.rx[i] == ' ') {
            if (ushell.argc >= USHELL_ARGC_MAX) {
                ushell_println("ushell: argc > ARGC_MAX");

                return;
            }

            ushell.rx[i]               = '\0';
            ushell.argv[ushell.argc++] = ushell.rx + arg_pos;
            arg_pos                    = ++i;
        }
    }
    ushell.argv[ushell.argc++] = ushell.rx + arg_pos;
}

static void ushell_exec(void)
{
    usize i;
    int   res;
    int   command_found = 0;

    if (ushell.argc == 0) return;

    for (i = 0; i < ushell.commands_cnt; ++i) {
        if (!ushell_strcmp(ushell.commands[i].name, ushell.argv[0])) {
            command_found = 1;
            ushell.commands[i].func(ushell.argc, ushell.argv);
            /* TODO: figure out why returning from a syscall messes
             * up the stack*/
            goto end;
        }
    }
    if (!ushell_strcmp("help", ushell.argv[0])) {
        command_found = 1;
        ushell_print_help();
    }

    if (!command_found) {
        ushell_println("ushell: command not found, use \"help\"");
    }

end:
    ushell_print(USHELL_PROMPT);

    ushell.rx_ptr = 0;
    ushell.argc   = 0;
}

void ushell_process(char chr)
{
    ushell_receive(chr);
    ushell_parse();
    ushell_exec();
}
