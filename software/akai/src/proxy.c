#include "hw.h"
#include "kernel.h"

#define err A.pr.curr->last_error

#define GET_RES(type)                       \
    ((type) == HW   ? stream->res.hw->num : \
     (type) == FILE ? stream->res.fd :      \
     (type) == VDEV ? stream->res.vdevnum : \
                      BAD_RES)

void proxy_attach(ProcessPID pid, u32 proxy, struct IOStream *stream)
{
    int res_type = stream->res_type;
    u32 res      = GET_RES(res_type);

    if (res == BAD_RES) return;

    switch (proxy) {
    case PDEV_STDIN:

        if (res_type == HW) {
            if (res < _DEV_NUM && res != DEV_KEYBOARD && res != DEV_SERIAL) return;
            if (DEED_OWNER(&A.devices[res].deed) != pid) return;
            A.pr.proc[pid].stdin.res_type = HW;
            A.pr.proc[pid].stdin.res.hw   = &A.devices[res];
        } else if (res_type == FILE) {
            if (A.pr.proc[pid].fds[res] == FREE_FD) return;
            A.pr.proc[pid].stdin.res_type = FILE;
            A.pr.proc[pid].stdin.res.fd   = A.pr.proc[pid].fds[res];
        }

        break;
    case PDEV_STDOUT:
        if (res_type == HW) {
            if (res < _DEV_NUM && res != DEV_TEXTBUFFER && res != DEV_SERIAL) return;
            if (DEED_OWNER(&A.devices[res].deed) != pid) return;
            A.pr.proc[pid].stdout.res_type = HW;
            A.pr.proc[pid].stdout.res.hw   = &A.devices[res];
        } else if (res_type == FILE) {
            if (A.pr.proc[pid].fds[res] == FREE_FD) return;
            A.pr.proc[pid].stdout.res_type = FILE;
            A.pr.proc[pid].stdout.res.fd   = A.pr.proc[pid].fds[res];
        }

        break;
    case PDEV_STDERR:
        if (res_type == HW) {
            if (res < _DEV_NUM && res != DEV_TEXTBUFFER && res != DEV_SERIAL) return;
            if (DEED_OWNER(&A.devices[res].deed) != pid) return;
            A.pr.proc[pid].stderr.res_type = HW;
            A.pr.proc[pid].stderr.res.hw   = &A.devices[res];
        } else if (res_type == FILE) {
            if (A.pr.proc[pid].fds[res] == FREE_FD) return;
            A.pr.proc[pid].stderr.res_type = FILE;
            A.pr.proc[pid].stderr.res.fd   = A.pr.proc[pid].fds[res];
        }
        break;
    default: return;
    }
}

static i32 attach(u32 devnum, int res_type, u32 res)
{
    if (res_type == VDEV) {
        err = P_ERROR_NOT_IMPLEMENTED;
        return (signed)A_ERROR_CTL;
    }

    if (res_type != VDEV && res_type != HW && res_type != FILE) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_CTL;
    }

    switch (devnum) {
    case PDEV_STDIN:

        if (res_type == HW) {
            if (res < _DEV_NUM && res != DEV_KEYBOARD && res != DEV_SERIAL) {
                err = P_ERROR_CANNOT_ATTACH;
                return (signed)A_ERROR_CTL;
            }

            if (DEED_OWNER(&A.devices[res].deed) != A.pr.curr->pid) {
                err = A_ERROR_CLAIM;
                return (signed)A_ERROR_CTL;
            }

            A.pr.curr->stdin.res_type = HW;
            A.pr.curr->stdin.res.hw   = &A.devices[res];
        } else if (res_type == FILE) {
            if (A.pr.curr->fds[res] == FREE_FD) {
                err = P_ERROR_CANNOT_ATTACH;
                return (signed)A_ERROR_CTL;
            }

            A.pr.curr->stdin.res_type = FILE;
            A.pr.curr->stdin.res.fd   = A.pr.curr->fds[res];
        }

        break;
    case PDEV_STDOUT:
        if (res_type == HW) {
            if (res < _DEV_NUM && res != DEV_TEXTBUFFER && res != DEV_SERIAL) {
                err = P_ERROR_CANNOT_ATTACH;
                return (signed)A_ERROR_CTL;
            }

            if (DEED_OWNER(&A.devices[res].deed) != A.pr.curr->pid) {
                err = A_ERROR_CLAIM;
                return (signed)A_ERROR_CTL;
            }

            A.pr.curr->stdout.res_type = HW;
            A.pr.curr->stdout.res.hw   = &A.devices[res];
        } else if (res_type == FILE) {
            if (A.pr.curr->fds[res] == FREE_FD) {
                //("CANNOT ATTACH: %d\n", res);
                err = P_ERROR_CANNOT_ATTACH;
                return (signed)A_ERROR_CTL;
            }
            //("ATTACHED: %d (%d)\n", res, A.pr.curr->fds[res]);

            A.pr.curr->stdout.res_type = FILE;
            A.pr.curr->stdout.res.fd   = A.pr.curr->fds[res];
        }

        break;
    case PDEV_STDERR:
        if (res_type == HW) {
            if (res < _DEV_NUM && res != DEV_TEXTBUFFER && res != DEV_SERIAL) {
                err = P_ERROR_CANNOT_ATTACH;
                return (signed)A_ERROR_CTL;
            }

            if (DEED_OWNER(&A.devices[res].deed) != A.pr.curr->pid) {
                err = A_ERROR_CLAIM;
                return (signed)A_ERROR_CTL;
            }

            A.pr.curr->stderr.res_type = HW;
            A.pr.curr->stderr.res.hw   = &A.devices[res];
        } else if (res_type == FILE) {
            if (A.pr.curr->fds[res] == FREE_FD) {
                err = P_ERROR_CANNOT_ATTACH;
                return (signed)A_ERROR_CTL;
            }

            A.pr.curr->stderr.res_type = FILE;
            A.pr.curr->stderr.res.fd   = A.pr.curr->fds[res];
        }
        break;
    default: err = P_ERROR_NO_DEV; return (signed)A_ERROR_CTL;
    }

    return (signed)A_OK;
}

