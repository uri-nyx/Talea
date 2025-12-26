#include <stdio.h>
#include <string.h>
#include <threads.h> // C11 or more

#include "core/bus.h"
#include "talea.h"

// PORTS

#define P_HCS_COMMAND (DEV_DISK_BASE + 0)
#define P_HCS_DATA    (DEV_DISK_BASE + 1)
#define P_HCS_SECTORH (DEV_DISK_BASE + 2)
#define P_HCS_SECTORL (DEV_DISK_BASE + 3)
#define P_HCS_POINTH  (DEV_DISK_BASE + 4)
#define P_HCS_POINTL  (DEV_DISK_BASE + 5)
#define P_HCS_STATUSH (DEV_DISK_BASE + 6)
#define P_HCS_STATUSL (DEV_DISK_BASE + 7)

#define P_TPS_COMMAND (DEV_TPS_BASE + 0)
#define P_TPS_DATA    (DEV_TPS_BASE + 1)
#define P_TPS_POINTH  (DEV_TPS_BASE + 2)
#define P_TPS_POINTL  (DEV_TPS_BASE + 3)
#define P_TPS_STATUSH (DEV_TPS_BASE + 4)
#define P_TPS_STATUSL (DEV_TPS_BASE + 5)

enum StorageCommand {
    COMMAND_NOP,   // does nothing
    COMMAND_STORE, // stores to sector DATA of current bank from POINT_H:POINT_L
    COMMAND_LOAD,  // loads from sector DATA of current bank in POINT_H:POINT_L
    COMMAND_GET_STATUS, // returns the status register on STATUS_L. Should
                        // always be there
    COMMAND_SETCURRENT, // Sets the drive (A or B)
    COMMAND_GETCURRENT, // returns the current selected drive on STATUS_H
    COMMAND_MEDIUM,     // Returns the medium type in STATUS_H. Can infer size,
                        // sector count, etc based on a lookup table
    COMMAND_BANK,       // changes bank
    COMMAND_GETBANK,    // Return the current selected bank on STATUS_H
};

typedef struct {
    // --- File System Context ---
    enum TpsId id;     // The id of the tps being referred to
    u8         bank;   // Captured value of internal_bank_latch
    u8         sector; // Captured value of DATA register (0-255)

    // --- Memory Context ---
    u8 *dest_ram_ptr; // Direct pointer to machine_ram[POINT]

    // --- Command Context ---
    u8 command; // 0x01 (STORE) or 0x02 (LOAD)

    // --- Status Synchronization ---
    // This points back to the hardware's status register
    // Use 'volatile' to ensure the compiler doesn't optimize out
    // the polling checks in the main loop.
    volatile u8 *hardware_status;

    // --- Security Check ---
    u32 max_lba; // Derived from header; used to clamp access
} TpsRequest;

typedef struct {
    // --- File System Context ---
    u32 sector;

    // --- Memory Context ---
    u8 *dest_ram_ptr; // Direct pointer to machine_ram[POINT]

    // --- Command Context ---
    u8 command; // 0x01 (STORE) or 0x02 (LOAD)

    // --- Status Synchronization ---
    // This points back to the hardware's status register
    // Use 'volatile' to ensure the compiler doesn't optimize out
    // the polling checks in the main loop.
    volatile u8 *hardware_status;

    // --- Security Check ---
    u32 max_lba; // Derived from header; used to clamp access
} HcsRequest;

static mtx_t tps_mutex;
static int   Tps_WorkerThread(void *req)
{
    extern Tps *TpsDrives;

    puts("In tps-worker\n");

    TpsRequest *request = (TpsRequest *)req;
    // 1. Calculate the absolute byte offset
    // LBA = (Bank << 8) | Sector
    // Host Offset = (LBA * 512) + 512 (for the TPS Header)
    u32 lba         = (request->bank << 8) | request->sector;
    u32 host_offset = (lba * TALEA_SECTOR_SIZE) +
                      STOR_HEADER_SIZE; // TODO: do not harcode the sector size

    /* CRITICAL SECTION, LOCK MUTEX */
    mtx_lock(&tps_mutex);

    Tps *t = &TpsDrives[request->id];

    // 2. Seek to the position
    if (fseek(t->file, host_offset, SEEK_SET) != 0) {
        *request->hardware_status |= STOR_STATUS_ERROR;
        *request->hardware_status &= ~STOR_STATUS_BUSY;
        return -1;
    }

    if (request->command == COMMAND_LOAD) {
        // Disk -> RAM
        puts("TPS Worker: Loading\n");
        fread(request->dest_ram_ptr, 1, TALEA_SECTOR_SIZE, t->file);
    }

    else if (request->command == COMMAND_STORE) {
        // RAM -> Disk
        puts("TPS Worker: Storing\n");
        fwrite(request->dest_ram_ptr, 1, TALEA_SECTOR_SIZE, t->file);
        fflush(t->file); // Ensure Host OS flushes buffers

// Optional: Hard sync to physical platter
#ifdef _WIN32
        _commit(_fileno(t->file));
#else
        fsync(fileno(t->file));
#endif
    }

    // 3. Signal completion to the Taleä CPU
    *request->hardware_status &= ~STOR_STATUS_BUSY;
    *request->hardware_status |= STOR_STATUS_READY;
    *request->hardware_status |= STOR_STATUS_DONE;

    mtx_unlock(&tps_mutex); // RELEASE MUTEX

    return 0;
}

