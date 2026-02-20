#include "kernel.h"

#define err A.pr.curr->last_error

static i32 attach(u32 devnum, int res_type, u32 res)
{
    if (res_type == VDEV) {
        err = P_ERROR_NOT_IMPLEMENTED;
        return A_ERROR_CTL;
    }

    if (res_type != VDEV && res_type != HW && res_type != FILE) {
        err = A_ERROR_INVAL;
        return A_ERROR_CTL;
    }

    switch (devnum) {
    case PDEV_STDIN:

        if (res_type == HW) {
            if (res < _DEV_NUM && res != DEV_KEYBOARD && res != DEV_SERIAL) {
                err = P_ERROR_CANNOT_ATTACH;
                return A_ERROR_CTL;
            }

            if (A.device_owners[res] != A.pr.curr->pid) {
                err = A_ERROR_CLAIM;
                return A_ERROR_CTL;
            }

            A.pr.curr->stdin.res_type = HW;
            A.pr.curr->stdin.res.hw   = &A.devices[res];
        } else if (res_type == FILE) {
            if (A.pr.curr->fds[res] == FREE_FD) {
                err = P_ERROR_CANNOT_ATTACH;
                return A_ERROR_CTL;
            }

            A.pr.curr->stdin.res_type = FILE;
            A.pr.curr->stdin.res.fd   = A.pr.curr->fds[res];
        }

        break;
    case PDEV_STDOUT:
        if (res_type == HW) {
            if (res < _DEV_NUM && res != DEV_TEXTBUFFER && res != DEV_SERIAL) {
                err = P_ERROR_CANNOT_ATTACH;
                return A_ERROR_CTL;
            }

            if (A.device_owners[res] != A.pr.curr->pid) {
                err = A_ERROR_CLAIM;
                return A_ERROR_CTL;
            }

            A.pr.curr->stdout.res_type = HW;
            A.pr.curr->stdout.res.hw   = &A.devices[res];
        } else if (res_type == FILE) {
            if (A.pr.curr->fds[res] == FREE_FD) {
                err = P_ERROR_CANNOT_ATTACH;
                return A_ERROR_CTL;
            }

            A.pr.curr->stdout.res_type = FILE;
            A.pr.curr->stdout.res.fd   = A.pr.curr->fds[res];
        }

        break;
    case PDEV_STDERR:
        if (res_type == HW) {
            if (res < _DEV_NUM && res != DEV_TEXTBUFFER && res != DEV_SERIAL) {
                err = P_ERROR_CANNOT_ATTACH;
                return A_ERROR_CTL;
            }

            if (A.device_owners[res] != A.pr.curr->pid) {
                err = A_ERROR_CLAIM;
                return A_ERROR_CTL;
            }

            A.pr.curr->stderr.res_type = HW;
            A.pr.curr->stderr.res.hw   = &A.devices[res];
        } else if (res_type == FILE) {
            if (A.pr.curr->fds[res] == FREE_FD) {
                err = P_ERROR_CANNOT_ATTACH;
                return A_ERROR_CTL;
            }

            A.pr.curr->stderr.res_type = FILE;
            A.pr.curr->stderr.res.fd   = A.pr.curr->fds[res];
        }
        break;
    default: err = P_ERROR_NO_DEV; return A_ERROR_CTL;
    }

    return A_OK;
}

