#ifndef STORAGE_H
#define STORAGE_H

#include <threads.h> // C11 or more

#include "frontend/config.h"
#include "logging.h"
#include "machine_description.h"
#include "types.h"

#ifndef DO_NOT_INCLUDE_RAYLIB
// Due to confilcts with the WinAPI
#include "raylib.h"
#else
// definitions we need from raylib
#include "need_from_raylib.h"
#endif

struct TaleaMachine;

/*----------------------------------------------------------------------------*/
/* DEVICE MAGIC ID                                                            */
/*----------------------------------------------------------------------------*/

#define DEVICE_STORAGE_MAGIC (1398034258) // 'STOR'

/*----------------------------------------------------------------------------*/
/* DEVICE PORTS                                                               */
/*----------------------------------------------------------------------------*/

#define P_STORAGE_TPS_COMMAND 0x0
#define P_STORAGE_TPS_DATA    0x1
#define P_STORAGE_TPS_POINTH  0x2
#define P_STORAGE_TPS_POINTL  0x3
#define P_STORAGE_TPS_RESULT  0x4
#define P_STORAGE_TPS_STATUS  0x5

#define P_STORAGE_HCS_COMMAND 0x6
#define P_STORAGE_HCS_DATA    0x7
#define P_STORAGE_HCS_SECTORH 0x8
#define P_STORAGE_HCS_SECTORL 0x9
#define P_STORAGE_HCS_POINTH  0xa
#define P_STORAGE_HCS_POINTL  0xb
#define P_STORAGE_HCS_RESULT  0xc
#define P_STORAGE_HCS_STATUS  0xd

/*----------------------------------------------------------------------------*/
/* DEVICE CONSTANTS                                                           */
/* ---------------------------------------------------------------------------*/

#define STORAGE_SECTOR_SIZE 512
#define HCS_FILE_PATH       "resources/emulated/devices/hcs/drive.hcs"

enum StorageCommand {
    STORAGE_COMMAND_NOP,        // does nothing
    STORAGE_COMMAND_STORE,      // stores to sector DATA of current bank from POINT_H:POINT_L
    STORAGE_COMMAND_LOAD,       // loads from sector DATA of current bank in POINT_H:POINT_L
    STORAGE_COMMAND_SETCURRENT, // sets the drive (A or B)
    STORAGE_COMMAND_GETCURRENT, // returns the current selected drive on STATUS_H
    STORAGE_COMMAND_MEDIUM,     // returns the medium type in STATUS_H. Can infer size,
                                //      sector count, etc based on a lookup table
    STORAGE_COMMAND_BANK,       // changes bank
    STORAGE_COMMAND_GETBANK,    // return the current selected bank on STATUS_H
};

enum StorageStatus {
    STORAGE_STATUS_READY = 0x01,
    STORAGE_STATUS_ERROR = 0x02,    // an error happened in the last operation
    STORAGE_STATUS_DONE  = 0x04,    // thread is done processing, it is safe to raise
                                    // interrupt
    STORAGE_STATUS_INSERTED = 0x08, // always high if inserted
    STORAGE_STATUS_WPROT    = 0x10, // always high if write protected
    STORAGE_STATUS_BOOT     = 0x20, // always high if disk is bootable. Also marked in
                                    // first sector
    STORAGE_STATUS_BUSY = 0x80,     // currently doing something, locked
};

enum StorageTpsId { STORAGE_TPS_ID_A, STORAGE_TPS_ID_B, STORAGE_TPS_TOTAL_DRIVES };

enum StorageMedium {
    STORAGE_NO_MEDIA, // No medium detected
    STORAGE_TPS_128K, // Tps, 256 sectors, 1 bank of 256 sectors, sector is 512 bytes
    STORAGE_TPS_512K, // Tps, 1024 sectors, 4 banks of 256 sectors, sector is 512 bytes
    STORAGE_TPS_1M,   // Tps, 2048 sectors, 8 banks of 256 sectors, sector is 512 bytes
    STORAGE_HCS_32M,  // Hcs, 65536 sectors, 1 banks of 65536 sectors, sector is 512 bytes
    STORAGE_HCS_64M,  // Hcs, 131072 sectors, 2 banks of 65536 sectors, sector is 512 bytes
    STORAGE_HCS_128M  // Hcs, 262144 sectors, 4 banks of 65536 sectors , sector is 512 bytes
};

