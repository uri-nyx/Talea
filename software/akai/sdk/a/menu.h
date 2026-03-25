#ifndef MENU_H
#define MENU_H

#include <akai.h>
#include <stdarg.h>

#include <stdio.h>

/**
 * Implements a simple standard layout for basic full screen terminal
 * applications in akai (more, nanocom, man, etc...):
 *
 * The main view is a panel of text, and the top has a status bar.
 * C-a (or other hotkey) switches to "command" mode, and the status bar changes color to ACTIVE
 * and optionally expands up to 5 rows (NOT IMPLEMENTED). Stdout goes to the panel, and stdin is
 * intercepted.
 *
 * Example:
 * (80x30 characters)
 *
 */

/*----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
 * 01 STATUS LINE
 * 02
 * 03
 * 04
 * 05
 * 06
 * 07
 * 08
 * 09
 * 10
 * 11
 * 12
 * 13
 * 14
 * 15
 * 16
 * 17
 * 18
 * 19
 * 20
 * 21
 * 22
 * 23
 * 24
 * 25
 * 26
 * 27
 * 28
 * 29
 * 30
 **/

#define LAST_LINE   (m.h - 1)
#define MENU_STATUS (0)

struct AkaiMenu {
    u8        w, h;
    bool      active;
    sgl_color bg;

    u8  old_imode, old_omode;
    u32 old_margins;
};

bool menu_init(u8 term_w, u8 term_h, sgl_color bg);
void menu_deinit(void);

void menu_activate(void (*on_activation)(void *), void *user);
void menu_deactivate(void (*on_deactivation)(void *), void *user);
void menu_update(void (*on_update)(void *), void *user);
void menu_printf(const char *fmt, ...);

/* IMPLEMENTATION */
#ifdef MENU_IMPLEMENTATION

static struct AkaiMenu m;

static int isatty(unsigned int proxy)
{
    int res = ak_dev_ctl(proxy, PX_GET_DEV, NULL, 0);
    if (res < A_OK) return 0;

    return (res == (PDEV_TYPE_HW | DEV_TEXTBUFFER)) || (res == (PDEV_TYPE_HW | DEV_SERIAL));
}

static void clear_lines(usize start, usize count)
{
    usize i;
    count = (start + count) > m.h ? m.h : count;

    for (i = 0; i < count; i++) printf("\x1b[%d;1H\x1b[2K", start + i + 1);
}

static void set_margins(void)
{
    u8 margins[4];
    m.old_margins = ak_dev_ctl(PDEV_STDOUT, PX_GET_MARGINS, NULL, 0);
    memset(margins, 0xFF, sizeof(margins));
    margins[T_MARGIN_TOP] = 1;

    ak_dev_ctl(PDEV_STDOUT, PX_SET_MARGINS, &margins, sizeof(margins));
}

static void reset_margins(void)
{
    u8 margins[4];
    margins[T_MARGIN_TOP]   = m.old_margins >> 24;
    margins[T_MARGIN_BOT]   = m.old_margins >> 16;
    margins[T_MARGIN_LEFT]  = m.old_margins >> 8;
    margins[T_MARGIN_RIGTH] = m.old_margins;

    ak_dev_ctl(PDEV_STDOUT, PX_SET_MARGINS, &margins, sizeof(margins));
}

bool menu_init(u8 term_w, u8 term_h, sgl_color bg)
{
    memset(&m, 0, sizeof(m));

    if (!isatty(PDEV_STDOUT)) return false;
    m.old_imode = ak_dev_ctl(PDEV_STDIN, PX_GETCANON, NULL, 0);
    m.old_omode = ak_dev_ctl(PDEV_STDOUT, PX_GETCANON, NULL, 0);

    if (term_h < 2) return false;

    set_margins();
    m.w      = term_w;
    m.h      = term_h;
    m.active = false;

    m.bg = bg > 7 ? 2 : bg;

    return true;
}

void menu_deinit(void)
{
    reset_margins();
    printf("\x1b[49m\x1b[0m\x1b[2J");
    fflush(stdout);
    ak_dev_ctl(PDEV_STDOUT, PX_SETCANON, &m.old_omode, 1);
    ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &m.old_imode, 1);
}

void menu_activate(void (*on_activation)(void *), void *user)
{
    u8 timode, nimode;
    if (m.active) return;

    m.active = true;

    reset_margins();
    timode = ak_dev_ctl(PDEV_STDIN, PX_GETCANON, NULL, 0);
    nimode |= OUT_NOSCROLL | OUT_NOWRAP;
    ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &nimode, 1);

    printf("\x1b[s\x1b[H\x1b[4%dm\x1b[2K", m.bg);
    if (on_activation) on_activation(user);
    fflush(stdout);
    ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &timode, 1);
    set_margins();
}

void menu_update(void (*on_update)(void *), void *user)
{
    u8 timode, nimode;

    reset_margins();
    timode = ak_dev_ctl(PDEV_STDIN, PX_GETCANON, NULL, 0);
    nimode |= OUT_NOSCROLL | OUT_NOWRAP;
    ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &nimode, 1);

    if (!m.active)
        fputs("\x1b[s\x1b[H\x1b[7m\x1b[2K", stdout);
    else
        fputs("\x1b[H\x1b[2K", stdout);

    if (on_update) on_update(user);
    ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &timode, 1);

    if (!m.active) printf("\x1b[27m\x1b[u");
    fflush(stdout);

    set_margins();
}

void menu_deactivate(void (*on_deactivation)(void *), void *user)
{
    u8 timode, nimode;

    reset_margins();
    timode = ak_dev_ctl(PDEV_STDIN, PX_GETCANON, NULL, 0);
    nimode |= OUT_NOSCROLL | OUT_NOWRAP;
    ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &nimode, 1);

    if (!m.active) return;

    fputs("\x1b[H\x1b[49m\x1b[7m\x1b[2K", stdout);
    if (on_deactivation) on_deactivation(user);
    fputs("\x1b[u", stdout);

    m.active = false;
    fflush(stdout);
    ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &timode, 1);
    set_margins();
}

void menu_printf(const char *fmt, ...)
{
    va_list ap;
    u8      timode, nimode;

    reset_margins();
    timode = ak_dev_ctl(PDEV_STDIN, PX_GETCANON, NULL, 0);
    nimode |= OUT_NOSCROLL | OUT_NOWRAP;
    ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &nimode, 1);

    va_start(ap, fmt);

    if (!m.active)
        fputs("\x1b[s\x1b[H\x1b[7m\x1b[2K", stdout);
    else
        fputs("\x1b[H\x1b[2K", stdout);
    vprintf(fmt, (char *)ap);
    if (!m.active) fputs("\x1b[27m\x1b[u", stdout);

    va_end(ap);
    fflush(stdout);
    ak_dev_ctl(PDEV_STDIN, PX_SETCANON, &timode, 1);
    set_margins();
}

#endif

#endif /* MENU_H */