i32 proxy_ctl(u32 devnum, u32 command, void *buff, u32 len)
{
    _trace(0xdd1dede, devnum, command);
    switch (command) {
    case PX_ATTACH: {
        u32 *args = (u32 *)buff;

        if (len < (2 * sizeof(u32))) {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }

        return attach(devnum, args[0], args[1]);
    }
    case PX_GET_DEV: {
        struct IOStream *s;
        u32              dev;

        if (devnum < PDEV_STDIN || devnum > PDEV_STDERR) {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }

        s = devnum == PDEV_STDOUT ? &A.pr.curr->stdout :
            devnum == PDEV_STDERR ? &A.pr.curr->stderr :
                                    &A.pr.curr->stdin;

        if (s->res_type == FILE)
            return (PDEV_TYPE_FILE | s->res.fd);
        else if (s->res_type == HW)
            return (PDEV_TYPE_HW | s->res.hw->num);
        else if (s->res_type == VDEV)
            return (PDEV_TYPE_VDEV | s->res.vdevnum);
        else {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }
    }
    case PX_READ: {
        int res_type = A.pr.curr->stdin.res_type;
        if (devnum != PDEV_STDIN) goto noctl;
        if (res_type == VDEV) {
            err = P_ERROR_NOT_IMPLEMENTED;
            return (signed)A_ERROR_CTL;
        } else if (res_type == HW) {
            return (signed)A.pr.curr->stdin.res.hw->ctl(PX_READ, buff, len);
        } else if (res_type == FILE) {
            FIL    *f = &A.fp.files[A.pr.curr->stdin.res.fd];
            UINT    br;
            FRESULT res = f_read(f, buff, len, &br);
            if (res != FR_OK) {
                err = FS_ERROR | res;
                return (signed)A_ERROR_CTL;
            } else {
                return br;
            }
        } else {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }
        break;
    }
    case PX_WRITE: {
        struct IOStream *s        = devnum == PDEV_STDOUT ? &A.pr.curr->stdout : &A.pr.curr->stderr;
        int              res_type = s->res_type;

        _trace(0xdd1dedd, res_type, devnum);
        //("[KERNEL] writing to proxy io stream (type: %d, devnum: %d)\n", res_type, devnum);

        if (devnum != PDEV_STDOUT && devnum != PDEV_STDERR) goto noctl;

        if (res_type == VDEV) {
            err = P_ERROR_NOT_IMPLEMENTED;
            return (signed)A_ERROR_CTL;
        } else if (res_type == HW) {
            return s->res.hw->ctl(PX_WRITE, buff, len);
        } else if (res_type == FILE) {
            FIL    *f = &A.fp.files[s->res.fd];
            UINT    bw;
            FRESULT res = f_write(f, buff, len, &bw);
            //("[KERNEL] writing to proxy io stream: %d\n", s->res.fd);
            if (res != FR_OK) {
                //("[KERNEL] writing failed: %d\n", res);
                err = FS_ERROR | res;
                return (signed)A_ERROR_CTL;
            } else {
                //("[KERNEL] writing succeded: %d bytes written\n", bw);
                return bw;
            }
        } else {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
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
            return (signed)A_ERROR_CTL;
        } else if (res_type == HW) {
            return s->res.hw->ctl(PX_POLL, buff, len);
        } else if (res_type == FILE) {
            // Always ready
            return 1;
        } else {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }
        break;
    }
    case PX_FLUSH: {
        struct IOStream *s        = devnum == PDEV_STDOUT ? &A.pr.curr->stdout : &A.pr.curr->stderr;
        int              res_type = s->res_type;

        if (devnum != PDEV_STDOUT && devnum != PDEV_STDERR) goto noctl;

        if (res_type == VDEV) {
            err = P_ERROR_NOT_IMPLEMENTED;
            return (signed)A_ERROR_CTL;
        } else if (res_type == HW) {
            return s->res.hw->ctl(PX_FLUSH, buff, len);
        } else if (res_type == FILE) {
            FIL    *f   = &A.fp.files[s->res.fd];
            FRESULT res = f_sync(f);
            if (res != FR_OK) {
                err = FS_ERROR | res;
                return (signed)A_ERROR_CTL;
            } else {
                return (signed)A_OK;
            }
        } else {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
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
            return (signed)A_ERROR_CTL;
        } else if (res_type == HW) {
            return s->res.hw->ctl(PX_SETCANON, buff, len);
        } else if (res_type == FILE) {
            return (signed)A_OK; // Files are always in raw mode
        } else {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
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
            return (signed)A_ERROR_CTL;
        } else if (res_type == HW) {
            return s->res.hw->ctl(PX_GETCANON, buff, len);
        } else if (res_type == FILE) {
            return (signed)A_OK; // Files are always in raw mode
        } else {
            err = A_ERROR_INVAL;
            return (signed)A_ERROR_CTL;
        }
        break;
    }

    default: {
noctl:
        err = P_ERROR_NO_CTL_COMMAND;
        return (signed)A_ERROR_CTL;
    }
    }

    return (signed)A_OK;
}

