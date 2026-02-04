CPPFLAGS = -zI -I../ -I../freestanding
BINUTILS=../../tools/lcc-sirius/binutils/bin
LCCDIR=../../tools/lcc-sirius/lcc/bin
TOOLS=../../tools


LD=$(BINUTILS)/ld
CC=$(LCCDIR)/lcc


img:
	dd if=/dev/zero of=build/akai.img bs=512 count=2048
	mkfs.fat -F 12 build/akai.img -f2 -s2 -n "AKAI" -S512 -M 0xf0 -r 16

kernel: src/entry.s src/hw.s src/kernel.c src/mmu.c src/mem.c src/process.c src/exception.c src/syscall.c src/interrupt.c img
	$(CC) $(CPPFLAGS) -c src/entry.s src/hw.s src/kernel.c src/mmu.c src/mem.c src/process.c src/exception.c src/syscall.c src/interrupt.c
	$(LD) -h -o build/kernel.bin -m build/kernel.map entry.o hw.o kernel.o mmu.o mem.o process.o exception.o syscall.o interrupt.o
	rm entry.o kernel.o mmu.o hw.o mem.o process.o exception.o syscall.o interrupt.o
	mcopy -i build/akai.img build/kernel.bin "::KERNEL"

tps: kernel
	python3 $(TOOLS)/tpstools/tps_wrap.py build/akai.img build/kernel.map _kstart

load: tps
	cp akai.tps ../../resources/tps_images/akai.tps 