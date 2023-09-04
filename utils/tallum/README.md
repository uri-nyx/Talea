# TALLUM: Taleä Language Layer for Universal Machines

TALLUM is a backend and abstract VM architecture aimed to the Taleä Computer system, but machine agnostic in nature.

It specifies and implements a simple stack machine based on the one presented in Nand2Tetris, and expands it. Currently, the only frontends compatible with this implementation are compilers for the JACK programming language.

## Optimizations

For simple virtual machine code, as dezcribed in Nand2Tetris, TALLUM can perform a series of optimizations both in at the virtual machine code level and then target specific optimizations are available.

### Machine independent

- Constant folding, and passing constants as innmediate in the operations (if the target allows it)
- Segment to segment transfer: `push segment n; pop segment m -> move segment n segment m`.
- Top of stack copy: `pop segment n; push segment n -> dup segment n`

### Machine dependent

- Removal of `push reg, sp; pop reg, sp` redundancies.
