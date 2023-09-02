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
- (`0x01`) Store Sector: stores the sector specified in `pointh-pointl` into the sector `data`.
- (`0x02`) Load Sector: stores the sector specified in `data` into `pointh-pointl`.
- (`0x03`) Is Bootable: returns `1` in `STATUSL` if the selected drive is bootable.
- (`0x04`) Is Present: returns `1` in `STATUSL` if the selected drive is present.
- (`0x05`) Open: opens the selected drive.
- (`0x06`) Close: closes the selected drive.
- (`0x07`) Set Current: selects the drive indicated by `DATA` (0 for A, 1 for B).
- (`0x08`) Size: returns number of sectors - 1 in `STATUSH` and size of sector in `STATUSL` (as a multiple of 256).

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
- (`0x01`) Store Sector: stores the sector at `point` into `sector` at `data` disk.
- (`0x02`) Load Sector: loads the sector at `sector` at `data` disk into `point`.
- (`0x03`) Is Bootable: returns `1` in `STATUS1` if the disk at `data` is bootable.
- (`0x04`) Is Present: returns `1` in `STATUS1` if the disk at `data` is present.
- (`0x08`) Size: returns the number of disks in `STATUS0` and size of sector in `STATUS1` as a multiple of 256, it is assumed that there are 65536 sectors per disk.
