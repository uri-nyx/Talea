
export BINUTILS:=$(abspath ../../tools/lcc-sirius/binutils/bin)
export LCCDIR:=$(abspath ../../tools/lcc-sirius/lcc/bin)
export TOOLS:=$(abspath ../../tools)
export SDK:= $(abspath sdk)



export AS:=$(BINUTILS)/as
export AR:=$(BINUTILS)/ar
export CC:=$(LCCDIR)/lcc
export LD:=$(BINUTILS)/ld

export CPPFLAGS:= -zI

HEADERS = $(shell find include -name "*.h" -not -name "akai.h")


sdk/akai.h: $(HEADERS)
	@printf "#ifndef AKAI_H\n#define AKAI_H\n\n" > $@
	@find include -name "*.h" -not -name "akai.h" -exec awk ' \
		/@AKAI:/ { tag=$$3; gsub(/[ \t\*\/]+/, "", tag); active=1; next } \
		/@AKAI/  { active=0; next } \
		active   { print tag "\t" NR "\t" $$0 }' {} + \
		| sort -k1,1 -k2,2n | cut -f3- >> $@
	@cp sdk/_crt0.s sdk/crt0.s
	@sed -n '/BEGIN_SYSCALL_LIST/,/END_SYSCALL_LIST/p' $@ | python3 gen_syscall_wrappers.py $@ sdk/crt0.s
	@cat src/hw_user.s >> sdk/crt0.s
	@printf "\n#endif /* AKAI_H */\n" >> $@

img:
	dd if=/dev/zero of=build/akai.img bs=512 count=2048
	mkfs.fat -F 12 build/akai.img -f2 -s2 -n "AKAI" -S512 -M 0xf0 -r 16

utils:
	dd if=/dev/zero of=build/akai_utils.img bs=512 count=2048
	mkfs.fat -F 12 build/akai_utils.img -f2 -s2 -n "UTILS" -S512 -M 0xf0 -r 16

format-hcs:
	dd if=/dev/zero of=build/hcs.img bs=512 count=65536
	mkfs.fat -F 16 build/hcs.img -f2 -s1 -n "NAI" -S512 -M 0xf8 -r 512
	python3 $(TOOLS)/tpstools/hcs_wrap.py build/hcs.img FAT16 32M
	mv hcs.hcs ../../resources/emulated/devices/hcs/drive.hcs


init: src/init.s
	$(AS) -o init.out src/init.s && $(LD) -o init.bin -rc 524288 -h init.out && xxd -i init.bin > include/init_code.h
	rm init.out init.bin

kernel: src/entry.s src/hw.s src/hw_user.s src/kernel.c src/mmu.c src/mem.c src/process.c src/exception.c src/syscall.c src/interrupt.c src/storage.c src/ff.c src/diskio.c src/kstring.c src/proxy.c src/kb.c src/video.c src/asm.c src/serial.c src/mouse.c img init
	$(CC) $(CPPFLAGS) -Iinclude/ -c src/entry.s src/hw.s src/hw_user.s src/kernel.c src/mmu.c src/mem.c src/process.c src/exception.c src/syscall.c src/interrupt.c src/storage.c src/ff.c src/diskio.c src/kstring.c src/proxy.c src/kb.c src/video.c src/asm.c src/serial.c src/mouse.c 
	$(LD) -h -ra -o build/kernel.bin -m build/kernel.map entry.o hw.o hw_user.o kernel.o mmu.o mem.o process.o exception.o syscall.o interrupt.o storage.o ff.o diskio.o kstring.o proxy.o kb.o video.o asm.o serial.o mouse.o
	rm entry.o kernel.o mmu.o hw.o hw_user.o mem.o process.o exception.o syscall.o interrupt.o storage.o ff.o diskio.o kstring.o proxy.o kb.o video.o asm.o serial.o mouse.o
	mcopy -i build/akai.img build/kernel.bin "::KERNEL"

check:
	clang -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast  -nostdinc -nobuiltininc -H -Iinclude/ -fsyntax-only -std=c89 -Wall -Wextra -Wno-comment src/entry.s src/hw.s src/kernel.c src/mmu.c src/mem.c src/process.c src/exception.c src/syscall.c src/interrupt.c src/storage.c src/ff.c src/diskio.c src/kstring.c src/proxy.c src/kb.c src/video.c src/asm.c

check-errors:
	clang -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast  -nostdinc -nobuiltininc -H -Iinclude/ -fsyntax-only -std=c89 -Wno-comment src/entry.s src/hw.s src/kernel.c src/mmu.c src/mem.c src/process.c src/exception.c src/syscall.c src/interrupt.c src/storage.c src/ff.c src/diskio.c src/kstring.c src/proxy.c src/kb.c src/video.c src/asm.c

compiler-test:
	$(CC) -v -S t.c
	micro t.s

compiler-test-d:
	$(CC) -v -S -Wf-d t.c
	micro t.s

naimen: kernel
	$(MAKE) -C naimen

libs:
	$(MAKE) -C naimen libs

libc-d:
	$(MAKE) -C naimen libc-d

apps:
	$(MAKE) -C naimen apps

tps: naimen
	python3 $(TOOLS)/tpstools/tps_wrap.py build/akai.img FAT12 1M --bootable build/kernel.map _kstart KERNEL 1024

load: tps
	python3 $(TOOLS)/tpstools/tps_wrap.py build/akai_utils.img FAT12 1M 
	cp akai.tps ../../resources/tps_images/akai.tps 
	cp akai_utils.tps ../../resources/tps_images/
