#include "terminal.h"

#include "talea.h"

/*----------------------------------------------------------------------------*/
/* STATIC GLOBALS                                                             */
/*----------------------------------------------------------------------------*/

// Put statics here

/*----------------------------------------------------------------------------*/
/* INTERNAL DEVICE FUNCIONS: declare as static inline                         */
/*----------------------------------------------------------------------------*/

void Keyboard_PushEvent(TerminalKeyboard *k, u16 scan, u8 chr, u8 mod, bool isDown)
{
    u8 next_head = (k->head + 1) % TERMINAL_KB_QUEUE_LEN;
    if (next_head == k->tail) {
        TALEA_LOG_TRACE("Pushing to keyboard hw queue failed: queue full\n");
        return;
    }

    k->queue[k->head].scancode  = scan;
    k->queue[k->head].character = chr;
    k->queue[k->head].modifiers = mod;
    k->queue[k->head].isDown    = isDown;

    k->head = next_head;
}

void Keyboard_PopEvent(TerminalKeyboard *k)
{
    if (k->head != k->tail) {
        k->tail = (k->tail + 1) % TERMINAL_KB_QUEUE_LEN;
    }
}

// Returns null on empty queue
TerminalKbdEvent *Keyboard_PeekEvent(TerminalKeyboard *k)
{
    if (k->head == k->tail) {
        static TerminalKbdEvent null_ev = { 0, 0, false };
        return &null_ev;
    }
    return &k->queue[k->tail];
}

/*----------------------------------------------------------------------------*/
/* INTERFACE IMPLEMENTATION                                                   */
/*----------------------------------------------------------------------------*/

// Callbacks for frontend

void Keyboard_ProcessKeypress(TaleaMachine *m, bool is_down, int key, u8 chr, u16 mod)
{
    u8   csr  = m->terminal.kb.csr;
    bool fire = false;

    TALEA_LOG_TRACE("Polling keyboard, csr %0x \n", csr);
    if (csr & TERMINAL_KB_GLOBAL_EN) {
        bool want_down   = (csr & TERMINAL_KB_IE_DOWN) && is_down;
        bool want_up     = (csr & TERMINAL_KB_IE_UP) && !is_down;
        bool char_filter = (csr & TERMINAL_KB_IE_CHAR);
        TALEA_LOG_TRACE("Polling keyboard, want down %d, want up %d\n", want_down, want_up);

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

// DEVICE INITIALIZATION

// DEVICE UPDATE (on vblank)

// DEVICE UPDATE (on cpu tick)

// DEVICE PORT IO HANDLERS
