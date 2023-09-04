# The Keyboard

The keyboard is simple, and can be configured either to trigger an interrupt on the reception of scancodes or characters. The registers are:

    ╭─────────┬────╮
    │MODIFIERS│0x00│
    ├─────────┼────┤
    │CHARACTER│0x01│
    ├─────────┼────┤
    │SCANCODE │0x02│
    ├─────────┼────┤
    │KBDMODE  │0x00│
    ╰─────────┴────╯

The modes are `0` for `character mode` (fire interrupt on character reception) and `1` for `scancode mode` (fire interrupt on scancode reception). The `scancode` register is 2 bytes long. The `modifiers` register exposes the modifiers pressed in any given moment in the keyboard: `shift`, `control`, `alt` or `logo`:

    ╭──────────┬───────┬──────┬─────┬──────╮
    │reserved:4│shift:1│ctrl:1│alt:1│logo:1│
    ╰──────────┴───────┴──────┴─────┴──────╯
