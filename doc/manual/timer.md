# The Timer Module

The timer module is a simple device that provides the possibility to time certain events of the system. It exposes the following registers:

    ╭─────────┬────╮
    │TIMEOUT  │0x00│
    ├─────────┼────┤
    │INTERVAL │0x02│
    ├─────────┼────┤
    │FREQ T   │0x04│
    ├─────────┼────┤
    │FREQ I   │0x05│
    ╰─────────┴────╯

Writing to the `TIMEOUT` register will immediately enable it, and it will count down from the value set until it reaches zero. Then, it will fire an interrupt at priority level 6, and disable itself.

Writing to the `INTERVAL` register will toggle it, and it will fire an interrupt at priority level 6, and disable itself every time the amount of time specified has passed.

The timer will count based every `FREQ` cycles, running at 10Mhz. Setting it to zero will disable the timer.
