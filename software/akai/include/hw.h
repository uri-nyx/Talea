#ifndef HW_H
#define HW_H

#include "libsirius/types.h"
#include "../include/system.h"

extern void  _ssreg(u32 new_sreg);
extern u32   _gsreg(void);
extern void  _sbd(u16 addr, u8 value);
extern void  _shd(u16 addr, u16 value);
extern void  _swd(u16 addr, u32 value);
extern u8    _lbud(u16 addr);
extern u16   _lhud(u16 addr);
extern u32   _lwd(u16 addr);
extern usize _copydm(u16 data_addr_src, void *buff_dest, usize sz);
extern usize _copymd(void *buff_src, u16 data_addr_dest, usize sz);

extern void *memcpy(void *dest, const void *src, usize n);
extern void *memset(void *s, int c, usize n);


#endif /* HW_H */
