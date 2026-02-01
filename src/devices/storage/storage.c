#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/bus.h"
#include "talea.h"
#include "storage.h"

extern mtx_t tpsMutex;
extern mtx_t hcsMutex;

/*----------------------------------------------------------------------------*/
/* INTERFACE IMPLEMENTATION                                                   */
/*----------------------------------------------------------------------------*/

// DEVICE INITIALIZATION

void Storage_Reset(TaleaMachine *m, TaleaConfig *config, bool is_restart)
{
    extern bool Hcs_Init(StorageHcs * hcs, const char *path);
    
    m->storage.currentTps   = &m->storage.tpsDrives[STORAGE_TPS_ID_A];
    m->storage.currentTpsId = STORAGE_TPS_ID_A;
    if (!is_restart) {
        if (!Hcs_Init(&m->storage.hcs, HCS_FILE_PATH)) {
            TALEA_LOG_ERROR("Failed to initialize hcs. Panic!\n");
            exit(1); // TODO: same as below
        }

        int success = mtx_init(&tpsMutex, mtx_plain | mtx_recursive);
        if (success != thrd_success) {
            perror("Error initializing mutex - tps");
            exit(1); // TODO: handle this gracefully, but this is a panic
        }

        success = mtx_init(&hcsMutex, mtx_plain | mtx_recursive);
        if (success != thrd_success) {
            perror("Error initializing mutex - hcs");
            exit(1); // TODO: handle this gracefully, but this is a panic
        }
    }

    if (!is_restart) {
        if (config->TPS_A_path) {
            Storage_InsertTps(STORAGE_TPS_ID_A, config->TPS_A_path);
        }

        if (config->TPS_B_path) {
            Storage_InsertTps(STORAGE_TPS_ID_B, config->TPS_B_path);
        }
    }

    TALEA_LOG_WARNING("Should reset to a known state\n");
}

// DEVICE DEINITIALIZATION

void Storage_Deinit(TaleaMachine *m)
{
    mtx_destroy(&tpsMutex);
    mtx_destroy(&hcsMutex);

    fclose(m->storage.hcs.file);
}

// DEVICE UPDATE (on vblank)
// None

// DEVICE UPDATE (on cpu tick)
// Yes, // TODO: update on tick

// DEVICE PORT IO HANDLERS

// Handled by storage.c

