        .const *printer-out $0200
        .const *printer-options $0201
        .const *teletype-in $0100
        .const *teletype-status $0101  
        .org #x0300
check   ldr acc *teletype-status
        bnz putc
        bez check
putc    ldr bcc *teletype-in
        bez check
        str bcc *printer-out
        str acc *printer-options
        str r1  *printer-options
        str r1  *teletype-status
        bnz check
endprog .end