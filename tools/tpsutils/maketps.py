from sys import argv

def main(argv):
    
    if len(argv) != 2:
        print(
            """Usage: maketps.py <new_file_name>
            This program will emit a blank 128Kb tps image, marked as bootable""")
        return
    
    fname = argv[1]
    
    blank_sector = bytearray(512)
    with open(fname, "wb") as tps:
        first_sector = blank_sector
        #add boot signature
        first_sector[510] = 0xaa
        first_sector[511] = 0x55
        tps.write(blank_sector)
        
        for i in range(255):
            tps.write(blank_sector)
            
if __name__ == "__main__":
    main(argv)