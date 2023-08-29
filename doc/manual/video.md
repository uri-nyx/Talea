# The Video System

The video system is the most complex module of the machine following Sirius. Its main task is displayiing images and performing graphic oriented operations (COMING SOON). Its control registers are the following:

    ╭────────┬────┬────╮
    │COMMAND │byte│ 0x0│
    ├────────┼────┼────┤
    │DATAH   │byte│ 0x1│
    ├────────┼────┼────┤
    │DATAM   │byte│ 0x2│
    ├────────┼────┼────┤
    │DATAL   │byte│ 0x3│
    ├────────┼────┼────┤
    │GPU0    │byte│ 0x4│
    ├────────┼────┼────┤
    │GPU1    │byte│ 0x5│
    ├────────┼────┼────┤
    │GPU2    │byte│ 0x6│
    ├────────┼────┼────┤
    │GPU3    │byte│ 0x7│
    ├────────┼────┼────┤
    │GPU4    │byte│ 0x8│
    ├────────┼────┼────┤
    │GPU5    │byte│ 0x9│
    ├────────┼────┼────┤
    │GPU6    │byte│ 0xa│
    ├────────┼────┼────┤
    │GPU7    │byte│ 0xb│
    ├────────┼────┼────┤
    │STATUS0 │byte│ 0xc│
    ├────────┼────┼────┤
    │STATUS1 │byte│ 0xd│
    ├────────┼────┼────┤
    │STATUS2 │byte│ 0xe│
    ├────────┼────┼────┤
    │STATUS3 │byte│ 0xf│
    ╰────────┴────┴────╯

## Video Modes

The video system supports three basic modes:

### *Monocrome basic TEXT*

Number `0`, monochrome text mode of 80*25 characters. The characters are encoded as CP437 characters. 

### *Rich TEXT*

Number `1`,  a 16 color text mode of 160*50 characters. The characters are encoded as CP437 character code and a byte of attributes as follows.

    ╭────────┬──────────────┬──────────────╮
    │blink: 1│ background: 3│ foreground: 4│
    ╰────────┴──────────────┴──────────────╯

This adds up to a total of 2 bytes per character. The colors are indexed into a configurable pallete (NOT BY NOW)

### *GRAPHIC*

Number `2` , a graphic 256 color mode of 640*480 pixels. Pixels are represented a byte per pixel, encoding as rgb332 values.

## Basic commands

A set of basic commands manage primary functions of the video controller, such as setting modes, fonts, and sending actual data to the screen. A command is issued by writing its number to it and setting the correspondent arguments in the data register.

- (`0x0`) Nop: do nothing.
- (`0x1`) Clear: clear the framebuffer.
- (`0x2`) Set Mode (dh: mode): sets the requested mode.
- (`0x4`) Set Font (dh: font): sets the requested hardware font (if it does not exist, default to 0).
- (`0x6`) Blit (dh-dm-dl): blits the framebuffer with the contents of the buffer at address dh-dm-dl.

## Screen Refresh

The screen refreshes 60 times a second (60hz), and issues a *SCREEN_REFRESH* interrupt to the cpu at priority level 6.
