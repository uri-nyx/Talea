#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h> // C11 or more

#include "core/bus.h"
#include "talea.h"

// TODO: modify to not do DMA

// PORTS

#define P_HCS_COMMAND (DEV_DISK_BASE + 0)
#define P_HCS_DATA    (DEV_DISK_BASE + 1)
#define P_HCS_SECTORH (DEV_DISK_BASE + 2)
#define P_HCS_SECTORL (DEV_DISK_BASE + 3)
#define P_HCS_POINTH  (DEV_DISK_BASE + 4)
#define P_HCS_POINTL  (DEV_DISK_BASE + 5)
#define P_HCS_RESULT  (DEV_DISK_BASE + 6)
#define P_HCS_STATUS  (DEV_DISK_BASE + 7)

#define P_TPS_COMMAND (DEV_TPS_BASE + 0)
#define P_TPS_DATA    (DEV_TPS_BASE + 1)
#define P_TPS_POINTH  (DEV_TPS_BASE + 2)
#define P_TPS_POINTL  (DEV_TPS_BASE + 3)
#define P_TPS_RESULT  (DEV_TPS_BASE + 4)
#define P_TPS_STATUS  (DEV_TPS_BASE + 5)

enum StorageCommand {
    COMMAND_NOP,        // does nothing
    COMMAND_STORE,      // stores to sector DATA of current bank from POINT_H:POINT_L
    COMMAND_LOAD,       // loads from sector DATA of current bank in POINT_H:POINT_L
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
    atomic_uchar *hardware_status;

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
    atomic_uchar *hardware_status;

    // --- Security Check ---
    u32 max_lba; // Derived from header; used to clamp access
} HcsRequest;

static mtx_t tps_mutex;
static int   Tps_WorkerThread(void *req)
{
    extern Tps *TpsDrives;

    TpsRequest *request = (TpsRequest *)req;
    // 1. Calculate the absolute byte offset
    // LBA = (Bank << 8) | Sector
    // Host Offset = (LBA * 512) + 512 (for the TPS Header)
    u32 lba         = (request->bank << 8) | request->sector;
    u32 host_offset = (lba * TALEA_SECTOR_SIZE) + STOR_HEADER_SIZE; // TODO: do not harcode the
                                                                    // sector size

    /* CRITICAL SECTION, LOCK MUTEX */
    mtx_lock(&tps_mutex);

    Tps *t = &TpsDrives[request->id];

    // 2. Seek to the position
    if (fseek(t->file, host_offset, SEEK_SET) != 0) {
        atomic_fetch_or(request->hardware_status, STOR_STATUS_ERROR);
        atomic_fetch_and(request->hardware_status, ~STOR_STATUS_BUSY);
        mtx_unlock(&tps_mutex); // RELEASE MUTEX
        free(request);
        return -1;
    }

    if (request->command == COMMAND_LOAD) {
        // Disk -> RAM
        // TODO: think if doing DMA is the right thing. Maybe loading the sectors to a configurable
        // location in DATA memory is easier, and then, the kernel can copy it wherever in the
        // virtual address space
        fread(request->dest_ram_ptr, 1, TALEA_SECTOR_SIZE, t->file);
    }

    else if (request->command == COMMAND_STORE) {
        // RAM -> Disk
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
    atomic_fetch_or(request->hardware_status, STOR_STATUS_DONE);
    atomic_fetch_or(request->hardware_status, STOR_STATUS_READY);
    atomic_fetch_and(request->hardware_status, ~STOR_STATUS_BUSY);

    mtx_unlock(&tps_mutex); // RELEASE MUTEX
    free(request);

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

static void Storage_ParseHeader(u8 in_buff[STOR_HEADER_SIZE], struct StorageHeader *out)
{
    memcpy(out->magic, in_buff, 4);
    out->version         = in_buff[HEAD_VERSION];
    u8 flags             = in_buff[HEAD_FLAGS];
    out->bootable        = flags & HEAD_FLAG_BOOT;
    out->write_protected = flags & HEAD_FLAG_WPROT;
    out->medium_type     = in_buff[HEAD_MEDIUM];
    out->banks           = in_buff[HEAD_BANKS];
    out->sector_count    = (u32)in_buff[HEAD_SECTORS] << 24 | (u32)in_buff[HEAD_SECTORS + 1] << 16 |
                        (u32)in_buff[HEAD_SECTORS + 2] << 8 | (u32)in_buff[HEAD_SECTORS + 3];
    out->sector_size = (u16)in_buff[HEAD_SECTORSZ] << 8 | (u16)in_buff[HEAD_SECTORSZ + 1];
    out->creation_date =
        (u64)in_buff[HEAD_DATECREATION] << 56 | (u64)in_buff[HEAD_DATECREATION + 1] << 48 |
        (u64)in_buff[HEAD_DATECREATION + 2] << 40 | (u64)in_buff[HEAD_DATECREATION + 3] << 32 |
        (u64)in_buff[HEAD_DATECREATION + 4] << 24 | (u64)in_buff[HEAD_DATECREATION + 5] << 16 |
        (u64)in_buff[HEAD_DATECREATION + 6] << 8 | (u64)in_buff[HEAD_DATECREATION + 7];
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
    tps->status         = (h->write_protected ? STOR_STATUS_WPROT : 0) |
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

    hcs->status = (h->write_protected ? STOR_STATUS_WPROT : 0) |
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
            TALEA_LOG_TRACE("There's a tps already inserted in drive %d, eject it first!\n",
                            tps_id);
            return false;
        }

        t->file = fopen(path, "rb+");
        if (!t->file) {
            mtx_unlock(&tps_mutex);
            TALEA_LOG_TRACE("Tps file not found, or could not be opened!\n");
            return false;
        }

        if (!Tps_ParseHeader(t)) {
            fclose(t->file);
            mtx_unlock(&tps_mutex);
            TALEA_LOG_TRACE("Tps file header could not be parsed!\n");
            return false;
        }

        t->inserted      = true;
        t->just_inserted = true;
        atomic_fetch_or(&t->status, STOR_STATUS_INSERTED | STOR_STATUS_READY);

        mtx_unlock(&tps_mutex);

        return true;
    }

    TALEA_LOG_ERROR("Error inserting TPS: error acquiring lock\n");
    return false;
}

