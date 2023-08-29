; Headers & drivers for the video device in the Taleä System
#once
#include "headers.asm"

; Subroutines
__video:
    .nop:
        __asm_inline_video_nop
        ret

    .clear:
        __asm_inline_video_clear
        ret

    .clearregs:
        __asm_inline_video_clearregs
        ret

    .blit:
        __asm_inline_video_blit
        ret

    .set:
        ..fb:
            __asm_inline_video_setfb
            ret
        ..mode:
            __asm_inline_video_setmode
            ret
        ..vblank:
            __asm_inline_video_setvblank
            ret
        ..bg:
            __asm_inline_video_setbg
            ret
        ..fg:
            __asm_inline_video_setfg
            ret
        ..font:
            __asm_inline_video_setfont
            ret

    .load:
        ..font:
            __asm_inline_video_loadfont
            ret
        ..palette:
            __asm_inline_video_loadpalette
            ret
