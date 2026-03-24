CPPFLAGS := -I$(SDK) -I$(NAIMEN)/libc/include
LIBS := -l $(NAIMEN)/libc/libcmin.a

all: nanocom

nanocom: nanocom.c
	$(CC) $(CPPFLAGS) -c $(CRT0) nanocom.c
	$(LD) $(LIBS) $(LDFLAGS) -o nanocom crt0.o nanocom.o
	cp nanocom ../tps_b/bin

clean:
	rm crt0.o nanocom.o nanocom
