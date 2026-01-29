#include "terminal.h"

#include "talea.h"

/*----------------------------------------------------------------------------*/
/* STATIC GLOBALS                                                             */
/*----------------------------------------------------------------------------*/

// Put statics here

/*----------------------------------------------------------------------------*/
/* INTERNAL DEVICE FUNCIONS: declare as static inline                         */
/*----------------------------------------------------------------------------*/

// Put static inline functions here

/*----------------------------------------------------------------------------*/
/* INTERFACE IMPLEMENTATION                                                   */
/*----------------------------------------------------------------------------*/

// Callbacks for Raylib's AudioStream

// DEVICE INITIALIZATION

// DEVICE UPDATE (on vblank)

// DEVICE UPDATE (on cpu tick)

void Timer_Update(TaleaMachine *m, u32 cycles)
{
    TerminalTimer *t = &m->terminal.timer;

    if (!(t->csr & TERMINAL_TIM_GLOBAL_EN) || t->prescaler == 0) return;

    t->accumI += cycles;
    while (t->accumI >= t->prescaler) {
        t->accumI -= t->prescaler;
        if (t->csr & TERMINAL_TIM_INTERVAL_EN) {
            if (t->intervalCounter > 0) {
                t->intervalCounter--;
            } else {
                t->intervalCounter = t->intervalReload;
                Machine_RaiseInterrupt(m, INT_TIMER_INTERVAL, PRIORITY_INTERVAL_INTERRUPT);
            }
        }
    }

    // TODO: DOCUMENT (Fixed x256 multiplier)
    t->accumT += cycles;
    // We treat 'prescaler' as the high-byte of a 16-bit prescaler here
    u32 timeout_threshold = (u32)t->prescaler << 8;

    while (t->accumT >= timeout_threshold) {
        t->accumT -= timeout_threshold;
        if (t->csr & TERMINAL_TIM_TIMEOUT_EN) {
            if (t->timeoutCounter > 0) {
                t->timeoutCounter--;
                if (t->timeoutCounter == 0) {
                    t->csr &= ~TERMINAL_TIM_TIMEOUT_EN; // Auto-stop
                    Machine_RaiseInterrupt(m, INT_TIMER_TIMEOUT, PRIORITY_INTERVAL_INTERRUPT);
                }
            }
        }
    }
}

// DEVICE PORT IO HANDLERS