u8 proxy_in(u32 devnum, u8 port)
{
    u8  buff[2];
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
    u8 buff[2];
    _trace(0xDAF0, devnum, val);
    buff[0] = val;
    buff[1] = 0;
    (void)port;
    return proxy_ctl(devnum, PX_WRITE, buff, 1);
}

bool dev_lease(ProcessPID lessor, ProcessPID receiver, u32 devnum)
{
    struct DeviceDeed *deed;

    if (lessor == receiver) return false; // no leasing to oneself

    deed = get_deed(lessor, devnum);
    if (!deed) return false;

    if (DEED_OWNER(deed) == receiver) return false;
    if (lessor != DEED_OWNER(deed)) return false;

    if (deed->depth >= MAX_PROCESS) {
        // should this warrant a kernel panic?
        miniprint("unreacheable: dev_lease\n");
        return false;
    }

    deed->lineage[deed->depth++] = receiver;

    return true;
}

bool dev_return(ProcessPID owner, u32 devnum)
{
    struct DeviceDeed *deed = get_deed(owner, devnum);
    usize              i;

    if (!deed) return false;

    if (DEED_UNCLAIMED(deed)) return true;

    if (DEED_OWNER(deed) == owner) {
        deed->lineage[deed->depth - 1] = KERNEL_PID;
        deed->depth--;
        return true;
    }

    for (i = deed->depth; i > 0; i--) {
        if (deed->lineage[i - 1] == owner) {
            deed->lineage[i - 1] = KERNEL_PID;
            _copybck(&deed->lineage[i], &deed->lineage[i - 1], deed->depth - i);
            deed->depth--;
            return true;
        }
    }

    return false;
}

struct DeviceDeed *get_deed(ProcessPID pid, u32 devnum)
{
    if (devnum < _DEV_NUM) {
        // Hardware device
        return &A.devices[devnum].deed;
    } else if (devnum >= PDEV_STDIN && devnum <= PDEV_STDERR) {
        // Proxy device, deref deed
        struct IOStream *s = devnum == PDEV_STDIN  ? &A.pr.proc[pid].stdin :
                             devnum == PDEV_STDOUT ? &A.pr.proc[pid].stdout :
                                                     &A.pr.proc[pid].stderr;
        if (s->res_type != HW)
            return NULL; // cannot lease a file or a VDEV
        else
            return &s->res.hw->deed;
    } else {
        return NULL; // cannot lease a VDEV
    }
}

bool is_in_lineage(ProcessPID pid, struct DeviceDeed *deed)
{
    usize i;

    if (!deed) return false;
    if (DEED_UNCLAIMED(deed)) return false;
    if (DEED_OWNER(deed) == pid) return true;

    for (i = 0; i < deed->depth; i++) {
        if (deed->lineage[i] == pid) return true;
    }

    return false;
}
