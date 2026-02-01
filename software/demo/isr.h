#ifndef ISR_H
#define ISR_H

#include "libsirius/types.h"

void bios_syscall_handler(u32 x12, u32 x13, u32 x14, u32 x15, u32 x16, u32 x17);
void bios_reset(void);
void bios_abort(u32 reason);
void bios_kbd_handler(void);
void bios_timeout_handler(void);
void bios_interval_handler(void);
void bios_vblank_handler(void);
void bios_pointer_pressed(void);
void bios_tps_handler(u32 event);
void bios_music_note_end(void);

struct interrupts_triggered {
        u8 reset;
        u8 abort;
        u8 character;
        u8 scancode;
        u8 timeout;
        u8 interval;
        u8 vblank;
        u8 pointer;
        u8 tps;
        u8 music_note_end;
};

typedef void (*interrupt_handler_t)(u32);

#endif /* ISR_H */