/*
Storage device header (only for virtual emulated disks!)
0x00	Magic Number	    4B	TPS! or HCS! (Identifies this as a Taleä disk).
0x04	Version	            1B	Hardware revision (e.g., 0x01).
0x05	Flags	            1B	Bit 0: Bootable, Bit 1: Write-Protected.
0x06	Medium Type	        1B	NoMedia, Tps128K, Tps512K, Tps1M, Hcs32M,
Hcs64M, Hcs128M 0x07	Bank count          1B	number of 256 banks and size of
sector as an exponent of a power of two. 0x08	Sector Count	    4B	Total
LBA sectors (e.g., 2048 for 1MB). STORED IN BIG ENDIAN. 0x0C	Sector Size 2B
Usually 512. STORED in BIG ENDIAN 0x0E	Creation Date	    8B	Unix Timestamp
(64-bit) of when the crystal was "charged." STORED IN BIG ENDIAN 0x16	Disk
Name	        16B	ASCII label (e.g., "SYSTEM_DISK_01"). 0x26 -- 0x200
Reserved.
*/

#define HEAD_MAGIC        0x0
#define HEAD_VERSION      0x04
#define HEAD_FLAGS        0x05
#define HEAD_MEDIUM       0x06
#define HEAD_BANKS        0x07
#define HEAD_SECTORS      0x08
#define HEAD_SECTORSZ     0x0c
#define HEAD_DATECREATION 0x0e
#define HEAD_LABEL        0x16

#define HEAD_FLAG_BOOT  0x1
#define HEAD_FLAG_WPROT 0x2

static void Storage_ParseHeader(u8 in_buff[STOR_HEADER_SIZE],
                                struct StorageHeader *out)
{
    memcpy(out->magic, in_buff, 4);
    out->version         = in_buff[HEAD_VERSION];
    u8 flags             = in_buff[HEAD_FLAGS];
    out->bootable        = flags & HEAD_FLAG_BOOT;
    out->write_protected = flags & HEAD_FLAG_WPROT;
    out->medium_type     = in_buff[HEAD_MEDIUM];
    out->banks           = in_buff[HEAD_BANKS];
    out->sector_count    = (u32)in_buff[HEAD_SECTORS] << 24 |
                        (u32)in_buff[HEAD_SECTORS + 1] << 16 |
                        (u32)in_buff[HEAD_SECTORS + 2] << 8 |
                        (u32)in_buff[HEAD_SECTORS + 3];
    out->sector_size = (u16)in_buff[HEAD_SECTORSZ] << 8 |
                       (u16)in_buff[HEAD_SECTORSZ + 1];
    out->creation_date = (u64)in_buff[HEAD_DATECREATION] << 56 |
                         (u64)in_buff[HEAD_DATECREATION + 1] << 48 |
                         (u64)in_buff[HEAD_DATECREATION + 2] << 40 |
                         (u64)in_buff[HEAD_DATECREATION + 3] << 32 |
                         (u64)in_buff[HEAD_DATECREATION + 4] << 24 |
                         (u64)in_buff[HEAD_DATECREATION + 5] << 16 |
                         (u64)in_buff[HEAD_DATECREATION + 6] << 8 |
                         (u64)in_buff[HEAD_DATECREATION + 7];
    memcpy(out->label, &in_buff[HEAD_LABEL], 16);
}

