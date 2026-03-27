#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/bus.h"
#include "talea.h"
#include "storage.h"

#ifndef _WIN32
#include <unistd.h>
#endif

/*----------------------------------------------------------------------------*/
/* STATIC GLOBALS                                                             */
/*----------------------------------------------------------------------------*/

mtx_t hcsMutex;

/*----------------------------------------------------------------------------*/
/* INTERNAL DEVICE FUNCIONS: declare as static inline                         */
/*----------------------------------------------------------------------------*/

static int Hcs_WorkerThread(void *req)
{
    extern StorageHcs *HcsDrive;

    StorageHcsRequest *request = (StorageHcsRequest *)req;
    // 1. Calculate the absolute byte offset
    // LBA = (Bank << 8) | Sector
    // Host Offset = (LBA * 512) + 512 (for the TPS Header)
    u32 hostOffset = (request->sector * STORAGE_SECTOR_SIZE) + STORAGE_HEADER_SIZE;
    // TODO: do not harcode the sector size

    /* CRITICAL SECTION, LOCK MUTEX */
    mtx_lock(&hcsMutex);

    StorageHcs *h = HcsDrive;

    // 2. Seek to the position
    if (fseek(h->file, hostOffset, SEEK_SET) != 0) {
        atomic_fetch_or(request->driveStatus, STORAGE_STATUS_ERROR);
        atomic_fetch_and(request->driveStatus, ~STORAGE_STATUS_BUSY);
        mtx_unlock(&hcsMutex);
        free(request);
        return -1;
    }

    if (request->command == STORAGE_COMMAND_LOAD) {
        // Disk -> RAM
        fread(request->ptr, 1, STORAGE_SECTOR_SIZE, h->file);
    } else if (request->command == STORAGE_COMMAND_STORE) {
        // RAM -> Disk
        fwrite(request->ptr, 1, STORAGE_SECTOR_SIZE, h->file);
        fflush(h->file); // Ensure Host OS flushes buffers

// Optional: Hard sync to physical platter
#ifdef _WIN32
        _commit(_fileno(h->file));
#else
        fsync(fileno(h->file));
#endif
    }

    // 3. Signal completion to the Taleä CPU
    atomic_fetch_or(request->driveStatus, STORAGE_STATUS_DONE);
    atomic_fetch_or(request->driveStatus, STORAGE_STATUS_READY);
    atomic_fetch_and(request->driveStatus, ~STORAGE_STATUS_BUSY);

    mtx_unlock(&hcsMutex);
    free(request);
    return 0;
}

static bool Hcs_ParseHeader(StorageHcs *hcs)
{
    // HAS MUTEX LOCK
    u8 buff[STORAGE_HEADER_SIZE];
    fseek(hcs->file, 0, SEEK_SET);
    fread(buff, 1, 512, hcs->file);

    struct StorageHeader *h = &hcs->header;
    Storage_ParseHeader(buff, h);

    if (memcmp(h->magic, STORAGE_HEADER_HCS_MAGIC, 4) != 0) {
        return false;
    }

    hcs->status = (h->writeProtected ? STORAGE_STATUS_WPROT : 0) |
                  (h->bootable ? STORAGE_STATUS_BOOT : 0);
    return true;
}

void Hcs_ProcessCommand(TaleaMachine *m, u8 command)
{
    // HAS THE MUTEX LOCKED
    StorageHcs *h = &m->storage.hcs;

    atomic_fetch_and(&m->storage.hcs.status,
                     ~(STORAGE_STATUS_BUSY | STORAGE_STATUS_ERROR | STORAGE_STATUS_DONE));

    switch (command) {
    case STORAGE_COMMAND_NOP: break;
    case STORAGE_COMMAND_MEDIUM: h->result = h->header.mediumType; break;
    case STORAGE_COMMAND_LOAD:
    case STORAGE_COMMAND_STORE: {
        // 1. SANDBOX CHECK: Ensure POINT is within RAM boundaries
        // Prevent Taleä from reading/writing outside its own memory
        u16 addr        = h->point << 9; // scale this depending on sector size
        u8 *dataPointer = Bus_GetDataPointer(m, addr, STORAGE_SECTOR_SIZE);

        if (!dataPointer || !(h->data < h->header.banks)) {
            // check also if the hcs has that many banks
            atomic_fetch_or(&h->status, STORAGE_STATUS_ERROR);
            TALEA_LOG_TRACE(
                "HCS ERROR address %08x not suitable for sector load/store (bank %d, header banks %d)\n",
                addr, h->data, h->header.banks);

            return;
        }

        int access = command == STORAGE_COMMAND_LOAD ? 1 : 0; // TODO: use names

        // 2. Prepare the request object for the thread
        StorageHcsRequest *hcsRequest = malloc(sizeof(StorageHcsRequest));

        hcsRequest->command     = command;
        hcsRequest->sector      = ((u32)h->data << 16) | h->sector;
        hcsRequest->ptr         = dataPointer;
        hcsRequest->driveStatus = &h->status;

        h->data = 0;

        // 3. Set Hardware to BUSY
        atomic_fetch_or(&h->status, STORAGE_STATUS_BUSY);
        atomic_fetch_and(&h->status, ~STORAGE_STATUS_READY);

        // 4. Start Thread
        // POSIX: pthread_create
        // Windows: CreateThread or _beginthreadex
        // C11: thrd_create
        // Start_Async_Worker(&tps_request);
        thrd_t worker;
        thrd_create(&worker, Hcs_WorkerThread, hcsRequest);
        thrd_detach(worker);
        break;
    }
    default:
        // command not recognized
        atomic_fetch_or(&h->status, STORAGE_STATUS_ERROR);
        break;
    }
}

/*----------------------------------------------------------------------------*/
/* INTERFACE IMPLEMENTATION                                                   */
/*----------------------------------------------------------------------------*/

// DEVICE INITIALIZATION

bool Hcs_Init(StorageHcs *hcs, const char *path)
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
    atomic_fetch_or(&hcs->status, STORAGE_STATUS_READY);

    TALEA_LOG_TRACE("HCS Initialized: %u sectors available.\n", hcs->header.sectorCount);
    return true;
}

// DEVICE UPDATE (on vblank)
// None

// DEVICE UPDATE (on cpu tick)
// Yes, // TODO: update on tick

// DEVICE PORT IO HANDLERS

// Handled by storage.c