enum StorageMediumLookup {
    STORAGE_MEDIUM_LOOKUP_TYPE,
    STORAGE_MEDIUM_LOOKUP_SECTORS,
    STORAGE_MEDIUM_LOOKUP_BANKS,
    STORAGE_MEDIUM_LOOKUP_BANK_SZ,
    STORAGE_MEDIUM_LOOKUP_SECTOR_SZ,
};

#define STORAGE_NOMEDIA 0
#define STORAGE_TPS     1
#define STORAGE_HCS     2

static const u32 Storage_MediumLookup[7][5] = {
    // TYPE, SECTOR COUNT, BANKS, BANK SIZE, SECTOR SIZE
    [STORAGE_NO_MEDIA] = { STORAGE_NOMEDIA, 0, 0, 0, 0 },
    [STORAGE_TPS_128K] = { STORAGE_TPS, 256, 1, 256, 512 },
    [STORAGE_TPS_512K] = { STORAGE_TPS, 1024, 4, 256, 512 },
    [STORAGE_TPS_1M]   = { STORAGE_TPS, 2048, 8, 256, 512 },
    [STORAGE_HCS_32M]  = { STORAGE_HCS, 65536, 1, 65536, 512 },
    [STORAGE_HCS_64M]  = { STORAGE_HCS, 131072, 2, 65536, 512 },
    [STORAGE_HCS_128M] = { STORAGE_HCS, 262144, 4, 65536, 512 },
};

// Storage header

/*
ALL INTEGER MULTIBYTE INTEGER VALUES ARE STORED BIG ENDIAN
0x00	Magic Number	    4B	TPS! or HCS! (Identifies this as a Taleä disk).
0x04	Version	            1B	Hardware revision (e.g., 0x01).
0x05	Flags	            1B	Bit 0: Bootable, Bit 1: Write-Protected.
0x06	Medium Type	        1B	see StorageMedium.
0x07	Bank count          1B	number of 256 banks
0x08	Sector Count	    4B	Total LBA sectors (e.g., 2048 for 1MB).
0x0C	Sector Size	        2B	Usually 512.
0x0E	Creation Date	    8B	Unix Timestamp (64-bit)
0x16    Disk Name	        16B	ASCII label (e.g., "SYSTEM_DISK_01").
0x26 -- 0x200               Reserved.
*/

#define STORAGE_HEADER_SIZE      512
#define STORAGE_HEADER_TPS_MAGIC "TPS!"
#define STORAGE_HEADER_HCS_MAGIC "HCS!"

#define STORAGE_HEAD_MAGIC        0x0
#define STORAGE_HEAD_VERSION      0x04
#define STORAGE_HEAD_FLAGS        0x05
#define STORAGE_HEAD_MEDIUM       0x06
#define STORAGE_HEAD_BANKS        0x07
#define STORAGE_HEAD_SECTORS      0x08
#define STORAGE_HEAD_SECTORSZ     0x0c
#define STORAGE_HEAD_DATECREATION 0x0e
#define STORAGE_HEAD_LABEL        0x16

#define STORAGE_HEAD_FLAG_BOOT  0x1
#define STORAGE_HEAD_FLAG_WPROT 0x2

/*----------------------------------------------------------------------------*/
/* DEVICE STRUCTURES                                                          */
/*--------------------------------------------------------------------------- */

struct StorageHeader {
    char               magic[4];
    u8                 version;
    bool               bootable, writeProtected;
    enum StorageMedium mediumType;
    u8                 banks;
    u32                sectorCount;
    u16                sectorSize;
    u64                creationDate;
    char               label[16];
};

