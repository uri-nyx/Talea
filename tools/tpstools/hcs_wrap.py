# Wraps a binary file in the TPS or HCS file format expected by the emulator
# HEADER
# ALL INTEGER MULTIBYTE INTEGER VALUES ARE STORED BIG ENDIAN
# 0x00	Magic Number	    4B	TPS! or HCS! (Identifies this as a Taleä disk).
# 0x04	Version	            1B	Hardware revision (e.g., 0x01).
# 0x05	Flags	            1B	Bit 0: Bootable, Bit 1: Write-Protected.
# 0x06	Medium Type	        1B	see StorageMedium.
# 0x07	Bank count          1B	number of 256 banks
# 0x08	Sector Count	    4B	Total LBA sectors (e.g., 2048 for 1MB).
# 0x0C	Sector Size	        2B	Usually 512.
# 0x0E	Creation Date	    8B	Unix Timestamp (64-bit)
# 0x16    Disk Name	        16B	ASCII label (e.g., "SYSTEM_DISK_01").
# 0x26 -- 0x200               Reserved.

import time
import pathlib
import struct
from sys import argv, stderr
from typing import Tuple

class Header:
    
    hcs_formats = {
        "32M": {
            "type": 4,
            "sectors" : 65536,
            "banks" : 1,
            "bank-size": 65536,
            "sector-size": 512,
        },
        "64M": {
            "type": 5,
            "sectors" : 131072,
            "banks" : 2,
            "bank-size": 65536,
            "sector-size": 512,
        },
        "128M": {
            "type": 6,
            "sectors" : 262144,
            "banks" : 4,
            "bank-size": 65536,
            "sector-size": 512,
        }
    }

    def __init__(self, path: str, fs: str = "FAT16"):
        self.path = pathlib.Path(path)
        self.name = self.path.stem
        self.fs = fs
        
    def wrap(self, label: str, hcs_format: str):   
        # Flags: Bit 0 = Bootable, Bit 1 = Write-Protected (not writable)
        
        T = Header.hcs_formats[hcs_format]
        
        header_data = struct.pack(
            ">4sBBBB I H Q 16s",
            b"HCS!",          
            0x01,              # Version
            0,             
            T["type"],              
            T["banks"],              
            T["sectors"],              
            T["sector-size"],               
            int(time.time()),  
            label.encode('ascii')[:16].ljust(16, b' ') 
        )

        # Pad to 512 bytes
        header = header_data.ljust(512, b'\x00')
        
        with open(self.path, "rb") as ifile, \
            open(self.path.stem + ".hcs", "wb") as ofile:
                print(ofile.name)
                data = ifile.read()
                ofile.write(header)
                ofile.write(data)
    
    
def main(argv):
    if len(argv) < 4:
        print(f"Usage: {argv[0]} <image> <fs> <format>")
        print(f"\twraps a raw disk image into the TPS header. ")
        print(f"\tOptions:")
        print(f"\t<image>: path to image file")
        print(f"\t<fs>: image filesystem. Only FAT16 for now")
        print(f"\t<format>: TPS format (32M, 64M, 128M)")
        exit(1)
    
    image = argv[1]
    fs = argv[2]
    hcs_format =argv[3]
    
    if  hcs_format not in Header.hcs_formats:
        print(f"Format {hcs_format} is not recognized. Use 128K, 512K or 1M")
        exit(1)
    
    hcs = Header(path=image, fs=fs)
    hcs.wrap(hcs.path.stem, hcs_format=hcs_format)
    
    
if __name__ == "__main__":
    main(argv)
