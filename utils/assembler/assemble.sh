#!/bin/bash

infile=$1
outfile=$2

# 1: Expand macros
python macro.py $infile expanded.s

# 2: Truncate macros
cat expanded.s | sed '/endmacro/,$!d' | sed -e '/endmacro/ { d; }' > expanded2.s
# 3: Assembsle
cat expanded2.s
python include/assembler.py -x -o expanded.s expanded2.s

# 4: To Little Endian
python big2little.py expanded.s

# 5: To hex
xxd -r -p expanded.s $outfile

rm expanded.s
rm expanded2.s
echo "Assembled Real Mode Binary"
