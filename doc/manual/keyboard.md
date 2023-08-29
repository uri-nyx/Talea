# The Keyboard

The keyboard is simple, and can be configured either to trigger an interrupt on the reception of scancodes or characters. The registers are:

    ╭─────────┬────╮
    │CHARACTER│0x00│
    ├─────────┼────┤
    │CODE     │0x01│
    ├─────────┼────┤
    │MODIFIERS│0x02│
    ├─────────┼────┤
    │KBDMODE  │0x03│
    ╰─────────┴────╯

The modes are `0` for `character mode` and `1` for `scancode mode`. The `modifiers` register exposes the modifiers pressed in any given moment in the keyboard: `shift`, `control`, `alt` or `logo`:

    ╭──────────┬───────┬──────┬─────┬──────╮
    │reserved:4│shift:1│ctrl:1│alt:1│logo:1│
    ╰──────────┴───────┴──────┴─────┴──────╯