static bool Tps_ParseHeader(Tps *tps)
{
    // HAS MUTEX LOCK
    u8 buff[STOR_HEADER_SIZE];
    fseek(tps->file, 0, SEEK_SET);
    fread(buff, 1, 512, tps->file);

    struct StorageHeader *h = &tps->header;
    Storage_ParseHeader(buff, h);

    if (memcmp(h->magic, STOR_HEADER_TPS_MAGIC, 4) != 0) {
        return false;
    }

    tps->writeProtected = h->write_protected;
    tps->real_status    = (h->write_protected ? STOR_STATUS_WPROT : 0) |
                       (h->bootable ? STOR_STATUS_BOOT : 0);
    return true;
}

static bool Hcs_ParseHeader(Hcs *hcs)
{
    // HAS MUTEX LOCK
    u8 buff[STOR_HEADER_SIZE];
    fseek(hcs->file, 0, SEEK_SET);
    fread(buff, 1, 512, hcs->file);

    struct StorageHeader *h = &hcs->header;
    Storage_ParseHeader(buff, h);

    if (memcmp(h->magic, STOR_HEADER_HCS_MAGIC, 4) != 0) {
        return false;
    }

    hcs->real_status = (h->write_protected ? STOR_STATUS_WPROT : 0) |
                       (h->bootable ? STOR_STATUS_BOOT : 0);
    return true;
}

bool Storage_InsertTps(enum TpsId tps_id, const char *path)
{
    extern Tps *TpsDrives;
    if (mtx_lock(&tps_mutex) == thrd_success) {
        Tps *t = &TpsDrives[tps_id];

        if (t->inserted) {
            mtx_unlock(&tps_mutex);
            return false;
        }

        t->file = fopen(path, "rb+");
        if (!t->file) {
            mtx_unlock(&tps_mutex);
            return false;
        }

        if (!Tps_ParseHeader(t)) {
            fclose(t->file);
            mtx_unlock(&tps_mutex);
            return false;
        }

        t->inserted      = true;
        t->just_inserted = true;
        t->real_status |= STOR_STATUS_INSERTED | STOR_STATUS_READY;
        t->statusL = t->real_status;
        mtx_unlock(&tps_mutex);

        return true;
    }

    TALEA_LOG_ERROR("Error inserting TPS: error acquiring lock\n");
    return false;
}

void Storage_EjectTps(enum TpsId tps_id)
{
    extern Tps *TpsDrives;
    if (mtx_lock(&tps_mutex) == thrd_success) {
        Tps *t = &TpsDrives[tps_id];

        if (!t->inserted) {
            mtx_unlock(&tps_mutex);
            return;
        }

        fclose(t->file);
        t->file         = NULL;
        t->inserted     = false;
        t->just_ejected = true;
        t->real_status &= ~STOR_STATUS_INSERTED;
        t->statusL = t->real_status;
        mtx_unlock(&tps_mutex);
    }

    TALEA_LOG_ERROR("Error ejecting TPS: error acquiring lock\n");
}

