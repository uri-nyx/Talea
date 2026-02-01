CPPFLAGS = -zI -I../ -I../freestanding

BINUTILS=../../tools/lcc-sirius/binutils/bin
LCCDIR=../../tools/lcc-sirius/lcc/bin
TOOLS=../../tools

LD=$(BINUTILS)/ld
CC=$(LCCDIR)/lcc

img:
	dd if=/dev/zero of=build/akai.img bs=512 count=2048
	mkfs.fat -F 12 build/akai.img -f2 -s2 -n "AKAI" -S512 -M 0xf0

tps: kernel
	python3 $(TOOLS)/tpstools/tps_wrap.py build/akai.img

kernel: entry.s kernel.c img
	$(CC) $(CPPFLAGS) -c entry.s kernel.c
	$(LD) -h -g -o build/kernel.bin -m build/kernel.map entry.o kernel.o
	rm entry.o kernel.o
	mcopy -i build/akai.img build/kernel.bin "::KERNEL"