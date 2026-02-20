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
    
    tps_formats = {
        "128K": {
            "type": 1,
            "sectors" : 256,
            "banks" : 1,
            "bank-size": 256,
            "sector-size": 512,
        },
        "512K": {
            "type": 2,
            "sectors" : 1024,
            "banks" : 4,
            "bank-size": 256,
            "sector-size": 512,
        },
        "1M": {
            "type": 3,
            "sectors" : 2048,
            "banks" : 8,
            "bank-size": 256,
            "sector-size": 512,
        }
    }

    def fat12_get_file(self, data: bytes) -> Tuple[int, int]:
        reserved_sectors = struct.unpack("<H", data[14:16])[0]
        num_fats = data[16]
        fat_size = struct.unpack("<H", data[22:24])[0]
        root_entries = struct.unpack("<H", data[17:19])[0]
        sectors_per_cluster = data[13]

        # 2. Calculate Offsets
        root_dir_start = (reserved_sectors + (num_fats * fat_size)) * 512
        root_dir_size = root_entries * 32
        data_region_start = root_dir_start + root_dir_size

        # 3. Search Root Directory
        for i in range(0, root_entries):
            entry_offset = root_dir_start + (i * 32)
            entry = data[entry_offset : entry_offset + 32]
            
            # Check if empty or deleted
            if entry[0] == 0x00: break
            if entry[0] == 0xE5: continue
            
            # Extract name (8.3 format)
            file_name = entry[0:11].decode('ASCII')
            if file_name == self.fname:
                first_cluster = struct.unpack("<H", entry[26:28])[0]
                file_size = struct.unpack("<I", entry[28:32])[0]
                
                # FAT cluster 2 is the start of the data region
                # LBA = ((cluster - 2) * sectors_per_cluster) + data_region_sector_offset
                data_sector = ((first_cluster - 2) * sectors_per_cluster)
                abs_lba = (data_region_start // 512) + data_sector
                
                return abs_lba, file_size

        return None, None
    
    def parse_map(self) -> Tuple[int, int]:
        entry, load_addr = None, None
        with open(self.map, "r") as m:
            for line in m.readlines():
                if self.entry_symbol in line:
                    t = line.split()
                    if "CODE" not in t:
                        print(f"Error: cannot write boot header: entry point {self.entry_symbol} must be in CODE segment", file=stderr)
                        exit(1)
                    
                    entry = int(t[2].split("-")[0], 16)
                
                elif "CODE" in line and "start" in line:
                    load_addr = int(line.split()[2], 16)
                    break
                
        return entry, load_addr
 
    def write_boot_header(self, data: bytes) -> bytes:
        if self.fs == "FAT12":
            sector, fsize = self.fat12_get_file(data)
            if sector == None or fsize == None:
                print(f"Error: cannot write boot header: {self.fname} not found", file=stderr)
                exit(1)
            
            entry, load_addr = self.parse_map()
            if entry == None or load_addr == None:
                print(f"Error: cannot write boot header: {self.entry_symbol} not found", file=stderr)
                exit(1)
            
            data = bytearray(data)
            boot_header = struct.pack(
                ">4sIIIII",
                b"TAL!",
                entry,
                load_addr,
                (fsize +511)//512,
                sector,
                self.min_ram
            )
            
            data[0x100:0x100+len(boot_header)] = boot_header
            return data
    
        else:
            print(f"Error: cannot write boot header to filesystem {self.fs}: Not supported", file=stderr)
            exit(1)
    
    def fname_to_8_3(self, fname:str) -> str:
        # Ensure uppercase as per FAT standard
        fname = fname.upper()
        
        name = ""
        extension = "" 
        if '.' in fname:
            l = fname.rsplit('.', 1) 
            name = l[0]
            extension = l[1]
        else:
            name = fname

        name = name[:8].ljust(8, ' ')
        extension = extension[:3].ljust(3, ' ')

        return name + extension

    def __init__(self, path: str, map_file_path: str, min_ram: int = 256*1024, fs: str = "FAT12", fname: str = "KERNEL", entry_symbol: str = "_start"):
        self.path = pathlib.Path(path)
        self.name = self.path.stem
        self.map = pathlib.Path(map_file_path)
        self.fs = fs
        self.fname = self.fname_to_8_3(fname)
        self.entry_symbol = entry_symbol
        self.min_ram = min_ram
        
    def wrap(self, label: str, tps_format: str, bootable: bool, writable: bool):   
        # Flags: Bit 0 = Bootable, Bit 1 = Write-Protected (not writable)
        flags = (1 if bootable else 0) | (2 if not writable else 0)
        
        T = Header.tps_formats[tps_format]
        
        header_data = struct.pack(
            ">4sBBBB I H Q 16s",
            b"TPS!",          
            0x01,              # Version
            flags,             
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
            open(self.path.stem + (".tps" if writable else ".tpc"), "wb") as ofile:
            data = ifile.read()
            ofile.write(header)
            
            if bootable: data = self.write_boot_header(data)
            ofile.write(data)
    
    
def main(argv):
    if len(argv) < 4:
        print(f"Usage: {argv[0]} <image> <fs> <format> [--bootable <linker-map> <entry-symbol> <boot-filename> <min-ram-K>] [--wprot]")
        print(f"\twraps a raw disk image into the TPS header. ")
        print(f"\tOptions:")
        print(f"\t<image>: path to image file")
        print(f"\t<fs>: image filesystem. Only FAT12 for now")
        print(f"\t<format>: TPS format (128K, 512K, 1M)")
        print(f"\t--bootable: marks tps TPS as bootable")
        print(f"\t--wprot: marks the TPS as write protected")
        exit(1)
    
    image = argv[1]
    fs = argv[2]
    tps_format =argv[3]
    
    if  tps_format not in Header.tps_formats:
        print(f"Format {tps_format} is not recognized. Use 128K, 512K or 1M")
        exit(1)
        
    bootable = False
    writable = True
    linker_map = ""
    entry_symbol = ""
    boot_fname = "KERNEL"
    min_ram = 64 * 1024
    
    options = argv[3:]
    for i, option in enumerate(options):
        if option == "--bootable":
            if (len(options) < 5):
                print(f"Option --bootable takes <linker-map>, <entry-symbol>, <boot-filename>, and required amount of ram in Kb")
                exit(1)
            bootable = True
            linker_map =  options[i + 1]
            entry_symbol =  options[i + 2]
            boot_fname =  options[i + 3]
            min_ram = int(options[i+4]) * 1024
        elif option == "--wprot":
            writable = False
        else:
            continue
        
        
    
    tps = Header(path=image, map_file_path=linker_map, min_ram=min_ram, fs=fs, fname=boot_fname, entry_symbol=entry_symbol)
    tps.wrap(tps.path.stem, tps_format=tps_format, bootable=bootable, writable=writable)
    
    
if __name__ == "__main__":
    main(argv)