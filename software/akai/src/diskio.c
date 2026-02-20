/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
/*clang-format off*/
#include "diskio.h" /* Declarations FatFs MAI */
#include "ff.h"     /* Basic definitions of FatFs */

/* Example: Declarations of the platform and disk functions in the project */
#define INCLUDE_STORAGE_MEDIUM_LOOKUP
#include "hw.h"
#include "kernel.h"

/* Definitions of physical drive number for each drive */
#define DEV_TPS_A 0 /* Map TPS A to physical drive 0 */
#define DEV_TPS_B 1 /* Map TPS B card to physical drive 1 */
#define DEV_HCS   2 /* Map HCS to physical drive 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS      stat = 0;
    unsigned int result;

    switch (pdrv) {
    case DEV_TPS_A:
        // NOTE: Should be safe to not wait for this command, based on the emulator's implementation
        _sbd(A.info.storage + STORAGE_TPS_DATA, TPS_ID_A);
        _sbd(A.info.storage + STORAGE_TPS_COMMAND, STORAGE_COMMAND_SETCURRENT);
        result = _lbud(A.info.storage + STORAGE_TPS_STATUS);

        stat = (result & STOR_STATUS_INSERTED) ? 0 : STA_NODISK;
        stat |= (result & STOR_STATUS_WPROT) ? STA_PROTECT : 0;

        return stat;

    case DEV_TPS_B:
        _sbd(A.info.storage + STORAGE_TPS_DATA, TPS_ID_A);
        _sbd(A.info.storage + STORAGE_TPS_COMMAND, STORAGE_COMMAND_SETCURRENT);
        result = _lbud(A.info.storage + STORAGE_TPS_STATUS);

        stat = (result & STOR_STATUS_INSERTED) ? 0 : STA_NODISK;
        stat |= (result & STOR_STATUS_WPROT) ? STA_PROTECT : 0;

        return stat;

    case DEV_HCS: return 0;
    }
    return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat;
    int     result;

    switch (pdrv) {
    case DEV_TPS_A:
    case DEV_TPS_B:
    case DEV_HCS: return 0;
    }
    return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE  pdrv,   /* Physical drive nmuber to identify the drive */
                  BYTE *buff,   /* Data buffer to store read data */
                  LBA_t sector, /* Start sector in LBA */
                  UINT  count   /* Number of sectors to read */
)
{
    DRESULT      res;
    unsigned int result;
    unsigned int i;

    switch (pdrv) {
    case DEV_TPS_A:

        // TODO: CHECK THE PARAMETERS FOR THE TPS

        _sbd(A.info.storage + STORAGE_TPS_DATA, TPS_ID_A);
        _sbd(A.info.storage + STORAGE_TPS_COMMAND, STORAGE_COMMAND_SETCURRENT);
        result = _lbud(A.info.storage + STORAGE_TPS_STATUS);

        // The function tps_sector_io already check for ready
        if (result & STOR_STATUS_ERROR) return RES_NOTRDY;

        for (i = 0; i < count; i++) {
            // TODO: transform this in a sector block read
            u8 bank = (sector + i) >> 8; // Tps has 255 sectors in up to 255 banks
            u8 sect = (sector + i) & 0xff;
            result = tps_sector_io(STORAGE_COMMAND_LOAD, bank, sect, (BYTE *)buff + (i * FF_MAX_SS),
                                   AKAI_TPSA_SECTOR);
            if (!result) return RES_ERROR;
        }

        return RES_OK;

    case DEV_TPS_B:

        // TODO: CHECK THE PARAMETERS FOR THE TPS

        _sbd(A.info.storage + STORAGE_TPS_DATA, TPS_ID_B);
        _sbd(A.info.storage + STORAGE_TPS_COMMAND, STORAGE_COMMAND_SETCURRENT);
        result = _lbud(A.info.storage + STORAGE_TPS_STATUS);

        if (result & STOR_STATUS_ERROR) return RES_NOTRDY;

        for (i = 0; i < count; i++) {
            // TODO: transform this in a sector block read
            u8 bank = (sector + i) >> 8; // Tps has 255 sectors in up to 255 banks
            u8 sect = (sector + i) & 0xff;
            result = tps_sector_io(STORAGE_COMMAND_LOAD, bank, sect, (BYTE *)buff + (i * FF_MAX_SS),
                                   AKAI_TPSB_SECTOR);
            if (!result) return RES_ERROR;
        }

        return RES_OK;

    case DEV_HCS:
        result = _lbud(A.info.storage + STORAGE_HCS_STATUS);

        if (result & STOR_STATUS_ERROR) return RES_NOTRDY;

        for (i = 0; i < count; i++) {
            // TODO: transform this in a sector block read
            u8  bank = (sector + i) >> 16; // Hps has 65536 sectors in up to 255 banks
            u16 sect = (sector + i) & 0xffff;
            result = hcs_sector_io(STORAGE_COMMAND_LOAD, bank, sect, (BYTE *)buff + (i * FF_MAX_SS),
                                   AKAI_TPSB_SECTOR);
            if (!result) return RES_ERROR;
        }
        return RES_OK;
    }

    return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE        pdrv,   /* Physical drive nmuber to identify the drive */
                   const BYTE *buff,   /* Data to be written */
                   LBA_t       sector, /* Start sector in LBA */
                   UINT        count   /* Number of sectors to write */
)
{
    DRESULT      res;
    int          result;
    unsigned int i;

    switch (pdrv) {
    case DEV_TPS_A:

        // TODO: CHECK THE PARAMETERS FOR THE TPS

        _sbd(A.info.storage + STORAGE_TPS_DATA, TPS_ID_A);
        _sbd(A.info.storage + STORAGE_TPS_COMMAND, STORAGE_COMMAND_SETCURRENT);
        result = _lbud(A.info.storage + STORAGE_TPS_STATUS);

        // The function tps_sector_io already check for ready
        if (result & STOR_STATUS_ERROR) return RES_NOTRDY;
        if (result & STOR_STATUS_WPROT) return RES_WRPRT;

        for (i = 0; i < count; i++) {
            // TODO: transform this in a sector block read
            u8 bank = (sector + i) >> 8; // Tps has 255 sectors in up to 255 banks
            u8 sect = (sector + i) & 0xff;
            result  = tps_sector_io(STORAGE_COMMAND_STORE, bank, sect,
                                    (BYTE *)buff + (i * FF_MAX_SS), AKAI_TPSA_SECTOR);
            if (!result) return RES_ERROR;
        }

        return RES_OK;

    case DEV_TPS_B:

        // TODO: CHECK THE PARAMETERS FOR THE TPS

        _sbd(A.info.storage + STORAGE_TPS_DATA, TPS_ID_B);
        _sbd(A.info.storage + STORAGE_TPS_COMMAND, STORAGE_COMMAND_SETCURRENT);
        result = _lbud(A.info.storage + STORAGE_TPS_STATUS);

        if (result & STOR_STATUS_ERROR) return RES_NOTRDY;
        if (result & STOR_STATUS_WPROT) return RES_WRPRT;

        for (i = 0; i < count; i++) {
            // TODO: transform this in a sector block read
            u8 bank = (sector + i) >> 8; // Tps has 255 sectors in up to 255 banks
            u8 sect = (sector + i) & 0xff;
            result  = tps_sector_io(STORAGE_COMMAND_STORE, bank, sect,
                                    (BYTE *)buff + (i * FF_MAX_SS), AKAI_TPSB_SECTOR);
            if (!result) return RES_ERROR;
        }

        return RES_OK;

    case DEV_HCS:
        result = _lbud(A.info.storage + STORAGE_HCS_STATUS);

        if (result & STOR_STATUS_ERROR) return RES_NOTRDY;
        if (result & STOR_STATUS_WPROT) return RES_WRPRT;

        for (i = 0; i < count; i++) {
            // TODO: transform this in a sector block read
            u8  bank = (sector + i) >> 16; // Hps has 65536 sectors in up to 255 banks
            u16 sect = (sector + i) & 0xffff;
            result   = hcs_sector_io(STORAGE_COMMAND_STORE, bank, sect,
                                     (BYTE *)buff + (i * FF_MAX_SS), AKAI_TPSB_SECTOR);
            if (!result) return RES_ERROR;
        }
        return RES_OK;
    }

    return RES_PARERR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

static DRESULT tps_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    BYTE result;

    _sbd(A.info.storage + STORAGE_TPS_DATA, pdrv);
    _sbd(A.info.storage + STORAGE_TPS_COMMAND, STORAGE_COMMAND_SETCURRENT);
    result = _lbud(A.info.storage + STORAGE_TPS_STATUS);

    if (result & STOR_STATUS_ERROR) return RES_ERROR;

    switch (cmd) {
    case CTRL_SYNC:
        while (_lbud(A.info.storage + STORAGE_TPS_STATUS) & STOR_STATUS_BUSY);
        return RES_OK;
    case GET_SECTOR_COUNT: {
        _sbd(A.info.storage + STORAGE_TPS_COMMAND, STORAGE_COMMAND_MEDIUM);
        result         = _lbud(A.info.storage + STORAGE_TPS_RESULT);
        *(LBA_t *)buff = (LBA_t)Storage_MediumLookup[result][1];
        return RES_OK;
    }
    case GET_SECTOR_SIZE:
        *(WORD *)buff = 512; // hardcode for now (or maybe forever)
        return RES_OK;
    case GET_BLOCK_SIZE: *(DWORD *)buff = 1; return RES_OK;
    case CTRL_TRIM: return RES_OK;
    }

    return RES_PARERR;
}