static void Tps_ProcessCommand(TaleaMachine *m, u8 command)
{
    // HAS THE MUTEX LOCKED
    extern Tps *TpsDrives;
    Tps        *t = &TpsDrives[m->storage.current_tps_id];

    if (t->real_status & STOR_STATUS_BUSY) return;

    switch (command) {
    case COMMAND_NOP:
        break;
    case COMMAND_GET_STATUS:
        t->statusL = t->real_status;
        break;
    case COMMAND_MEDIUM:
        t->statusH = t->header.medium_type;
        break;
    case COMMAND_GETBANK:
        t->statusH = t->bank;
        break;
    case COMMAND_GETCURRENT:
        t->statusH = m->storage.current_tps_id;
        break;
    case COMMAND_BANK:
        if (t->data < t->header.banks) {
            t->bank = t->data;
        } else {
            t->real_status |= STOR_STATUS_ERROR;
            t->statusL = t->real_status; // update the status visible from the
                                         // machine side (TODO: document that
                                         // calling LOAD or STORE also calls
                                         // GET_STATUS)
            return;
        }
        break;
    case COMMAND_SETCURRENT:
        if (t->data >= TPS_TOTAL_DRIVES) {
            // that drive does not exist
            t->real_status |= STOR_STATUS_ERROR;
            t->statusL = t->real_status; // update the status visible from the
                                         // machine side (TODO: document that
                                         // calling LOAD or STORE also calls
                                         // GET_STATUS)
            return;
        }
        m->storage.current_tps = &m->storage.tps_drives[t->data];
        break;
    case COMMAND_LOAD:
    case COMMAND_STORE: {
        TALEA_LOG_TRACE("Written Tps load or store!\n");
        if (!t->inserted) return;
        if (command == COMMAND_STORE && t->writeProtected) {
            t->real_status |= STOR_STATUS_ERROR | STOR_STATUS_WPROT;
            t->statusL = t->real_status; // update the status visible from the
                                         // machine side (TODO: document that
                                         // calling LOAD or STORE also calls
                                         // GET_STATUS)
            return;
        }
        // 1. SANDBOX CHECK: Ensure POINT is within RAM boundaries
        // Prevent Taleä from reading/writing outside its own memory
        u32 addr = t->point << 9; // scale this depending on sector size
        if (addr > (TALEA_MAIN_MEM_SZ - TALEA_SECTOR_SIZE) ||
            !(t->bank < t->header.banks)) {
            t->real_status |= STOR_STATUS_ERROR;
            t->statusL = t->real_status; // update the status visible from the
                                         // machine side (TODO: document that
                                         // calling LOAD or STORE also calls
                                         // GET_STATUS)
            return;
        }

        // 2. Prepare the request object for the thread
        TpsRequest tps_request = (TpsRequest){
            .command         = command,
            .bank            = t->bank,
            .sector          = t->data,
            .dest_ram_ptr    = &m->main_memory[addr],
            .id              = m->storage.current_tps_id,
            .hardware_status = &t->real_status,
        };

        // 3. Set Hardware to BUSY
        t->real_status |= STOR_STATUS_BUSY;
        t->real_status &= ~STOR_STATUS_READY;
        t->statusL = t->real_status; // update the status visible from the
                                     // machine side (TODO: document that
                                     // calling LOAD or STORE also calls
                                     // GET_STATUS)

        // 4. Start Thread
        // POSIX: pthread_create
        // Windows: CreateThread or _beginthreadex
        // C11: thrd_create
        // Start_Async_Worker(&tps_request);
        thrd_t worker;
        thrd_create(&worker, Tps_WorkerThread, &tps_request);
        thrd_detach(worker);
        t->statusL = t->real_status;
        break;
    }
    default:
        // command not recognized
        t->real_status |= STOR_STATUS_ERROR;
        t->statusL = t->real_status; // update the status visible from the
                                     // machine side (TODO: document that
                                     // calling LOAD or STORE also calls
                                     // GET_STATUS)
        break;
    }
}

static mtx_t hcs_mutex;
static int   Hcs_WorkerThread(void *req)
{
    extern Hcs *HcsDrive;

    HcsRequest *request = (HcsRequest *)req;
    // 1. Calculate the absolute byte offset
    // LBA = (Bank << 8) | Sector
    // Host Offset = (LBA * 512) + 512 (for the TPS Header)
    u32 host_offset = (request->sector * TALEA_SECTOR_SIZE) +
                      STOR_HEADER_SIZE; // TODO: do not harcode the sector size

    /* CRITICAL SECTION, LOCK MUTEX */
    mtx_lock(&hcs_mutex);

    Hcs *h = HcsDrive;

    // 2. Seek to the position
    if (fseek(h->file, host_offset, SEEK_SET) != 0) {
        *request->hardware_status |= STOR_STATUS_ERROR;
        *request->hardware_status &= ~STOR_STATUS_BUSY;
        return -1;
    }

    if (request->command == COMMAND_LOAD) {
        // Disk -> RAM
        puts("HCS Worker: Loading\n");
        fread(request->dest_ram_ptr, 1, TALEA_SECTOR_SIZE, h->file);
    }

    else if (request->command == COMMAND_STORE) {
        // RAM -> Disk
        puts("HCS Worker: Storing\n");
        fwrite(request->dest_ram_ptr, 1, TALEA_SECTOR_SIZE, h->file);
        fflush(h->file); // Ensure Host OS flushes buffers

// Optional: Hard sync to physical platter
#ifdef _WIN32
        _commit(_fileno(h->file));
#else
        fsync(fileno(request->file_handle));
#endif
    }

    // 3. Signal completion to the Taleä CPU
    *request->hardware_status &= ~STOR_STATUS_BUSY;
    *request->hardware_status |= STOR_STATUS_READY;
    *request->hardware_status |= STOR_STATUS_DONE;

    mtx_unlock(&hcs_mutex);
    return 0;
}

