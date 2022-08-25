# Taleä Tabula System Cross Assembler
from sys import argv
import asmodule.asm, asmodule.machineconst, asmodule.macro

def main(argv):
    if len(argv) < 4:
        print("Usage: tcasm.py <output_format> <source_file> <output_file> [<options>]")
        return
    
    output_format = argv[1]
    source_file = argv[2]
    output_file = argv[3]
    options = None if len(argv) == 4 else argv[4:]
    
    formats = ["raw"]
    
    if output_format not in formats:
        print("Invalid output format: " + output_format)
        return
    
    if output_format == "raw":
        with open(source_file, "r") as f:
            source = f.read()
            
        if "macro" in options:
            source = asmodule.macro.expand_macros_and_remove_definitions(source)
            
        assembler = asmodule.asm.Assembler(source)
        hex_string = assembler.assemble_raw()
        
        with open(output_file, "wb") as f:
            f.write(bytes.fromhex(hex_string))
        
        if "debug" in options:
            debug_info = str(assembler.labels) + "\n"
            debug_info += str(assembler.section_start_addresses) + "\n"
            debug_info += str(assembler.sections) + "\n"
            debug_info += "Assembler Dump\n" + str(assembler.debug_info)
            
            with open(output_file + ".debug", "w") as f:
                f.write(debug_info + "\n\n" + hex_string)
        

if __name__ == "__main__":
    main(argv)