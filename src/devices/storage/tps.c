#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/bus.h"
#include "talea.h"
#include "storage.h"

/*----------------------------------------------------------------------------*/
/* STATIC GLOBALS                                                             */
/*----------------------------------------------------------------------------*/

mtx_t tpsMutex;

/*----------------------------------------------------------------------------*/
/* INTERNAL DEVICE FUNCIONS: declare as static inline                         */
/*----------------------------------------------------------------------------*/

static int Tps_WorkerThread(void *req)
{
    extern StorageTps *TpsDrives;

    StorageTpsRequest *request = (StorageTpsRequest *)req;
    // 1. Calculate the absolute byte offset
    // LBA = (Bank << 8) | Sector
    // Host Offset = (LBA * 512) + 512 (for the TPS Header)
    u32 lba        = (request->bank << 8) | request->sector;
    u32 hostOffset = (lba * STORAGE_SECTOR_SIZE) + STORAGE_HEADER_SIZE;
    // TODO: do not harcode the sector size

    /* CRITICAL SECTION, LOCK MUTEX */
    mtx_lock(&tpsMutex);

    StorageTps *t = &TpsDrives[request->id];

    // 2. Seek to the position
    if (fseek(t->file, hostOffset, SEEK_SET) != 0) {
        atomic_fetch_or(request->driveStatus, STORAGE_STATUS_ERROR);
        atomic_fetch_and(request->driveStatus, ~STORAGE_STATUS_BUSY);
        mtx_unlock(&tpsMutex); // RELEASE MUTEX
        free(request);
        return -1;
    }

    if (request->command == STORAGE_COMMAND_LOAD) {
        // Disk -> DATA
        fread(request->ptr, 1, STORAGE_SECTOR_SIZE, t->file);
    }

    else if (request->command == STORAGE_COMMAND_STORE) {
        // DATA -> Disk
        fwrite(request->ptr, 1, STORAGE_SECTOR_SIZE, t->file);
        fflush(t->file); // Ensure Host OS flushes buffers

#ifdef _WIN32
        _commit(_fileno(t->file));
#else
        fsync(fileno(t->file));
#endif
    }

    // 3. Signal completion
    atomic_fetch_or(request->driveStatus, STORAGE_STATUS_DONE);
    atomic_fetch_or(request->driveStatus, STORAGE_STATUS_READY);
    atomic_fetch_and(request->driveStatus, ~STORAGE_STATUS_BUSY);

    mtx_unlock(&tpsMutex); // RELEASE MUTEX
    free(request);

    return 0;
}

static bool Tps_ParseHeader(StorageTps *tps)
{
    // HAS MUTEX LOCK
    u8 buff[STORAGE_HEADER_SIZE];
    fseek(tps->file, 0, SEEK_SET);
    fread(buff, 1, 512, tps->file);

    struct StorageHeader *h = &tps->header;
    Storage_ParseHeader(buff, h);

    if (memcmp(h->magic, STORAGE_HEADER_TPS_MAGIC, 4) != 0) {
        return false;
    }

    tps->writeProtected = h->writeProtected;
    tps->status         = (h->writeProtected ? STORAGE_STATUS_WPROT : 0) |
                  (h->bootable ? STORAGE_STATUS_BOOT : 0);
    return true;
}

