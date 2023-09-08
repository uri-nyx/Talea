#node ../JackCompiler/JackCompiler.js utils/firmware/jack -x
#python3 utils/tallum/tallum.py sft -O -o utils/firmware/jack.s utils/firmware/jack/Main.vm --std=tal
#customasm utils/asm/master.asm utils/asm/sys.asm utils/firmware/isr.s -o jack.bin
 ./tpsA.sh ../Ion/tps/ion.tps
echo "\t\v--- TALEÄ Z EXECUTION ---"
./zig-out/bin/TaleaZ -f minimal.bin -z $1 --scale=$2
