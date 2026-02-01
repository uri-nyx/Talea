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
from sys import argv

class Header:
    
    def __init__(self, path: str):
        self.path = pathlib.Path(path)
        self.name = self.path.stem
    
    def wrap_tps_1mb(self, label: str, bootable: bool, writable: bool):   
        header = bytearray([0 for x in range(0, 512)])
        header[0] = ord('T')
        header[1] = ord('P')
        header[2] = ord('S')
        header[3] = ord('!')
        header[4] = 1
        header[5] = 1 if bootable else 0
        header[5] |= 2 if not writable else 0
        header[6] = 3 # TPS1MB
        header[7] = 8
        header[8] = 0x00
        header[9] = 0x00
        header[10] = 0x08
        header[11] = 0x00
        header[12] = 0x02
        header[13] = 0x00
        
        now = int(time.time())
        header[14] = (now >> 56) & 0xff
        header[15] = (now >> 48) & 0xff
        header[16] = (now >> 40) & 0xff
        header[17] = (now >> 32) & 0xff
        header[18] = (now >> 24) & 0xff
        header[19] = (now >> 16) & 0xff
        header[20] = (now >> 8) & 0xff
        header[21] = (now) & 0xff
        
        name = bytearray(label, "ASCII")
        for i in range(0, 16):
            header[22 + i] = name[i] if len(name) > i else ord(' ')
        
        with open(self.path, "rb") as ifile, \
            open(self.path.stem + (".tps" if writable else ".tpc"), "wb") as ofile:
            data = ifile.read()
            ofile.write(header)
            ofile.write(data)
    
def main(argv):
    if len(argv) < 2:
        print(f"Usage: {argv[0]} <path>")
        print(f"\twraps a raw disk image into the TPS 1MB header, bootable")
    
    tps = Header(argv[1])
    tps.wrap_tps_1mb(tps.path.stem, bootable=True, writable=True)
    
if __name__ == "__main__":
    main(argv)