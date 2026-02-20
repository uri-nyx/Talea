CPPFLAGS := -I$(SDK)
LIBS := -l $(NAIMEN)/libc/libc.a

all: sh.c
	$(CC) $(CPPFLAGS) -c $(CRT0) sh.c
	$(LD) $(LIBS) $(LDFLAGS) -o sh crt0.o sh.o
	cp sh ../tps_a

clean:
	rm crt0.o sh.o sh