DRESULT disk_ioctl(BYTE  pdrv, /* Physical drive nmuber (0..) */
                   BYTE  cmd,  /* Control code */
                   void *buff  /* Buffer to send/receive control data */
)
{
    DRESULT res;
    int     result;

    switch (pdrv) {
    case DEV_TPS_A:
    case DEV_TPS_B: return tps_ioctl(pdrv, cmd, buff);

    case DEV_HCS:
        result = _lbud(A.info.storage + STORAGE_HCS_STATUS);
        if (result & STOR_STATUS_ERROR) return RES_ERROR;

        switch (cmd) {
        case CTRL_SYNC:
            while (_lbud(A.info.storage + STORAGE_HCS_STATUS) & STOR_STATUS_BUSY);
            return RES_OK;
        case GET_SECTOR_COUNT: {
            _sbd(A.info.storage + STORAGE_HCS_COMMAND, STORAGE_COMMAND_MEDIUM);
            result         = _lbud(A.info.storage + STORAGE_HCS_RESULT);
            *(LBA_t *)buff = (LBA_t)Storage_MediumLookup[result][1];

            return RES_OK;
        }
        case GET_SECTOR_SIZE:
            *(WORD *)buff = 512; // hardcode for now (or maybe forever)
            return RES_OK;
        case GET_BLOCK_SIZE: *(DWORD *)buff = 1; return RES_OK;
        case CTRL_TRIM: return RES_OK;
        }
    }

    return RES_PARERR;
}

DWORD get_fattime(void)
{
    BYTE ss, mm, hh, dd, mo, yy;
    // set calendar mode
    _sbd(REG_SYSTEM_YEAR, TALEA_SYSTEM_CALENDAR_MODE);

    yy = _lbud(REG_SYSTEM_YEAR);
    ss = _lbud(REG_SYSTEM_SECOND);
    mm = _lbud(REG_SYSTEM_MINUTE);
    hh = _lbud(REG_SYSTEM_HOUR);
    dd = _lbud(REG_SYSTEM_DAY);
    mo = _lbud(REG_SYSTEM_MONTH);

    if (ss == 59) {
        yy = _lbud(REG_SYSTEM_YEAR);
        ss = _lbud(REG_SYSTEM_SECOND);
        mm = _lbud(REG_SYSTEM_MINUTE);
        hh = _lbud(REG_SYSTEM_HOUR);
        dd = _lbud(REG_SYSTEM_DAY);
        mo = _lbud(REG_SYSTEM_MONTH);
    }

    return (DWORD)(yy - 80) << 25 | (DWORD)(mo + 1) << 21 | (DWORD)dd << 16 | (DWORD)hh << 11 |
           (DWORD)mm << 5 | (DWORD)ss >> 1;
}