void Tps_ProcessCommand(TaleaMachine *m, u8 command)
{
    // HAS THE MUTEX LOCKED
    extern StorageTps *TpsDrives;
    StorageTps        *t = &TpsDrives[m->storage.currentTpsId];

    atomic_fetch_and(&m->storage.currentTps->status,
                     ~(STORAGE_STATUS_BUSY | STORAGE_STATUS_ERROR | STORAGE_STATUS_DONE));

    switch (command) {
    case STORAGE_COMMAND_NOP: break;
    case STORAGE_COMMAND_MEDIUM: {
        if (t->inserted)
            t->result = t->header.mediumType;
        else
            t->result = STORAGE_NO_MEDIA;
        break;
    }
    case STORAGE_COMMAND_GETBANK: t->result = t->bank; break;
    case STORAGE_COMMAND_GETCURRENT: t->result = m->storage.currentTpsId; break;
    case STORAGE_COMMAND_BANK:
        if (t->data < t->header.banks) {
            t->bank = t->data;
        } else {
            atomic_fetch_or(&t->status, STORAGE_STATUS_ERROR);
            return;
        }
        break;
    case STORAGE_COMMAND_SETCURRENT:
        if (t->data >= STORAGE_TPS_TOTAL_DRIVES) {
            // that drive does not exist
            TALEA_LOG_TRACE("Requested Tps drive (%d) does not exist\n", t->data);
            atomic_fetch_or(&t->status, STORAGE_STATUS_ERROR);
            return;
        }
        m->storage.currentTps   = &m->storage.tpsDrives[t->data];
        m->storage.currentTpsId = t->data;
        break;
    case STORAGE_COMMAND_LOAD:
    case STORAGE_COMMAND_STORE: {
        TALEA_LOG_TRACE("Written Tps load or store!\n");
        if (!t->inserted) return;
        if (command == STORAGE_COMMAND_STORE && t->writeProtected) {
            atomic_fetch_or(&t->status, STORAGE_STATUS_ERROR | STORAGE_STATUS_WPROT);
            TALEA_LOG_TRACE("ERROR TPS %d WRIRTE PROTECTED\n", t->id);
            return;
        }
        // 1. SANDBOX CHECK: Ensure POINT is within RAM boundaries
        // Prevent Taleä from reading/writing outside its own memory
        u16 addr         = t->point << 9; // TODO: scale this depending on sector size
        u8 *data_pointer = Bus_GetDataPointer(m, addr, STORAGE_SECTOR_SIZE);
        if (!data_pointer || (t->bank >= t->header.banks)) {
            atomic_fetch_or(&t->status, STORAGE_STATUS_ERROR);

            TALEA_LOG_TRACE(
                "TPS ERROR address %08x not suitable for sector load/store (bank %d, header banks %d)\n",
                t->id, t->bank, t->header.banks);
            return;
        }

        int access = command == STORAGE_COMMAND_LOAD ? 1 : 0;

        // 2. Prepare the request object for the thread
        StorageTpsRequest *tpsRequest = malloc(sizeof(StorageTpsRequest));

        tpsRequest->command     = command;
        tpsRequest->bank        = t->bank;
        tpsRequest->sector      = t->data;
        tpsRequest->ptr         = data_pointer;
        tpsRequest->id          = m->storage.currentTpsId;
        tpsRequest->driveStatus = &t->status;

        // 3. Set Hardware to BUSY
        atomic_fetch_or(&t->status, STORAGE_STATUS_BUSY);
        atomic_fetch_and(&t->status, ~STORAGE_STATUS_READY);

        // 4. Start Thread
        // POSIX: pthread_create
        // Windows: CreateThread or _beginthreadex
        // C11: thrd_create
        // Start_Async_Worker(&tpsRequest);
        thrd_t worker;
        thrd_create(&worker, Tps_WorkerThread, tpsRequest);
        thrd_detach(worker);

        break;
    }
    default:
        // command not recognized
        atomic_fetch_or(&t->status, STORAGE_STATUS_ERROR);
        break;
    }
}

/*----------------------------------------------------------------------------*/
/* INTERFACE IMPLEMENTATION                                                   */
/*----------------------------------------------------------------------------*/

// Callbacks for Raylib's AudioStream

bool Storage_InsertTps(enum TpsId tps_id, const char *path)
{
    extern StorageTps *TpsDrives;
    if (mtx_lock(&tpsMutex) == thrd_success) {
        StorageTps *t = &TpsDrives[tps_id];

        if (t->inserted) {
            mtx_unlock(&tpsMutex);
            TALEA_LOG_TRACE("There's a tps already inserted in drive %d, eject it first!\n",
                            tps_id);
            return false;
        }

        t->file = fopen(path, "rb+");
        if (!t->file) {
            mtx_unlock(&tpsMutex);
            TALEA_LOG_TRACE("Tps file not found, or could not be opened!\n");
            return false;
        }

        if (!Tps_ParseHeader(t)) {
            fclose(t->file);
            mtx_unlock(&tpsMutex);
            TALEA_LOG_TRACE("Tps file header could not be parsed!\n");
            return false;
        }

        t->inserted     = true;
        t->justInserted = true;
        atomic_fetch_or(&t->status, STORAGE_STATUS_INSERTED | STORAGE_STATUS_READY);

        mtx_unlock(&tpsMutex);

        return true;
    }

    TALEA_LOG_ERROR("Error inserting TPS: error acquiring lock\n");
    return false;
}

bool Storage_EjectTps(enum TpsId tps_id)
{
    extern StorageTps *TpsDrives;
    if (mtx_lock(&tpsMutex) == thrd_success) {
        StorageTps *t = &TpsDrives[tps_id];

        if (!t->inserted) {
            mtx_unlock(&tpsMutex);
            TALEA_LOG_TRACE("Cannot eject tps %d if it was not inserted!\n", tps_id);
            return false;
        }

        fclose(t->file);
        t->file        = NULL;
        t->inserted    = false;
        t->justEjected = true;
        atomic_fetch_and(&t->status, ~STORAGE_STATUS_INSERTED);
        mtx_unlock(&tpsMutex);
        return true;
    }

    TALEA_LOG_ERROR("Error ejecting TPS: error acquiring lock\n");
    return false;
}

// DEVICE INITIALIZATION

// Handled by storage.c

// DEVICE UPDATE (on vblank)
// None

// DEVICE UPDATE (on cpu tick)
// Yes, // TODO: update on tick

// DEVICE PORT IO HANDLERS

// Handled by storage.c