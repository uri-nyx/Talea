    .org #q300
hello   .stringz "hello
    lea hello
    ldr bcc #q2
putc    ldi acc
    bez endprog
    str acc $0200
    str bcc $0201
    ldr acc #q1
    add lx
    ldr lx acc
    bnz putc
endprog nop