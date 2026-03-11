CPPFLAGS := -I$(SDK) -I$(NAIMEN)/libc/include
LIBS := -l $(NAIMEN)/libc/libcmin.a

all: ed

ed: ed.c
	$(CC) $(CPPFLAGS) -c $(CRT0) ed.c
	$(LD) $(LIBS) $(LDFLAGS) -o ed crt0.o ed.o
	cp ed ../tps_a/bin

clean:
	rm crt0.o ed.o ed
