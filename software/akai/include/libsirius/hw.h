#ifndef HW_H
#define HW_H

/*
    Access to some privileged instructions from C. Implemented in assembly.
    Compile with hw.o
*/

#include "types.h"

extern u16  hw_lhud(dptr address);
extern i16  hw_lhd(dptr address);
extern u32  hw_lwd(dptr address);
extern u8   hw_lbud(dptr address);
extern void hw_shd(dptr address, u16 value);
extern void hw_sbd(dptr address, u8 value);
extern void hw_swd(dptr address, u32 value);
extern void hw_fill(void *dest, usize size, u8 c);
extern void hw_copy(void *src, const void *dest, usize size);

#endif /* HW_H */