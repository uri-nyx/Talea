# relocate at the top of the address space, where ROM sits
LDFLAGS = -h -rc 16769024 
# include libsirius headers
CPPFLAGS = -I../

BINUTILS=../../tools/lcc-sirius/binutils/bin
LCCDIR=../../tools/lcc-sirius/lcc/bin

LD=$(BINUTILS)/ld
CC=$(LCCDIR)/lcc

all: ushell.c entry.s bios.c
	$(CC) $(CPPFLAGS) -c entry.s bios.c ushell.c
	$(LD) $(LDFLAGS) -o bios.bin entry.o bios.o ushell.o 

.PHONY: clean
clean:
	rm -f bios.bin entry.o bios.o ushell.o

