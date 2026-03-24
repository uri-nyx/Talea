#ifndef HW_H
#define HW_H

#include "akai_def.h"

extern void  _cli(void);
extern void  _sti(void);
extern void  _ssreg(u32 new_sreg);
extern u32   _gsreg(void);
extern void  _sbd(u16 addr, u8 value);
extern void  _shd(u16 addr, u16 value);
extern void  _swd(u16 addr, u32 value);
extern u8    _lbud(u16 addr);
extern u16   _lhud(u16 addr);
extern u32   _lwd(u16 addr);
extern void  _copydm(u16 data_addr_src, void *buff_dest, usize sz);
extern void  _copymd(void *buff_src, u16 data_addr_dest, usize sz);

/* @AKAI: 10000_HW */
extern void *_fillw(void *s, u32 c, usize n);
extern void *_copybck(void *src, void *dest, usize sz);
/* @AKAI */

extern u32   _disable_interrupts(void);
#define _restore_interrupts(sreg) _ssreg((sreg))

bool tps_sector_io(int command, u8 bank, u8 sector, u8 *buff, u16 data_buff);
bool hcs_sector_io(int command, u8 bank, u16 sector, u8 *buff, u16 data_buff);

#endif /* HW_H */
