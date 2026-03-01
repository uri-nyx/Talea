#ifndef DEV_H
#define DEV_H

#include "akai_def.h"

typedef i32 (*DevCtl)(u32, void *, u32);
typedef i32 (*DevReset)(void);
typedef u8 (*DevIn)(u8);
typedef i32 (*DevOut)(u8, u8);

/* @AKAI: 200_DRIVERS */

#define PDEV_TYPE_HW   0x1000
#define PDEV_TYPE_FILE 0x2000
#define PDEV_TYPE_VDEV 0x4000

enum HwDevices {
    DEV_FRAMEBUFFER,
    DEV_TEXTBUFFER,
    DEV_SYNTH,
    DEV_PCM,
    DEV_SERIAL,
    DEV_KEYBOARD,
    _DEV_NUM,
};

enum VDevices {
    _VDEV_START = 128,
    VDEV_ZERO   = 128,
    _VDEV_NUM,
};

enum DevProxy {
    PDEV_STDIN = 255,
    PDEV_STDOUT,
    PDEV_STDERR,
};

enum CanonIMode {
    IN_CANON = 1U << 0U,
    IN_ECHO  = 1U << 1U,
    IN_CRNL  = 1U << 2U,
    IN_BLOCK = 1U << 3U,
};

enum CanonOMode {
    OUT_NLCR = 1U << 0U,
};

// Common CTL commands
enum CtlCommand {
    DEV_RESET = 100,
    /* Proxy stream operations */
    PX_ATTACH = 1000,
    PX_READ,
    PX_WRITE,
    PX_POLL,
    PX_FLUSH,
    PX_SETCANON,
    PX_GETCANON,
    PX_GET_DEV,
};

enum KbCtl {
    KCTL_GETKEY,      // Non blocking key read
    KCTL_WAITKEY,     // Blocking key read
    KCTL_PEEK,        // Peek item from keyboard queue
    KCTL_QUEUE_COUNT, // Get no of items in queue
};

enum TxtCtl {
    TCTL_GET_INFO,
    TCTL_GET_CURSOR,
    TCTL_SET_CURSOR,
    TCTL_SET_ATTR,
    TCTL_GET_ATTR,
};

/* @AKAI */

struct DeviceDeed {
    ProcessPID original;
    ProcessPID lessor;
    ProcessPID owner;
};

struct Device {
    struct DeviceDeed deed;
    u32               num;
    u8                id;
    u16               base;
    u8                ports;
    DevCtl            ctl;
    DevReset          reset;
    DevOut            out;
    DevIn             in;
};

struct IOStream {
    enum { HW, VDEV, FILE } res_type;
    union {
        struct Device *hw;
        u32            vdevnum;
        i16            fd;
    } res;
};

i32 proxy_ctl(u32 devnum, u32 command, void *buff, u32 len);
u8  proxy_in(u32 devnum, u8 port);
i32 proxy_out(u32 devnum, u8 port, u8 val);

bool dev_lease(ProcessPID lessor, ProcessPID receiver, u32 devnum);
void proxy_attach(ProcessPID pid, u32 proxy, struct IOStream *stream);

#endif /* DEV_H */
