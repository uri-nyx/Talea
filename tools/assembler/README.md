# Roadmap and Features for the assembler

## Output formats

+ *Hex* or *Bin*, raw formats. Intended for setting up the bootloader and interrupt vector tables
+ *Obj*, *Comm*, *Elf*... A (maybe custom) simple format for executables with some metadata and relocation

## Features

+ Directives: org, align, sections, data insertions, constants, simple math
+ [Preprocessor: macros] <- **Done by a different program that generates plain assembler**
+ Warnings and aligment checking, correctness checking
+ Automatic label offsets and relative addressing
+ Dissasembly?
  
In python. Depends on Arpeggio