u8 Storage_Read(TaleaMachine *m, u8 port)
{
    int tpsLock = mtx_lock(&tpsMutex);
    int hcsLock = mtx_lock(&hcsMutex);
    port    = port & 0xf;
    u8  value   = 0;

    if ((tpsLock != thrd_success) &&
        (port <= P_STORAGE_TPS_COMMAND || port <= P_STORAGE_TPS_STATUS)) {
        atomic_fetch_or(&m->storage.currentTps->status, STORAGE_STATUS_ERROR);
    } else if ((hcsLock != thrd_success) &&
               (port <= P_STORAGE_HCS_COMMAND || port <= P_STORAGE_HCS_STATUS)) {
        atomic_fetch_or(&m->storage.hcs.status, STORAGE_STATUS_ERROR);
    }

    switch (port) {
    // TPS
    case P_STORAGE_TPS_COMMAND: value = 0xff; break;
    case P_STORAGE_TPS_DATA: {
        if (tpsLock != thrd_success)
            value = 0xff;
        else
            value = m->storage.currentTps->data;
    } break;

    case P_STORAGE_TPS_POINTH: {
        if (tpsLock != thrd_success)
            value = 0xff;
        else
            value = m->storage.currentTps->point >> 8;
    } break;

    case P_STORAGE_TPS_POINTL: {
        if (tpsLock != thrd_success)
            value = 0xff;
        else
            value = m->storage.currentTps->point & 0xff;
    } break;

    case P_STORAGE_TPS_RESULT: {
        if (tpsLock != thrd_success)
            value = 0xff;
        else
            value = m->storage.currentTps->result;
    } break;

    case P_STORAGE_TPS_STATUS: {
        // even if we did not acquire the lock this is safe, because it is atomic
        value = atomic_load(&m->storage.currentTps->status);
    } break;

    // HCS
    case P_STORAGE_HCS_COMMAND: value = 0xff; break;

    case P_STORAGE_HCS_DATA: {
        if (hcsLock != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.data;
    } break;

    case P_STORAGE_HCS_SECTORH: {
        if (hcsLock != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.sector >> 8;
    } break;

    case P_STORAGE_HCS_SECTORL: {
        if (hcsLock != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.sector & 0xff;
    } break;

    case P_STORAGE_HCS_POINTH: {
        if (hcsLock != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.point >> 8;
    } break;

    case P_STORAGE_HCS_POINTL: {
        if (hcsLock != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.point & 0xff;
    } break;

    case P_STORAGE_HCS_RESULT: {
        if (hcsLock != thrd_success)
            value = 0xff;
        else
            value = m->storage.hcs.result;
    } break;

    case P_STORAGE_HCS_STATUS: {
        if (hcsLock != thrd_success)
            value = STORAGE_STATUS_BUSY;
        else
            value = atomic_load(&m->storage.hcs.status);
    } break;

    default: TALEA_LOG_WARNING("Reading Storage from malformed port: %x\n", port); return 0xff;
    }

    if (tpsLock == thrd_success) mtx_unlock(&tpsMutex);
    if (hcsLock == thrd_success) mtx_unlock(&hcsMutex);
    return value;
}

void Storage_Write(TaleaMachine *m, u8 port, u8 value)
{
    int tpsLock = mtx_lock(&tpsMutex);
    int hcsLock = mtx_lock(&hcsMutex);

    switch (port & 0xf) {
    // TPS
    case P_STORAGE_TPS_COMMAND: {
        if (tpsLock != thrd_success) {
            atomic_fetch_or(&m->storage.currentTps->status, STORAGE_STATUS_ERROR);
            break;
        }
        Tps_ProcessCommand(m, value);
        break;
    }
    case P_STORAGE_TPS_DATA: {
        if (tpsLock != thrd_success) {
            atomic_fetch_or(&m->storage.currentTps->status, STORAGE_STATUS_ERROR);
            break;
        }
        m->storage.currentTps->data = value;
        break;
    }
    case P_STORAGE_TPS_POINTH: {
        if (tpsLock != thrd_success) {
            atomic_fetch_or(&m->storage.currentTps->status, STORAGE_STATUS_ERROR);
            break;
        };
        TALEA_LOG_TRACE("Pointh: %02x\n", value);
        m->storage.currentTps->point = (m->storage.currentTps->point & 0x00ff) | (u16)value << 8;
        break; // TODO: proper sector alignment based on sector_size
    }
    case P_STORAGE_TPS_POINTL: {
        if (tpsLock != thrd_success) {
            atomic_fetch_or(&m->storage.currentTps->status, STORAGE_STATUS_ERROR);
            break;
        };
        TALEA_LOG_TRACE("Pointl: %02x\n", value);
        m->storage.currentTps->point = (m->storage.currentTps->point & 0xff00) | value;
        break;
    }
    case P_STORAGE_TPS_RESULT: break; // non writable register
    case P_STORAGE_TPS_STATUS:
        break; // non writable register

    // HCS
    case P_STORAGE_HCS_COMMAND: {
        if (hcsLock != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STORAGE_STATUS_ERROR);
            break;
        }
        Hcs_ProcessCommand(m, value);
        break;
    }
    case P_STORAGE_HCS_DATA: {
        if (hcsLock != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STORAGE_STATUS_ERROR);
            break;
        }
        m->storage.hcs.data = value;
        break;
    }
    case P_STORAGE_HCS_SECTORH: {
        if (hcsLock != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STORAGE_STATUS_ERROR);
            break;
        }
        m->storage.hcs.sector = (m->storage.hcs.sector & 0x00ff) | (u16)value << 8;
        break;
    }
    case P_STORAGE_HCS_SECTORL: {
        if (hcsLock != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STORAGE_STATUS_ERROR);
            break;
        }
        m->storage.hcs.sector = (m->storage.hcs.sector & 0xff00) | value;
        break;
    }
    case P_STORAGE_HCS_POINTH: {
        if (hcsLock != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STORAGE_STATUS_ERROR);
            break;
        }
        m->storage.hcs.point = (m->storage.hcs.point & 0x00ff) | (u16)value << 8;
        break; // TODO: proper sector alignment based on sector_size
    }
    case P_STORAGE_HCS_POINTL: {
        if (hcsLock != thrd_success) {
            atomic_fetch_or(&m->storage.hcs.status, STORAGE_STATUS_ERROR);
            break;
        }
        m->storage.hcs.point = (m->storage.hcs.point & 0xff00) | value;
        break;
    }
    case P_STORAGE_HCS_RESULT: break; // non writable register
    case P_STORAGE_HCS_STATUS: break; // non writable register

    default: TALEA_LOG_WARNING("Wrtiting Storage to malformed port: %x\n", port);
    }

    if (tpsLock == thrd_success) mtx_unlock(&tpsMutex);
    if (hcsLock == thrd_success) mtx_unlock(&hcsMutex);
}