static void Hcs_ProcessCommand(TaleaMachine *m, u8 command)
{
    // HAS THE MUTEX LOCKED
    Hcs *h = &m->storage.hcs;

    if (h->real_status & STOR_STATUS_BUSY) return;

    switch (command) {
    case COMMAND_NOP:
        break;
    case COMMAND_GET_STATUS:
        h->statusL = h->real_status;
        break;
    case COMMAND_MEDIUM:
        h->statusH = h->header.medium_type;
        break;
    case COMMAND_LOAD:
    case COMMAND_STORE: {
        // 1. SANDBOX CHECK: Ensure POINT is within RAM boundaries
        // Prevent Taleä from reading/writing outside its own memory
        u32 addr = h->point << 9; // scale this depending on sector size
        if (addr > (TALEA_MAIN_MEM_SZ - TALEA_SECTOR_SIZE) ||
            !(h->data < h->header.banks)) {
            // check also if the hcs has that many banks
            h->real_status |= STOR_STATUS_ERROR;
            h->statusL = h->real_status; // update the status visible from the
                                         // machine side (TODO: document that
                                         // calling LOAD or STORE also calls
                                         // GET_STATUS)
            return;
        }

        // 2. Prepare the request object for the thread
        HcsRequest hcs_request = (HcsRequest){
            .command         = command,
            .sector          = ((u32)h->data << 16) | h->sector,
            .dest_ram_ptr    = &m->main_memory[addr],
            .hardware_status = &h->real_status,
        };

        // 3. Set Hardware to BUSY
        h->real_status |= STOR_STATUS_BUSY;
        h->real_status &= ~STOR_STATUS_READY;
        h->statusL = h->real_status; // update the status visible from the
                                     // machine side (TODO: document that
                                     // calling LOAD or STORE also calls
                                     // GET_STATUS)

        // 4. Start Thread
        // POSIX: pthread_create
        // Windows: CreateThread or _beginthreadex
        // C11: thrd_create
        // Start_Async_Worker(&tps_request);
        thrd_t worker;
        thrd_create(&worker, Hcs_WorkerThread, &hcs_request);
        thrd_detach(worker);
        h->statusL = h->real_status;
    }
    default:
        // command not recognized
        h->real_status |= STOR_STATUS_ERROR;
        h->statusL = h->real_status; // update the status visible from the
                                     // machine side (TODO: document that
                                     // calling LOAD or STORE also calls
                                     // GET_STATUS)
        break;
    }
}

u8 Storage_ReadHandler(TaleaMachine *m, u16 addr)
{
    int tps_lock_status = mtx_trylock(&tps_mutex);
    int hcs_lock_status = mtx_trylock(&hcs_mutex);
    u8  value           = 0;

    switch (addr) {
    // TPS
    case P_TPS_COMMAND:
        value = 0xff;
    case P_TPS_DATA: {
        if (tps_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.current_tps->data;
    }
    case P_TPS_POINTH: {
        if (tps_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.current_tps->point >> 8;
    }
    case P_TPS_POINTL: {
        if (tps_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.current_tps->point & 0xff;
    }
    case P_TPS_STATUSH: {
        if (tps_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.current_tps->statusH;
    }
    case P_TPS_STATUSL: {
        if (tps_lock_status != thrd_success)
            value = STOR_STATUS_BUSY;
        else
            value = m->storage.current_tps->statusL;
    }
    // HCS
    case P_HCS_COMMAND:
        value = 0xff;
    case P_HCS_DATA: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.data;
    }
    case P_HCS_SECTORH: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.sector >> 8;
    }
    case P_HCS_SECTORL: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.sector & 0xff;
    }
    case P_HCS_POINTH: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.point >> 8;
    }
    case P_HCS_POINTL: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.point & 0xff;
    }
    case P_HCS_STATUSH: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.statusH;
    }
    case P_HCS_STATUSL: {
        if (hcs_lock_status != thrd_success)
            value = STOR_STATUS_BUSY;
        else
            value = m->storage.hcs.statusL;
    }

    default:
        TALEA_LOG_WARNING("Reading Storage from malformed addr: %x\n", addr);
        return 0xff;
    }

    if (tps_lock_status == thrd_success) mtx_unlock(&tps_mutex);
    if (hcs_lock_status == thrd_success) mtx_unlock(&hcs_mutex);
    return value;
}

