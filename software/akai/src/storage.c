#include "hw.h"
#include "kernel.h"

bool tps_sector_io(int command, u8 bank, u8 sector, u8 *buff, u16 data_buff)
{
    u32 left;
    // copy from TAM to DATA IF COMMAND == STORE
    if (command == STORAGE_COMMAND_STORE) {
        _copymd(buff, data_buff, 512);
    };

    while (_lbud(A.info.storage + STORAGE_TPS_STATUS) & STOR_STATUS_BUSY); // wait for ready

    _sbd(A.info.storage + STORAGE_TPS_DATA, bank);
    _sbd(A.info.storage + STORAGE_TPS_COMMAND, STORAGE_COMMAND_BANK);

    if (_lbud(A.info.storage + STORAGE_TPS_STATUS) & STOR_STATUS_ERROR) {
        return false;
    }

    while (_lbud(A.info.storage + STORAGE_TPS_STATUS) & STOR_STATUS_BUSY); // wait for ready

    _sbd(A.info.storage + STORAGE_TPS_DATA, sector);
    // Assume buff is aligned, and if not, we'r sorry
    _shd(A.info.storage + STORAGE_TPS_POINT, data_buff >> 9);
    _sbd(A.info.storage + STORAGE_TPS_COMMAND, command);

    while (_lbud(A.info.storage + STORAGE_TPS_STATUS) & STOR_STATUS_BUSY); // wait for ready

    if (_lbud(A.info.storage + STORAGE_TPS_STATUS) & STOR_STATUS_ERROR) {
        return false;
    }

    // copy from DATA to RAM IF COMMAND == LOAD
    if (command == STORAGE_COMMAND_LOAD) {
        _copydm(data_buff, buff, 512);
    };

    return true;
}

bool hcs_sector_io(int command, u8 bank, u16 sector, u8 *buff, u16 data_buff)
{
    u32 left;
    // copy from TAM to DATA IF COMMAND == STORE
    if (command == STORAGE_COMMAND_STORE) {
        _copymd(buff, data_buff, 512);
    };

    while (_lbud(A.info.storage + STORAGE_HCS_STATUS) & STOR_STATUS_BUSY); // wait for ready

    _sbd(A.info.storage + STORAGE_HCS_DATA, bank);
    _shd(A.info.storage + STORAGE_HCS_SECTOR, sector);
    // Assume buff is aligned, and if not, we'r sorry
    _shd(A.info.storage + STORAGE_HCS_POINT, data_buff >> 9);
    _sbd(A.info.storage + STORAGE_HCS_COMMAND, command);

    if (_lbud(A.info.storage + STORAGE_HCS_STATUS) & STOR_STATUS_ERROR) {
        return false;
    };

    while (_lbud(A.info.storage + STORAGE_HCS_STATUS) & STOR_STATUS_BUSY); // wait for ready

    // return
    if (_lbud(A.info.storage + STORAGE_HCS_STATUS) & STOR_STATUS_ERROR) {
        return false;
    }

    if (command == STORAGE_COMMAND_LOAD) {
        _copydm(data_buff, buff, 512);
    };

    return true;
}