bool Storage_EjectTps(enum TpsId tps_id)
{
    extern Tps *TpsDrives;
    if (mtx_lock(&tps_mutex) == thrd_success) {
        Tps *t = &TpsDrives[tps_id];

        if (!t->inserted) {
            mtx_unlock(&tps_mutex);
            TALEA_LOG_TRACE("Cannot eject tps %d if it was not inserted!\n", tps_id);
            return false;
        }

        fclose(t->file);
        t->file         = NULL;
        t->inserted     = false;
        t->just_ejected = true;
        atomic_fetch_and(&t->status, ~STOR_STATUS_INSERTED);
        mtx_unlock(&tps_mutex);
        return true;
    }

    TALEA_LOG_ERROR("Error ejecting TPS: error acquiring lock\n");
    return false;
}

static void Tps_ProcessCommand(TaleaMachine *m, u8 command)
{
    // HAS THE MUTEX LOCKED
    extern Tps *TpsDrives;
    Tps        *t = &TpsDrives[m->storage.current_tps_id];

    atomic_fetch_and(&m->storage.current_tps->status, ~(STOR_STATUS_BUSY | STOR_STATUS_ERROR | STOR_STATUS_DONE));

    switch (command) {
    case COMMAND_NOP: break;
    // case COMMAND_GET_STATUS: t->statusL = atomic_load(&t->status); break;
    case COMMAND_MEDIUM: {
        if (t->inserted)
            t->result = t->header.medium_type;
        else
            t->result = NoMedia;
        break;
    }
    case COMMAND_GETBANK: t->result = t->bank; break;
    case COMMAND_GETCURRENT: t->result = m->storage.current_tps_id; break;
    case COMMAND_BANK:
        if (t->data < t->header.banks) {
            t->bank = t->data;
        } else {
            atomic_fetch_or(&t->status, STOR_STATUS_ERROR);
            return;
        }
        break;
    case COMMAND_SETCURRENT:
        if (t->data >= TPS_TOTAL_DRIVES) {
            // that drive does not exist
            TALEA_LOG_TRACE("Requested Tps drive (%d) does not exist\n", t->data);
            atomic_fetch_or(&t->status, STOR_STATUS_ERROR);
            return;
        }
        m->storage.current_tps    = &m->storage.tps_drives[t->data];
        m->storage.current_tps_id = t->data;
        break;
    case COMMAND_LOAD:
    case COMMAND_STORE: {
        TALEA_LOG_TRACE("Written Tps load or store!\n");
        if (!t->inserted) return;
        if (command == COMMAND_STORE && t->writeProtected) {
            atomic_fetch_or(&t->status, STOR_STATUS_ERROR | STOR_STATUS_WPROT);
            TALEA_LOG_TRACE("ERROR TPS %d WRIRTE PROTECTED\n", t->id);
            return;
        }
        // 1. SANDBOX CHECK: Ensure POINT is within RAM boundaries
        // Prevent Taleä from reading/writing outside its own memory
        u16 addr = t->point << 9; // TODO: scale this depending on sector size
        if (addr > (TALEA_DATA_MEM_SZ - TALEA_SECTOR_SIZE) || (t->bank >= t->header.banks)) {
            atomic_fetch_or(&t->status, STOR_STATUS_ERROR);

            TALEA_LOG_TRACE(
                "ERROR address %08x not suitable for sector load/store (bank %d, header banks %d)\n",
                t->id, t->bank, t->header.banks);
            return;
        }

        int access = command == COMMAND_LOAD ? 1 : 0;

        // 2. Prepare the request object for the thread
        TpsRequest *tps_request = malloc(sizeof(TpsRequest));

        tps_request->command         = command;
        tps_request->bank            = t->bank;
        tps_request->sector          = t->data;
        tps_request->dest_ram_ptr    = &m->data_memory[addr];
        tps_request->id              = m->storage.current_tps_id;
        tps_request->hardware_status = &t->status;

#if TALEA_WITH_MMU
        if (m->cpu.status & 0x20000000) {
            tps_request->dest_ram_ptr = &m->data_memory[addr];
           // ON_FAULT_RETURN_M
        }
#endif

        // 3. Set Hardware to BUSY
        atomic_fetch_or(&t->status, STOR_STATUS_BUSY);
        atomic_fetch_and(&t->status, ~STOR_STATUS_READY);

        // 4. Start Thread
        // POSIX: pthread_create
        // Windows: CreateThread or _beginthreadex
        // C11: thrd_create
        // Start_Async_Worker(&tps_request);
        thrd_t worker;
        thrd_create(&worker, Tps_WorkerThread, tps_request);
        thrd_detach(worker);

        break;
    }
    default:
        // command not recognized
        atomic_fetch_or(&t->status, STOR_STATUS_ERROR);
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
    u32 host_offset = (request->sector * TALEA_SECTOR_SIZE) + STOR_HEADER_SIZE;
    // TODO: do not harcode the sector size

    /* CRITICAL SECTION, LOCK MUTEX */
    mtx_lock(&hcs_mutex);

    Hcs *h = HcsDrive;

    // 2. Seek to the position
    if (fseek(h->file, host_offset, SEEK_SET) != 0) {
        atomic_fetch_or(request->hardware_status, STOR_STATUS_ERROR);
        atomic_fetch_and(request->hardware_status, ~STOR_STATUS_BUSY);
        mtx_unlock(&hcs_mutex);
        free(request);
        return -1;
    }

    if (request->command == COMMAND_LOAD) {
        // Disk -> RAM
        fread(request->dest_ram_ptr, 1, TALEA_SECTOR_SIZE, h->file);
    } else if (request->command == COMMAND_STORE) {
        // RAM -> Disk
        fwrite(request->dest_ram_ptr, 1, TALEA_SECTOR_SIZE, h->file);
        fflush(h->file); // Ensure Host OS flushes buffers

// Optional: Hard sync to physical platter
#ifdef _WIN32
        _commit(_fileno(h->file));
#else
        fsync(fileno(h->file));
#endif
    }

    // 3. Signal completion to the Taleä CPU
    atomic_fetch_or(request->hardware_status, STOR_STATUS_DONE);
    atomic_fetch_or(request->hardware_status, STOR_STATUS_READY);
    atomic_fetch_and(request->hardware_status, ~STOR_STATUS_BUSY);

    mtx_unlock(&hcs_mutex);
    free(request);
    return 0;
}

static void Hcs_ProcessCommand(TaleaMachine *m, u8 command)
{
    // HAS THE MUTEX LOCKED
    Hcs *h = &m->storage.hcs;

    atomic_fetch_and(&m->storage.hcs.status, ~(STOR_STATUS_BUSY | STOR_STATUS_ERROR | STOR_STATUS_DONE));

    switch (command) {
    case COMMAND_NOP: break;
    // case COMMAND_GET_STATUS: h->statusL = h->status; break;
    case COMMAND_MEDIUM: h->result = h->header.medium_type; break;
    case COMMAND_LOAD:
    case COMMAND_STORE: {
        // 1. SANDBOX CHECK: Ensure POINT is within RAM boundaries
        // Prevent Taleä from reading/writing outside its own memory
        u32 addr = h->point << 9; // scale this depending on sector size
        if (addr > (TALEA_MAIN_MEM_SZ - TALEA_SECTOR_SIZE) || !(h->data < h->header.banks)) {
            // check also if the hcs has that many banks
            atomic_fetch_or(&h->status, STOR_STATUS_ERROR);

            return;
        }

        int access = command == COMMAND_LOAD ? 1 : 0; // TODO: use names

        // 2. Prepare the request object for the thread
        HcsRequest *hcs_request = malloc(sizeof(HcsRequest));

        hcs_request->command         = command;
        hcs_request->sector          = ((u32)h->data << 16) | h->sector;
        hcs_request->dest_ram_ptr    = &m->main_memory[addr];
        hcs_request->hardware_status = &h->status;

        h->data = 0;

#if TALEA_WITH_MMU
        if (m->cpu.status & 0x20000000) { // IF MMU enableds
            hcs_request->dest_ram_ptr = &m->main_memory[MMU_TranslateAddr(m, addr, access)];
            ON_FAULT_RETURN_M
        }
#endif

        // 3. Set Hardware to BUSY
        atomic_fetch_or(&h->status, STOR_STATUS_BUSY);
        atomic_fetch_and(&h->status, ~STOR_STATUS_READY);

        // 4. Start Thread
        // POSIX: pthread_create
        // Windows: CreateThread or _beginthreadex
        // C11: thrd_create
        // Start_Async_Worker(&tps_request);
        thrd_t worker;
        thrd_create(&worker, Hcs_WorkerThread, hcs_request);
        thrd_detach(worker);
        break;
    }
    default:
        // command not recognized
        atomic_fetch_or(&h->status, STOR_STATUS_ERROR);
        break;
    }
}

u8 Storage_ReadHandler(TaleaMachine *m, u16 addr)
{
    int tps_lock_status = mtx_lock(&tps_mutex);
    int hcs_lock_status = mtx_lock(&hcs_mutex);
    u8  value           = 0;

    if ((tps_lock_status != thrd_success) && (addr <= P_TPS_COMMAND || addr <= P_TPS_STATUS)) {
        atomic_fetch_or(&m->storage.current_tps->status, STOR_STATUS_ERROR);
    } else if ((hcs_lock_status != thrd_success) &&
               (addr <= P_HCS_COMMAND || addr <= P_HCS_STATUS)) {
        atomic_fetch_or(&m->storage.hcs.status, STOR_STATUS_ERROR);
    }

    switch (addr) {
    // TPS
    case P_TPS_COMMAND: value = 0xff; break;
    case P_TPS_DATA: {
        if (tps_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.current_tps->data;
    } break;

    case P_TPS_POINTH: {
        if (tps_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.current_tps->point >> 8;
    } break;

    case P_TPS_POINTL: {
        if (tps_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.current_tps->point & 0xff;
    } break;

    case P_TPS_RESULT: {
        if (tps_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.current_tps->result;
    } break;

    case P_TPS_STATUS: {
        // even if we did not acquire the lock this is safe, because it is atomic
        value = atomic_load(&m->storage.current_tps->status);
    } break;

    // HCS
    case P_HCS_COMMAND: value = 0xff; break;

    case P_HCS_DATA: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.data;
    } break;

    case P_HCS_SECTORH: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.sector >> 8;
    } break;

    case P_HCS_SECTORL: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.sector & 0xff;
    } break;

    case P_HCS_POINTH: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.point >> 8;
    } break;

    case P_HCS_POINTL: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.point & 0xff;
    } break;

    case P_HCS_RESULT: {
        if (hcs_lock_status != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.result;
    } break;

    case P_HCS_STATUS: {
        if (hcs_lock_status != thrd_success)
            value = STOR_STATUS_BUSY;
        else
            value = atomic_load(&m->storage.hcs.status);
    } break;

    default: TALEA_LOG_WARNING("Reading Storage from malformed addr: %x\n", addr); return 0xff;
    }

    if (tps_lock_status == thrd_success) mtx_unlock(&tps_mutex);
    if (hcs_lock_status == thrd_success) mtx_unlock(&hcs_mutex);
    return value;
}

void Storage_WriteHandler(TaleaMachine *m, u16 addr, u8 value)
{
    int tps_lock_status = mtx_lock(&tps_mutex);
    int hcs_lock_status = mtx_lock(&hcs_mutex);

    switch (addr) {
    // TPS
    case P_TPS_COMMAND: {
        if (tps_lock_status != thrd_success) {
            atomic_fetch_or(&m->storage.current_tps->status, STOR_STATUS_ERROR);
            break;
        }
        Tps_ProcessCommand(m, value);
        break;
    }
    case P_TPS_DATA: {
        if (tps_lock_status != thrd_success) {
            atomic_fetch_or(&m->storage.current_tps->status, STOR_STATUS_ERROR);
            break;
        }
        m->storage.current_tps->data = value;
        break;
    }
    case P_TPS_POINTH: {
        if (tps_lock_status != thrd_success) {
            atomic_fetch_or(&m->storage.current_tps->status, STOR_STATUS_ERROR);
            break;
        };
        TALEA_LOG_TRACE("Pointh: %02x\n", value);
        m->storage.current_tps->point = (m->storage.current_tps->point & 0x00ff) | (u16)value << 8;
        break; // TODO: proper sector alignment based on sector_size
    }
    case P_TPS_POINTL: {
        if (tps_lock_status != thrd_success) {
            atomic_fetch_or(&m->storage.current_tps->status, STOR_STATUS_ERROR);
            break;
        };
        TALEA_LOG_TRACE("Pointl: %02x\n", value);
        m->storage.current_tps->point = (m->storage.current_tps->point & 0xff00) | value;
        break;
    }
    case P_TPS_RESULT: break; // non writable register
    case P_TPS_STATUS:
        break; // non writable register

    // HCS
    case P_HCS_COMMAND: {
        if (hcs_lock_status != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STOR_STATUS_ERROR);
            break;
        }
        Hcs_ProcessCommand(m, value);
        break;
    }
    case P_HCS_DATA: {
        if (hcs_lock_status != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STOR_STATUS_ERROR);
            break;
        }
        m->storage.hcs.data = value;
        break;
    }
    case P_HCS_SECTORH: {
        if (hcs_lock_status != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STOR_STATUS_ERROR);
            break;
        }
        m->storage.hcs.sector = (m->storage.hcs.sector & 0x00ff) | (u16)value << 8;
        break;
    }
    case P_HCS_SECTORL: {
        if (hcs_lock_status != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STOR_STATUS_ERROR);
            break;
        }
        m->storage.hcs.sector = (m->storage.hcs.sector & 0xff00) | value;
        break;
    }
    case P_HCS_POINTH: {
        if (hcs_lock_status != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STOR_STATUS_ERROR);
            break;
        }
        m->storage.hcs.point = (m->storage.hcs.point & 0x00ff) | (u16)value << 8;
        break; // TODO: proper sector alignment based on sector_size
    }
    case P_HCS_POINTL: {
        if (hcs_lock_status != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STOR_STATUS_ERROR);
            break;
        }
        m->storage.hcs.point = (m->storage.hcs.point & 0xff00) | value;
        break;
    }
    case P_HCS_RESULT: break; // non writable register
    case P_HCS_STATUS: break; // non writable register

    default: TALEA_LOG_WARNING("Wrtiting Storage to malformed addr: %x\n", addr);
    }

    if (tps_lock_status == thrd_success) mtx_unlock(&tps_mutex);
    if (hcs_lock_status == thrd_success) mtx_unlock(&hcs_mutex);
}

static bool Hcs_Init(Hcs *hcs, const char *path)
{
    // No need to lock, inits before cpu starts running
    hcs->file = fopen(path, "rb+");
    if (!hcs->file) {
        // TODO: If file doesn't exist,  create a blank  one here
        TALEA_LOG_ERROR("HCS Error: hcs disk file not found %s.\n", path);
        return false;
    }

    // 2. Validate the size (Lore: 128MB max)
    fseek(hcs->file, 0, SEEK_END);
    long file_size = ftell(hcs->file);
    rewind(hcs->file);

    if (file_size < 512) {
        fclose(hcs->file);
        TALEA_LOG_ERROR("HCS Error: file size too small, corrupt header.\n");
        return false;
    }

    if (!Hcs_ParseHeader(hcs)) {
        fclose(hcs->file);
        TALEA_LOG_ERROR("HCS Error: invalid header.\n");
        return false;
    }

    // 4. Set initial Hardware State
    atomic_fetch_or(&hcs->status, STOR_STATUS_READY);

    TALEA_LOG_TRACE("HCS Initialized: %u sectors available.\n", hcs->header.sector_count);
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