void Storage_WriteHandler(TaleaMachine *m, u16 addr, u8 value)
{
    int tps_lock_status = mtx_trylock(&tps_mutex);
    int hcs_lock_status = mtx_trylock(&hcs_mutex);

    switch (addr) {
    // TPS
    case P_TPS_COMMAND: {
        if (tps_lock_status != thrd_success) break;
        Tps_ProcessCommand(m, value);
        break;
    }
    case P_TPS_DATA: {
        if (tps_lock_status != thrd_success) break;
        m->storage.current_tps->data = value;
        break;
    }
    case P_TPS_POINTH: {
        if (tps_lock_status != thrd_success) break;
        m->storage.current_tps->point |= (u16)value << 8;
        break; // TODO: proper sector alignment based on sector_size
    }
    case P_TPS_POINTL: {
        if (tps_lock_status != thrd_success) break;
        m->storage.current_tps->point |= value;
        break;
    }
    case P_TPS_STATUSH:
        break; // non writable register
    case P_TPS_STATUSL:
        break; // non writable register

    // HCS
    case P_HCS_COMMAND: {
        if (hcs_lock_status != thrd_success) break;
        Hcs_ProcessCommand(m, value);
        break;
    }
    case P_HCS_DATA: {
        if (hcs_lock_status != thrd_success) break;
        m->storage.hcs.data = value;
        break;
    }
    case P_HCS_SECTORH: {
        if (hcs_lock_status != thrd_success) break;
        m->storage.hcs.sector |= (u16)value << 8;
        break;
    }
    case P_HCS_SECTORL: {
        if (hcs_lock_status != thrd_success) break;
        m->storage.hcs.sector |= value;
        break;
    }
    case P_HCS_POINTH: {
        if (hcs_lock_status != thrd_success) break;
        m->storage.hcs.point |= (u16)value << 8;
        break; // TODO: proper sector alignment based on sector_size
    }
    case P_HCS_POINTL: {
        if (hcs_lock_status != thrd_success) break;
        m->storage.hcs.point |= value;
        break;
    }
    case P_HCS_STATUSH:
        break; // non writable register
    case P_HCS_STATUSL:
        break; // non writable register

    default:
        TALEA_LOG_WARNING("Wrtiting Storage to malformed addr: %x\n", addr);
    }

    if (tps_lock_status == thrd_success) mtx_unlock(&tps_mutex);
    if (hcs_lock_status == thrd_success) mtx_unlock(&hcs_mutex);
}

static bool Hcs_Init(Hcs *hcs, const char *path)
{
    // No need to lock, inits before cpu starts running
    hcs->file = fopen(path, "rb+");
    if (!hcs->file) {
        // TODO: If file doesn't exist, you could create a blank  one here
        TALEA_LOG_ERROR(
            "HCS Error: Could not locate Hyper Crystal Matrix at %s.\n", path);
        return false;
    }

    // 2. Validate the size (Lore: 128MB max)
    fseek(hcs->file, 0, SEEK_END);
    long file_size = ftell(hcs->file);
    rewind(hcs->file);

    if (file_size < 512) {
        fclose(hcs->file);
        TALEA_LOG_ERROR("HCS Error: Matrix is too small (corrupt header).\n");
        return false;
    }

    if (!Hcs_ParseHeader(hcs)) {
        fclose(hcs->file);
        TALEA_LOG_ERROR("HCS Error: Invalid signature in obsidian slab.\n");
        return false;
    }

    // 4. Set initial Hardware State
    hcs->real_status |= STOR_STATUS_READY;
    hcs->statusL = hcs->real_status;

    TALEA_LOG_TRACE("HCS Initialized: %u sectors available in the Archive.\n",
                    hcs->header.sector_count);
    return true;
}

void Storage_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart)
{
    m->storage.current_tps    = &m->storage.tps_drives[TPS_ID_A];
    m->storage.current_tps_id = TPS_ID_A;
    if (!is_restart) {
        if (!Hcs_Init(&m->storage.hcs, HCS_FILE_PATH)) {
            TALEA_LOG_ERROR("Failed to initialize hcs. Panic!\n");
            exit(1); // TODO: same as below
        }

        int success = mtx_init(&tps_mutex, mtx_plain | mtx_recursive);
        if (success != thrd_success) {
            perror("Error initializing mutex - tps");
            exit(1); // TODO: handle this gracefully, but this is a panic
        }

        success = mtx_init(&hcs_mutex, mtx_plain | mtx_recursive);
        if (success != thrd_success) {
            perror("Error initializing mutex - hcs");
            exit(1); // TODO: handle this gracefully, but this is a panic
        }
    }

    TALEA_LOG_WARNING("Should reset to a known state\n");
}

void Storage_Deinit(TaleaMachine *m)
{
    mtx_destroy(&tps_mutex);
    mtx_destroy(&hcs_mutex);

    fclose(m->storage.hcs.file);
}