i32 proxy_ctl(u32 devnum, u32 command, void *buff, u32 len)
{
    switch (command) {
    case PX_ATTACH: {
        u32 *args = (u32 *)buff;

        if (len < (2 * sizeof(u32))) {
            err = A_ERROR_INVAL;
            return A_ERROR_CTL;
        }

        return attach(devnum, args[0], args[1]);
    }
    case PX_READ: {
        int res_type = A.pr.curr->stdin.res_type;
        if (devnum != PDEV_STDIN) goto noctl;
        if (res_type == VDEV) {
            err = P_ERROR_NOT_IMPLEMENTED;
            return A_ERROR_CTL;
        } else if (res_type == HW) {
            return A.pr.curr->stdin.res.hw->ctl(PX_READ, buff, len);
        } else if (res_type == FILE) {
            FIL    *f = &A.fp.files[A.pr.curr->stdin.res.fd];
            UINT    br;
            FRESULT res = f_read(f, buff, len, &br);
            if (res != FR_OK) {
                err = FS_ERROR | res;
                return A_ERROR_CTL;
            } else {
                return br;
            }
        } else {
            err = A_ERROR_INVAL;
            return A_ERROR_CTL;
        }
        break;
    }
    case PX_WRITE: {
        struct IOStream *s        = devnum == PDEV_STDOUT ? &A.pr.curr->stdout : &A.pr.curr->stderr;
        int              res_type = s->res_type;

        if (devnum != PDEV_STDOUT && devnum != PDEV_STDERR) goto noctl;

        if (res_type == VDEV) {
            err = P_ERROR_NOT_IMPLEMENTED;
            return A_ERROR_CTL;
        } else if (res_type == HW) {
            return s->res.hw->ctl(PX_WRITE, buff, len);
        } else if (res_type == FILE) {
            FIL    *f = &A.fp.files[s->res.fd];
            UINT    bw;
            FRESULT res = f_write(f, buff, len, &bw);
            if (res != FR_OK) {
                err = FS_ERROR | res;
                return A_ERROR_CTL;
            } else {
                return bw;
            }
        } else {
            err = A_ERROR_INVAL;
            return A_ERROR_CTL;
        }
        break;
    }
    case PX_POLL: {
        struct IOStream *s = devnum == PDEV_STDOUT ? &A.pr.curr->stdout :
                             devnum == PDEV_STDERR ? &A.pr.curr->stderr :
                                                     &A.pr.curr->stdin;

        int res_type = s->res_type;

        if (res_type == VDEV) {
            err = P_ERROR_NOT_IMPLEMENTED;
            return A_ERROR_CTL;
        } else if (res_type == HW) {
            return s->res.hw->ctl(PX_POLL, buff, len);
        } else if (res_type == FILE) {
            // Always ready
            return 1;
        } else {
            err = A_ERROR_INVAL;
            return A_ERROR_CTL;
        }
        break;
    }
    case PX_FLUSH: {
        struct IOStream *s        = devnum == PDEV_STDOUT ? &A.pr.curr->stdout : &A.pr.curr->stderr;
        int              res_type = s->res_type;

        if (devnum != PDEV_STDOUT && devnum != PDEV_STDERR) goto noctl;

        if (res_type == VDEV) {
            err = P_ERROR_NOT_IMPLEMENTED;
            return A_ERROR_CTL;
        } else if (res_type == HW) {
            return s->res.hw->ctl(PX_FLUSH, buff, len);
        } else if (res_type == FILE) {
            FIL    *f   = &A.fp.files[s->res.fd];
            FRESULT res = f_sync(f);
            if (res != FR_OK) {
                err = FS_ERROR | res;
                return A_ERROR_CTL;
            } else {
                return A_OK;
            }
        } else {
            err = A_ERROR_INVAL;
            return A_ERROR_CTL;
        }
        break;
    }

    case PX_SETCANON: {
        struct IOStream *s = devnum == PDEV_STDOUT ? &A.pr.curr->stdout :
                             devnum == PDEV_STDERR ? &A.pr.curr->stderr :
                                                     &A.pr.curr->stdin;

        int res_type = s->res_type;

        if (res_type == VDEV) {
            err = P_ERROR_NOT_IMPLEMENTED;
            return A_ERROR_CTL;
        } else if (res_type == HW) {
            return s->res.hw->ctl(PX_SETCANON, buff, len);
        } else if (res_type == FILE) {
            return A_OK; // Files are always in raw mode
        } else {
            err = A_ERROR_INVAL;
            return A_ERROR_CTL;
        }
        break;
    }

    case PX_GETCANON: {
        struct IOStream *s = devnum == PDEV_STDOUT ? &A.pr.curr->stdout :
                             devnum == PDEV_STDERR ? &A.pr.curr->stderr :
                                                     &A.pr.curr->stdin;

        int res_type = s->res_type;

        if (res_type == VDEV) {
            err = P_ERROR_NOT_IMPLEMENTED;
            return A_ERROR_CTL;
        } else if (res_type == HW) {
            return s->res.hw->ctl(PX_GETCANON, buff, len);
        } else if (res_type == FILE) {
            return A_OK; // Files are always in raw mode
        } else {
            err = A_ERROR_INVAL;
            return A_ERROR_CTL;
        }
        break;
    }

    default: {
noctl:
        err = P_ERROR_NO_CTL_COMMAND;
        return A_ERROR_CTL;
    }
    }

    return A_OK;
}

u8 proxy_in(u32 devnum, u8 port)
{
    u8  buff[1];
    i32 res = proxy_ctl(devnum, PX_READ, &buff, 1);
    (void)port;

    _trace(0xFEDEA, res);
    if (res <= 0) {
        return 0;
    }

    return buff[0];
}

i32 proxy_out(u32 devnum, u8 port, u8 val)
{
    u8 buff[1];

    _trace(0xDAF0, devnum);
    buff[0] = val;
    (void)port;
    return proxy_ctl(devnum, PX_WRITE, &buff, 1);
}
