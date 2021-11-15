        .org #x0300
check   ldr acc $0101
        bnz putc
        bez check
putc    ldr bcc $0100
        bez endprog
        str acc $0200
        str bcc $0201
        str r1  $0201
        bnz check
endprog .end