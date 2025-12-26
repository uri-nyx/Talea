#include "core/bus.h"
#include "talea.h"

#define P_MOUSE_STATE (DEV_MOUSE_BASE + 0)
#define P_MOUSE_X     (DEV_MOUSE_BASE + 1)
#define P_MOUSE_Y     (DEV_MOUSE_BASE + 3)

void Mouse_ProcessButtonPress(TaleaMachine *m, int buttons, int scaled_x,
                              int scaled_y)
{
    Mouse_UpdateCoordinates(m, buttons, scaled_x, scaled_y);
    Machine_RaiseInterrupt(m, INT_MOUSE_PRESSED, PRIORITY_KEYBOARD_INTERRUPT);
}

void Mouse_UpdateCoordinates(TaleaMachine *m, int buttons, int scaled_x,
                             int scaled_y)
{
    m->data_memory[P_MOUSE_STATE] = buttons;
    m->data_memory[P_MOUSE_X]     = scaled_x >> 8;
    m->data_memory[P_MOUSE_X + 1] = scaled_x;
    m->data_memory[P_MOUSE_Y]     = scaled_y >> 8;
    m->data_memory[P_MOUSE_Y + 1] = scaled_y;
}