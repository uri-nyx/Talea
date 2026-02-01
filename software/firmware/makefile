# relocate at the top of the address space, where ROM sits
# and data to 0x8000
LDFLAGS = -rc 16711680 -rd 32768 
# include libsirius headers
CPPFLAGS = -zI -I../ -I../freestanding

BINUTILS=../../tools/lcc-sirius/binutils/bin
LCCDIR=../../tools/lcc-sirius/lcc/bin

LD=$(BINUTILS)/ld
CC=$(LCCDIR)/lcc

all: entry.s firmware.c
	$(CC) $(CPPFLAGS) -c entry.s firmware.c 
	$(LD) $(LDFLAGS) -o firmware.bin entry.o firmware.o  
.PHONY: clean
clean:
	rm -f firmware.bin entry.o firmware.o 

