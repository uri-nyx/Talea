CPPFLAGS := -I$(SDK) -I$(NAIMEN)/libc/include
LIBS := -l $(NAIMEN)/libc/libcmin.a

all: more

more: more.c
	$(CC) $(CPPFLAGS) -c $(CRT0) more.c
	$(LD) $(LIBS) $(LDFLAGS) -o more crt0.o more.o
	cp more ../tps_a/bin

clean:
	rm crt0.o more.o more
