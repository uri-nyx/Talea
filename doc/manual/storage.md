# Persistent Storage

Persistent storage in the Taleä Computer System comes in two flavors: *Tiny Portable Storage* (TPS), and regular disk storage. TPS is capable of storaging up to 128 Kib of data, and the system has two drives for it. The disk storage is much larger at 128 Mib, but it is not removable nor portable.

## The TPS Drives

The TPS drives are controlled through this registers:

    ╭───────┬─────╮
    │COMMAND│ 0x00│
    ├───────┼─────┤
    │DATA   │ 0x01│
    ├───────┼─────┤
    │POINTH │ 0x02│
    ├───────┼─────┤
    │POINTL │ 0x03│
    ├───────┼─────┤
    │STATUSH│ 0x04│
    ├───────┼─────┤
    │STATUSL│ 0x05│
    ╰───────┴─────╯

The commands available to address the TPS drive are the following:

- (`0x00`) Nop: does nothing.
- (`0x04`) Is Bootable: returns `1` in `STATUSL` if the selected drive is bootable.
- (`0x05`) Is Present: returns `1` in `STATUSL` if the selected drive is present.
- (`0x06`) Open: opens the selected drive.
- (`0x07`) Close: closes the selected drive.
- (`0x01`) Store Sector: stores the sector specified in `pointh-pointl` into the sector `data`.
- (`0x02`) Load Sector: stores the sector specified in `data` into `pointh-pointl`.

The command must designe in the high order byte if the drive `0` or `1` is addressed.

## The Disk Drive

The Disk drive is controlled through this registers:

    ╭───────┬─────╮
    │COMMAND│ 0x00│
    ├───────┼─────┤
    │DATA   │ 0x01│
    ├───────┼─────┤
    │SECTORH│ 0x02│
    ├───────┼─────┤
    │SECTORL│ 0x03│
    ├───────┼─────┤
    │POINTH │ 0x04│
    ├───────┼─────┤
    │POINTL │ 0x05│
    ├───────┼─────┤
    │STATUS0│ 0x06│
    ├───────┼─────┤
    │STATUS1│ 0x07│
    ╰───────┴─────╯

The commands available to address the Disk Drive are the following:

- (`0x00`) Nop: does nothing.
- (`0x01`) Store Sector: stores the sector at `point` into `sector` at `data` drive.
- (`0x02`) Load Sector: loads the sector at `sector` at `data` into `point`.
