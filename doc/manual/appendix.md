# Appendixes

## A

This is the list of registers for the Taleä Computer System, and a proposed ABI (identical to the RISC-V IA32 ISA):

|  5-bit Encoding (rx)  |  Register  |  ABI Name  |  Description                           |  Saved by Calle-
 |
|-----------------------|------------|------------|----------------------------------------|-------------------|
|  0                    |  x0        |  zero      |  hardwired zero                        | Not saved
        |
|  1                    |  x1        |  ra        |  return address                        | Caller
           |
|  2                    |  x2        |  sp        |  stack pointer                         | Callee
           |
|  3                    |  x3        |  gp        |  global pointer                        | Not saved
        |
|  4                    |  x4        |  tp        |  thread pointer                        | Not saved
        |
|  5                    |  x5        |  t0        |  temporary register 0                  | Caller
           |
|  6                    |  x6        |  t1        |  temporary register 1                  | Caller
           |
|  7                    |  x7        |  t2        |  temporary register 2                  | Caller
           |
|  8                    |  x8        |  s0 / fp   |  saved register 0 / frame pointer      | Callee
           |
|  9                    |  x9        |  s1        |  saved register 1                      | Callee
           |
|  10                   |  x10       |  a0        |  function argument 0 / return value 0  | Caller
           |
|  11                   |  x11       |  a1        |  function argument 1 / return value 1  | Caller
           |
|  12                   |  x12       |  a2        |  function argument 2                   | Caller
           |
|  13                   |  x13       |  a3        |  function argument 3                   | Caller
           |
|  14                   |  x14       |  a4        |  function argument 4                   | Caller
           |
|  15                   |  x15       |  a5        |  function argument 5                   | Caller
           |
|  16                   |  x16       |  a6        |  function argument 6                   | Caller
           |
|  17                   |  x17       |  a7        |  function argument 7                   | Caller
           |
|  18                   |  x18       |  s2        |  saved register 2                      | Callee
           |
|  19                   |  x19       |  s3        |  saved register 3                      | Callee
           |
|  20                   |  x20       |  s4        |  saved register 4                      | Callee
           |
|  21                   |  x21       |  s5        |  saved register 5                      | Callee
           |
|  22                   |  x22       |  s6        |  saved register 6                      | Callee
           |
|  23                   |  x23       |  s7        |  saved register 7                      | Callee
           |
|  24                   |  x24       |  s8        |  saved register 8                      | Callee
           |
|  25                   |  x25       |  s9        |  saved register 9                      | Callee
           |
|  26                   |  x26       |  s10       |  saved register 10                     | Callee
           |
|  27                   |  x27       |  s11       |  saved register 11                     | Callee
           |
|  28                   |  x28       |  t3        |  temporary register 3                  | Caller
           |
|  29                   |  x29       |  t4        |  temporary register 4                  | Caller
           |
|  30                   |  x30       |  t5        |  temporary register 5                  | Caller
           |
|  31                   |  x31       |  t6        |  temporary register 6                  | Caller
           |