static inline void Storage_ParseHeader(u8 inBuff[STORAGE_HEADER_SIZE], struct StorageHeader *out)
{
    memcpy(out->magic, inBuff, 4);
    out->version        = inBuff[STORAGE_HEAD_VERSION];
    u8 flags            = inBuff[STORAGE_HEAD_FLAGS];
    out->bootable       = flags & STORAGE_HEAD_FLAG_BOOT;
    out->writeProtected = flags & STORAGE_HEAD_FLAG_WPROT;
    out->mediumType     = inBuff[STORAGE_HEAD_MEDIUM];
    out->banks          = inBuff[STORAGE_HEAD_BANKS];
    out->sectorCount =
        (u32)inBuff[STORAGE_HEAD_SECTORS] << 24 | (u32)inBuff[STORAGE_HEAD_SECTORS + 1] << 16 |
        (u32)inBuff[STORAGE_HEAD_SECTORS + 2] << 8 | (u32)inBuff[STORAGE_HEAD_SECTORS + 3];
    out->sectorSize = (u16)inBuff[STORAGE_HEAD_SECTORSZ] << 8 |
                      (u16)inBuff[STORAGE_HEAD_SECTORSZ + 1];
    out->creationDate = (u64)inBuff[STORAGE_HEAD_DATECREATION] << 56 |
                        (u64)inBuff[STORAGE_HEAD_DATECREATION + 1] << 48 |
                        (u64)inBuff[STORAGE_HEAD_DATECREATION + 2] << 40 |
                        (u64)inBuff[STORAGE_HEAD_DATECREATION + 3] << 32 |
                        (u64)inBuff[STORAGE_HEAD_DATECREATION + 4] << 24 |
                        (u64)inBuff[STORAGE_HEAD_DATECREATION + 5] << 16 |
                        (u64)inBuff[STORAGE_HEAD_DATECREATION + 6] << 8 |
                        (u64)inBuff[STORAGE_HEAD_DATECREATION + 7];
    memcpy(out->label, &inBuff[STORAGE_HEAD_LABEL], 16);
}

// Struct passed to worker tps threads
typedef struct {
    enum StorageTpsId id;          // the id of the tps being referred to
    u8                bank;        // captured value of selected bank
    u8                sector;      // captured value of DATA register (0-255)
    u8               *ptr;         // Direct pointer to &data_memory[POINT]
    u8                command;     // 0x01 (STORE) or 0x02 (LOAD)
    atomic_uchar     *driveStatus; // pointer to the drive's status register
    u32               max_lba;     // Derived from header; used to clamp access
} StorageTpsRequest;

typedef struct {
    u32           sector;
    u8           *ptr;         // Direct pointer to &data_memory[POINT]
    u8            command;     // 0x01 (STORE) or 0x02 (LOAD)
    atomic_uchar *driveStatus; // pointer to the drive's status register
    u32           max_lba;     // Derived from header; used to clamp access
} StorageHcsRequest;

typedef struct StorageTps {
    struct StorageHeader header;
    enum StorageTpsId    id;
    FILE                *file;
    bool                 inserted;
    bool                 writeProtected;
    bool                 justInserted;
    bool                 justEjected;
    // registers
    u8           data;
    u16          point;
    u8           result;
    u8           bank;
    atomic_uchar status;
} StorageTps;

typedef struct {
    struct StorageHeader header;
    FILE                *file; // The persistent handle to the .hcs file
    // registers
    u8           data;
    u16          sector;
    u16          point;  // 16-bit register (to be shifted << 9)
    u8           result; // BUSY, ERROR, READY bits
    atomic_uchar status;
    u32          totalSectors; // Cached from the header for safety checks
} StorageHcs;

/*----------------------------------------------------------------------------*/
/* DEVICE State                                                           */
/*----------------------------------------------------------------------------*/

typedef struct DeviceStorage {
    StorageTps        tpsDrives[STORAGE_TPS_TOTAL_DRIVES];
    StorageTps       *currentTps;
    enum StorageTpsId currentTpsId;
    StorageHcs        hcs;
} DeviceStorage;

/*----------------------------------------------------------------------------*/
/* DEVICE INTERFACE                                                           */
/*----------------------------------------------------------------------------*/

// Callbacks for Raylib's AudioStream
// Here put the callbacks you want raylib to execute on the AudioStream thread

// DEVICE INITIALIZATION
// and what do we do wen we pass *config?
void Storage_Reset(struct TaleaMachine *m, TaleaConfig *config, bool isRestart);

// DEVICE DEINITIALIZATION
void Storage_Deinit(struct TaleaMachine *m);

// DEVICE UPDATE (on vblank)
// NONE

// DEVICE UPDATE (on cpu tick)
// YES but its harcoded inCheckInterrupts

// DEVICE RENDER

// And this? it hooks up to the frontend. Maybe implement some declarative UI bits?
bool Storage_InsertTps(enum TpsId tps_id, const char *path);
bool Storage_EjectTps(enum TpsId tps_id);

// DEVICE PORT IO HANDLERS
void Storage_Write(struct TaleaMachine *m, u8 port, u8 value);
u8   Storage_Read(struct TaleaMachine *m, u8 port);

#endif
