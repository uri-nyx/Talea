# The Serial Port

The *serial port* is the simplest way to interface with the *Taleä Computer System*. It receives data over a serial connection.

## PORTS

Note that the transmit (TX) and receive (RX) ports are labeled here from the CPU's point of view (if we took the serial controler side, they would be reversed).

    ╭──────┬──────────┬─────╮
    │RX    │ halfword │ 0x00│
    ├──────┼──────────┼─────┤
    │TX    │ halfword │ 0x02│ <- Notice, only a byte is used
    ├──────┼──────────┼─────┤
    │STATUS│ byte     │ 0x04│
    ├──────┼──────────┼─────┤
    │CTRL  │ byte     │ 0x05│
    ╰──────┴──────────┴─────╯

## Sending a byte to the Serial Port

Sending a byte to the port is easy, one just needs to write it to the TX port.

## Receiving data from the Serial Port

An interrupt will be fired when there is data incoming from the serial device at *priority level* 4. Reading from `RX[0]` will transfer a byte to the system, whereas reading from `RX[1]` will return the number of remaining incoming bytes. Notice that subsequent reads from `RX[0]` will return the bytes of the input in a LIFO/FIFO fashion, until the buffer is exhausted, when it will read `0x00`.

## Controlling the Serial Port

Writing `0x00` to the `CTRL` register will swicht the port to a FIFO for readings. The default (`0x01`) is LIFO. If the buffer contained data before the swicht took place, no warranties are made about the rearrangement of the data, its loss, or its corruption, and reading from a port in that state can lead to *undefined behaviour*.
