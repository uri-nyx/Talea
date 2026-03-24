CPPFLAGS := -I$(SDK) -I$(NAIMEN)/libc/include
LIBS := -l $(NAIMEN)/libc/libcmin.a

all: tui

tui: tui.c
	$(CC) $(CPPFLAGS) -c $(CRT0) tui.c
	$(LD) $(LIBS) $(LDFLAGS) -m tui.map -o tui crt0.o tui.o
	cp tui ../tps_b/bin

clean:
	rm crt0.o tui.o tui
