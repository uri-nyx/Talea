CPPFLAGS := -I$(SDK) -I$(NAIMEN)/libc/include
LIBS := -l $(NAIMEN)/libc/libcmin.a

all: sh

sh: sh.c
	$(CC) $(CPPFLAGS) -c $(CRT0) sh.c
	$(LD) $(LIBS) $(LDFLAGS) -o sh crt0.o sh.o
	cp sh ../tps_a/bin
	cp autoexec.ash ../tps_a/

clean:
	rm crt0.o sh.o sh
