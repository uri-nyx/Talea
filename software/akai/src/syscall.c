#define INCLUDE_DAYS_TABLE
#include "a.out.h"
#include "hw.h"
#include "kernel.h"

#define err A.pr.curr->last_error

typedef i32 (*AkaiSyscall)(u32 *win);

static u8 path_get_drive(const char *path)
{
    // assume path has been checked
    if (path[0] != '/') goto current;

    switch (path[1]) {
    case 'a':
    case 'A': return 0;
    case 'b':
    case 'B': return 1;
    case 'c':
    case 'H': return 2;
    }

current:
    return (signed)A.pr.curr->curr_drive;
}

#define MAX_GUESS_HEADER       32
#define BIN_SIGNED_HEADER_SIZE 12
static int detect_exec_format(FIL *f)
{
    UINT    read;
    u8      header[MAX_GUESS_HEADER];
    FRESULT res;

    f_rewind(f);

    // try first with binary (has to be signed)
    res = f_read(f, header, BIN_SIGNED_HEADER_SIZE, &read);
    if (res != FR_OK || read != BIN_SIGNED_HEADER_SIZE) goto fail;
    f_rewind(f);
    if (memcmp(&header[4], "AKAIBIN!", 8) == 0) return O_EXEC_BIN_SIGNED;

    // try with a.out
    res = f_read(f, header, 32, &read);
    if (res != FR_OK || read != 32) goto fail;
    f_rewind(f);
    if (*(u32 *)header == EXEC_MAGIC) return O_EXEC_AOUT_FLAT;

fail:
    f_rewind(f);
    return -1;
}

// returns the address of the break pointer. O if loading failed
static u32 load_flat_binary(FIL *f)
{
    FSIZE_t sz = f_size(f);
    UINT    read;
    usize   file_pages = (sz + (PAGE_SIZE - 1)) >> 12;
    u8     *code;
    usize   i;

    if (sz > MAX_EXEC_FSIZE) return 0;

    code = alloc_pages_contiguous(A.pr.curr->pid, file_pages);
    if (!code) return 0;

    if (!map_active_range((u32)code, AKAI_PROCESS_BASE, file_pages, PTE_U | PTE_RWX)) {
        free_pages_contiguous(code, file_pages);
        return 0;
    }

    if (f_read(f, (u8 *)AKAI_PROCESS_BASE, sz, &read) != FR_OK || read != sz) {
        free_pages_contiguous(code, file_pages);
        return 0;
    }

    // Align to a page boundary
    return ((read + (u32)AKAI_PROCESS_BASE) + 0xFFFU) & ~0xFFFU;
}

// returns false on invalid header or failure
static bool parse_aout_header(FIL *f, ExecHeader *out)
{
    u32  header[8];
    UINT read;

    _trace(0xd1e1, 1);
    if (f == NULL || out == NULL) return false;

    _trace(0xd1e1, 2);
    if (f_read(f, header, 32, &read) != FR_OK) return false;

    _trace(0xd1e1, 3);
    if (read < 32) return false;

    out->magic = header[0];
    _trace(0xd1e1, 4, out->magic, EXEC_MAGIC);
    if (out->magic != EXEC_MAGIC) return false;
    out->csize   = header[1];
    out->dsize   = header[2];
    out->bsize   = header[3];
    out->crsize  = header[4];
    out->drsize  = header[5];
    out->symsize = header[6];
    out->strsize = header[7];

    /* perform some sanity checks. We dont allow non-static executables, so the relocation and
     * symbol tables should be 0, but lets keep this parser general. However, I dont think a 0 code
     * size is allowed.
     */

    _trace(0xd1e1, 5);
    if (out->csize == 0) return false;

    _trace(0xd1e1, 0xA);
    return true;
}

static bool check_static_aout(FIL *f, ExecHeader *hdr)
{
    usize sz             = f_size(f);
    usize exepected_size = hdr->csize + hdr->dsize + 32;

    if (hdr->crsize != 0 || hdr->drsize != 0 || hdr->symsize != 0 || hdr->strsize != 0) {
        // Only allow static executables;
        return false;
    }

    // maybe I'm paranoid
    if (sz > exepected_size) {
        return false;
    }

    return true;
}

static u32 load_aout_aligned(FIL *f)
{
    // TODO: implement this loader for 4KB aligned sections, as it can overcome fragmentation
    return 0;
}

static u32 load_aout_flat(FIL *f)
{
    ExecHeader hdr;
    usize      sz = f_size(f);
    usize      file_pages, code_pages, databss_pages, bss_pages;
    u8        *aout_pages, *code, *data;
    u32        code_end, data_end;
    u32        data_offset_in_page;
    bool       overlap;

    _trace(0xd1e0, 1);
    if (sz > MAX_EXEC_FSIZE) return 0;

    _trace(0xd1e0, 2);
    if (!parse_aout_header(f, &hdr)) return 0;
    _trace(0xd1e0, 3);
    if (!check_static_aout(f, &hdr)) return 0;

    // we consider the file valid. Note it must be relocated to 0x80000 or be PIC, but we cant
    // ensure that

    file_pages = (hdr.csize + hdr.dsize + hdr.bsize + (PAGE_SIZE - 1)) >> 12;
    aout_pages = alloc_pages_contiguous(A.pr.curr->pid, file_pages);
    _trace(0xd1e0, 4);
    if (!aout_pages) return 0;

    code_end            = AKAI_PROCESS_BASE + hdr.csize;
    data_offset_in_page = code_end & 0xFFF;
    data_end            = code_end + hdr.dsize;
    code_pages          = (hdr.csize + (PAGE_SIZE - 1)) >> 12;
    databss_pages       = (data_offset_in_page + hdr.dsize + hdr.bsize + (PAGE_SIZE - 1)) >> 12;
    bss_pages           = (hdr.bsize + (PAGE_SIZE - 1)) >> 12;
    overlap             = data_offset_in_page != 0;

    code = aout_pages;
    _trace(0xd1e0, 5, overlap);
    /*
        miniprint("[KERNEL] Loading a.out executable:\n");
        miniprint("     pages: %d, code: %d, data+bss: %d\n", file_pages, code_pages,
       databss_pages); miniprint("     [CODE] start: 0x%06x, end: 0x%06x, size: %d\n",
       AKAI_PROCESS_BASE, code_end, hdr.csize); miniprint("     [DATA] start: 0x%06x, end: 0x%06x,
       size: %d\n", code_end, data_end, hdr.dsize); miniprint("     [BSS]  start: 0x%06x, end:
       0x%06x, size: %d\n", data_end, data_end + hdr.bsize, hdr.bsize); miniprint("     total size:
       %d, overlap: %d, bss pages: %d\n", sz, overlap, bss_pages);
    */

    if (code_pages > (overlap ? 1 : 0)) {
        usize pages_to_map = code_pages - (overlap ? 1 : 0);
        _trace(0xd1e1, 1, pages_to_map, code);
        if (!map_active_range((u32)code, AKAI_PROCESS_BASE, pages_to_map, PTE_U | PTE_RX))
            goto fail;

        // miniprint("[KERNEL] Mapping code pages 0x%06x to 0x%06x RX\n", AKAI_PROCESS_BASE,
        //           AKAI_PROCESS_BASE + (pages_to_map * PAGE_SIZE));
    }

    if (overlap) {
        u32 overlap_vaddr = (u32)(AKAI_PROCESS_BASE + (code_pages - 1) * PAGE_SIZE);
        u32 overlap_paddr = (u32)(code + (code_pages - 1) * PAGE_SIZE);
        _trace(0xd1e0, 6, databss_pages);
        _trace(0xd1e1, 1, overlap_paddr, overlap_vaddr);
        if (!map_active_pt_entry(overlap_paddr, overlap_vaddr, PTE_U | PTE_RWX)) goto fail;
        // miniprint("[KERNEL] Overlapping page 0x%06x remammped RWX\n", overlap_vaddr);
    }

    if (databss_pages > (overlap ? 1 : 0)) {
        usize remaining   = databss_pages - (overlap ? 1 : 0);
        u32   data_vstart = (u32)(AKAI_PROCESS_BASE + (code_pages * PAGE_SIZE));
        u32   data_pstart = (u32)(code + (code_pages * PAGE_SIZE));
        _trace(0xd1e0, 7, data_vstart, data_pstart);
        if (!map_active_range(data_pstart, data_vstart, remaining, PTE_U | PTE_RW)) goto fail;
        // miniprint("[KERNEL] Mapping data+bss pages 0x%06x to 0x%06x RX\n", data_vstart,
        //           data_vstart + (databss_pages * PAGE_SIZE));
    }

    _trace(0xd1e0, 8);
    if (f_lseek(f, 32) != FR_OK) goto fail;
    _trace(0xd1e0, 9);

    {
        usize bytes = hdr.csize + hdr.dsize;
        UINT  read;
        u32   bss_start = (u32)aout_pages + hdr.csize + hdr.dsize;
        u32   code_phys = (u32)aout_pages;
        usize bss_todo  = hdr.bsize;
        u8   *work      = map_kernel_work_area(code_phys);

        if (!work) goto fail;

        do {
            usize chunk = (bytes > PAGE_SIZE) ? PAGE_SIZE : bytes;
            if (f_read(f, work, chunk, &read) != FR_OK || read != chunk) {
                unmap_kernel_work_area();
                goto fail;
            }
            bytes -= chunk;
            code_phys += chunk;
            if (chunk == PAGE_SIZE) work = remap_kernel_work_area(code_phys);
        } while (bytes != 0);

        while (bss_todo > 0) {
            u32   offset = bss_start & 0xFFF;
            usize chunk  = (bss_todo > (PAGE_SIZE - offset)) ? (PAGE_SIZE - offset) : bss_todo;

            work = remap_kernel_work_area(bss_start & ~0xFFFU);
            memset(work + offset, 0, chunk);

            bss_todo -= chunk;
            bss_start += chunk;
        }

        unmap_kernel_work_area();
    }

    _trace(0xd1e0, 0xA, (data_end + hdr.bsize + 0xFFF) & ~0xFFFU);
    // miniprint("[KERNEL] Setting BRK to 0x%06x\n", (data_end + hdr.bsize + 0xFFF) & ~0xFFFU);

    return (data_end + hdr.bsize + 0xFFF) & ~0xFFFU;

fail:
    free_pages_contiguous(aout_pages, file_pages);
    return 0;
}

static bool copy_argv(u32 *stack_page, u32 argc, char *path, char **argv, u32 *out_argv)
{
    u32   strings[MAX_EXEC_ARGS + 1];
    usize args   = argc + 1;
    u8   *work   = map_kernel_work_area((u32)stack_page);
    u8   *cursor = work + PAGE_SIZE - 1;
    i32   i, len;

    if (!work) return false;

    if (args > MAX_EXEC_ARGS) goto fail;

    for (i = args - 1; i >= 0; i--) {
        char *src;
        _trace(0xEEFF, i, cursor);
        if (i == 0) {
            src = path;
        } else {
            if (argv == NULL) goto fail;
            src = argv[i - 1];
        }

        len = strlen_max(src, MAX_EXEC_ARG_LEN) + 1; // include null
        if (len <= 0) goto fail;

        cursor -= len;
        if (cursor < work + (args * 4) + 8) goto fail;
        memcpy(cursor, src, len);
        strings[i] = AKAI_PROCESS_STACK_TOP - (u32)((work + PAGE_SIZE) - cursor);
    }

    cursor = (u8 *)((u32)cursor & ~15U); // sp is 16 byte aligned
    cursor -= 4;
    *(u32 *)cursor = 0;

    for (i = args - 1; i >= 0; i--) {
        cursor -= 4;
        *(u32 *)cursor = strings[i];
    }

    *out_argv = (AKAI_PROCESS_STACK_TOP - PAGE_SIZE) + (u32)(cursor - work);

    unmap_kernel_work_area();
    return true;

fail:
    unmap_kernel_work_area();
    return false;
}

// gets a process a new name based on the executable path
static void get_new_name(const char *path, char *newname)
{
    const char *last_slash = strrchr(path, '/');
    const char *filename   = last_slash ? (last_slash + 1) : path;

    usize i;
    for (i = 0; i < 9 && filename[i] != '\0'; i++) {
        newname[i] = filename[i];
    }

    newname[i] = '\0';
}

static i32 ak_exit(u32 *win)
{
    u32 exit_code = win[13];

    _trace(0xe1, exit_code);

    A.pr.curr->exit_code = exit_code;

    _trace(0xD00DA, A.pr.curr->last_error);
    //miniprint("Process %d (%s) exiting with code %d\n", A.pr.curr->pid, A.pr.curr->name, exit_code);

    process_terminate(A.pr.curr->pid, WARRANT_NONE);

    process_yield();
    return (signed)A_OK;
}

static i32 ak_yield(u32 *win)
{
    static yields = 0;
    _trace(0xe2, yields++);
    if (A.pr.curr->pid != 0) {
        puts("Yield ");
        puts(A.pr.curr->name);
        puts("\n");
    }
    process_yield();
    return (signed)A_OK;
}

static i32 ak_rfork(u32 *win)
{
    u32 flags    = win[13];
    u32 heirloom = win[14];

    ProcessPID child;
    int        parent_state;
    usize      i;
    int        td = 0;

    u32  mem     = flags & RF_MEM_CFG;
    u32  fil     = flags & RF_FIL_CFG;
    u32  par     = flags & RF_PARENT_CFG;
    int  stopped = flags & RF_CHILD_WAIT; // REMEMBER: bool is just a typedef to i8
    bool fresh   = false;

    // flags validation
    bool mem_ok = (mem == 0) || !(mem & (mem - 1));
    bool fil_ok = fil && !(fil & (fil - 1));
    bool par_ok = par && !(par & (par - 1));

    if (!mem_ok || !fil_ok || !par_ok) {
        _trace(0xDADFED, 0X3000);
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    // heirloom validation
    if (heirloom >= (1U << _DEV_NUM)) {
        _trace(0xDADFED, 0X3001);
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    for (i = 0; i < _DEV_NUM; i++) {
        if ((1U << i) & heirloom) {
            if (A.pr.curr->pid != DEED_OWNER(&A.devices[i].deed)) {
                err = A_ERROR_CLAIM;
                return (signed)A_ERROR_CLAIM;
            }
        }
    }

    // spawning stopped and not running the parent creates a deadlock
    if (stopped && par != RF_PARENT_KEEP_RUNNING) {
        _trace(0xDADFED, 0X3002);
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    // create a generic process first
    child = process_create(A.pr.curr->name, (ProcessEntry)AKAI_PROCESS_BASE);
    if (!child) {
        err = P_ERROR_NO_PID;
        return (signed)A_ERROR;
    }

    // now we have a new process, fill based on the flags
    _trace(0xDAFDE11, child, A.pr.proc[child].ctx.status, A.pr.proc[child].pdt);
    _trace(0xDAFDE11, mem);
    switch (mem) {
    case RF_MEM_COPY:
        _trace(0xDAFDE12, mem);
        /* easy, just copy all memory and registers */
        if (!process_clone_memory(child, A.pr.curr->pid)) goto fork_error;
        break;
    case RF_MEM_SHARE:
        if (!process_share_memory(child, A.pr.curr->pid)) goto fork_error;
        break;
    case RF_MEM_FRESH:
        /* it's already fresh! //TODO: some bits may be missing  */
        fresh = true;
        break;
    default:
        process_reap(child);
        _trace(0xDADFED, 0X3003);
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }
    _trace(0xDAFDE11, child, A.pr.proc[child].ctx.status, A.pr.proc[child].pdt);

    switch (fil) {
    case RF_FIL_SHARE:
        /* easy, just copy the table, and bump the refs */
        process_share_files(child, A.pr.curr->pid);
        break;
    case RF_FIL_CLEAN:
        /* Already clean */
        break;
    default:
        process_reap(child);
        _trace(0xDADFED, 0X3005);
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    _trace(0xDAFDE11, child, A.pr.proc[child].ctx.status, A.pr.proc[child].pdt);

    switch (par) {
    case RF_PARENT_KEEP_RUNNING: parent_state = PARENT_KEEP_RUNNING; break;
    case RF_PARENT_WAIT: parent_state = PARENT_WAIT; break;
    case RF_PARENT_DIE: parent_state = PARENT_DIE; break;
    case RF_PARENT_DETACH: parent_state = PARENT_DETACH; break;
    default:
        _trace(0xDADFED, 0X3006);
        process_reap(child);
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (flags & RF_LEASE_STDIN) {
        dev_lease(A.pr.curr->pid, child, PDEV_STDIN);
        proxy_attach(child, PDEV_STDIN, &A.pr.curr->stdin);
    }

    if (flags & RF_LEASE_STDOUT) {
        dev_lease(A.pr.curr->pid, child, PDEV_STDOUT);
        proxy_attach(child, PDEV_STDOUT, &A.pr.curr->stdout);
    }

    if (flags & RF_LEASE_STDERR) {
        dev_lease(A.pr.curr->pid, child, PDEV_STDERR);
        proxy_attach(child, PDEV_STDERR, &A.pr.curr->stderr);
    }

    for (i = 0; i < _DEV_NUM; i++) {
        if ((1U << i) & heirloom) dev_lease(A.pr.curr->pid, child, i);
    }

    if (!fresh) {
        u32 child_status = A.pr.proc[child].ctx.status; // Cache the status!!!
        memcpy(&A.pr.proc[child].ctx, &A.pr.curr->ctx, sizeof(struct ThreadCtx));
        memcpy(A.pr.proc[child].subs, A.pr.curr->subs, sizeof(A.pr.proc[child].subs));
        A.pr.proc[child].brk                 = A.pr.curr->brk;
        A.pr.proc[child].inbox               = A.pr.curr->inbox;
        A.pr.proc[child].outbox              = A.pr.curr->outbox;
        A.pr.proc[child].flags               = A.pr.curr->flags;
        A.pr.proc[child].event_handler       = A.pr.curr->event_handler;
        A.pr.proc[child].event_mask          = A.pr.curr->event_mask;
        A.pr.proc[child].pending_events      = A.pr.curr->pending_events;
        A.pr.proc[child].message_queue_flags = A.pr.curr->message_queue_flags;
        A.pr.proc[child].ctx.regs[10]        = 0;            // fork returns 0;
        A.pr.proc[child].ctx.status          = child_status; // RESTORE THE STATUS!!
    }

    A.pr.proc[child].user_ticks   = 0;
    A.pr.proc[child].system_ticks = 0;
    A.pr.proc[child].parent_state = parent_state;

    /* we have it! */
    /* If parent asked to stop the child, do so */
    if (stopped) {
        process_wait(child);
    } else if (parent_state != PARENT_KEEP_RUNNING) {
        process_run(child, parent_state);
    }

    return child;

fork_error:
    process_reap(child);
    err = A_ERROR;
    return (signed)A_ERROR;
}

static i32 ak_exec(u32 *win)
{
    FIL     executable;
    FRESULT res;
    u32     out_argv;
    char    new_name[10];

    char  *path   = (char *)win[13];
    u32    argc   = win[14];
    char **argv   = (char **)win[15];
    u32    flags  = win[16];
    u8     format = flags & EXEC_FILE_FORMAT_MASK;

    _trace(0XEE, argc, flags);

    if (!process_check_addr(A.pr.curr->pid, (u32)path, PTE_V | PTE_R, false) ||
        (u32)path < AKAI_PROCESS_BASE || (u32)path >= AKAI_PROCESS_STACK_TOP) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR;
    }

    if (argc > 0 || (u32)argv != 0) {
        if (!process_check_addr(A.pr.curr->pid, (u32)argv, PTE_V | PTE_R, false) ||
            (u32)argv < AKAI_PROCESS_BASE || (u32)argv >= AKAI_PROCESS_STACK_TOP) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR;
        }
    }

    // Check if file exists, first thing
    res = f_open(&executable, path, FA_READ | FA_OPEN_EXISTING);
    if (res) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    if (format == O_EXEC_GUESS) {
        int guessed = detect_exec_format(&executable);
        if (guessed < 0) {
            // miniprint("[KERNEL] Could not guess format\n");
            f_close(&executable);
            err = P_ERROR_NO_EXEC;
            return (signed)A_ERROR;
        }

        format = (u8)guessed;
        // miniprint("[KERNEL] Guessed format %d (%d)\n", format, guessed);
    }

    // load
    {
        // deallocate current process to be efficient
        u32 brk;
        u8 *start_page;
        u8 *stack_page = alloc_pages_contiguous(A.pr.curr->pid, 1);

        if (stack_page == NULL) {
            f_close(&executable);
            err = A_ERROR_OOM;
            return (signed)A_ERROR;
        }

        get_new_name(path, new_name);

        // validate and copy the args to the stack page
        if (!copy_argv((u32 *)stack_page, argc, path, argv, &out_argv)) {
            f_close(&executable);
            err = P_ERROR_BAD_ARGV;
            return (signed)A_ERROR;
        };

        // miniprint("[KERNEL] copied argc and argv to stack: new -> 0x%06x\n", out_argv);

        // orphan the stack page

        page_transfer(ORPHAN_PID, stack_page);
        if (!process_reset(A.pr.curr->pid, new_name, AKAI_PROCESS_BASE, (out_argv - 8) & ~0xFLU,
                           (ProcessEntry)AKAI_PROCESS_BASE, 0x4)) {
            miniprint("[KERNEL] killing %s, error reset\n", A.pr.curr->name);
            f_close(&executable);
            free_page(stack_page);
            process_terminate(A.pr.curr->pid, WARRANT_ERROR);
            process_yield();
            return (signed)A_ERROR;
        };
        // transfer back the stack page, and map it to the process
        page_transfer(A.pr.curr->pid, stack_page);

        map_active_pt_entry((u32)stack_page, AKAI_PROCESS_STACK_TOP - PAGE_SIZE, PTE_U | PTE_RW);
        _trace(0xfaaaa);

        f_rewind(&executable);
        // miniprint("[KERNEL] Loading %s, format %d\n", A.pr.curr->name, format);
        switch (format) {
        case O_EXEC_BIN_SIGNED:
        case O_EXEC_BIN: brk = load_flat_binary(&executable); break;
        case O_EXEC_AOUT: brk = load_aout_flat(&executable); break;
        case O_EXEC_AOUT_FLAT: brk = load_aout_flat(&executable); break;
        default: brk = 0; break;
        }

        if (!brk) {
            _trace(0xd1d0);
            miniprint("[KERNEL] killing %s, error wrong exectuable\n", A.pr.curr->name);
            f_close(&executable);
            process_terminate(A.pr.curr->pid, WARRANT_ERROR);
            process_yield();
            return (signed)A_ERROR;
        }

        _trace(0xe1e1e1e);
        A.pr.curr->ctx.regs[12] = argc + 1;
        A.pr.curr->ctx.regs[13] = out_argv;
        A.pr.curr->brk          = (void *)brk;
        f_close(&executable);
        process_yield();
        return (signed)A_OK;
    }
}

static i32 collect_pid(ProcessPID pid, int *status, bool any)
{
    if (any) {
        usize i;
        for (i = 1; i < MAX_PROCESS; i++) {
            if (A.pr.proc[i].state == ZOMBIE && A.pr.proc[i].parent == A.pr.curr->pid) {
                if (status) *status = A.pr.proc[i].exit_code;
                process_reap(i);
                return i;
            }
        }
    } else {
        if (A.pr.proc[pid].state == ZOMBIE && A.pr.proc[pid].parent == A.pr.curr->pid) {
            if (status) *status = A.pr.proc[pid].exit_code;
            process_reap(pid);
            return pid;
        }
    }

    return -1;
}

static i32 ak_wait(u32 *win)
{
    i32  child_pid = win[13];
    i32 *status    = (i32 *)win[14];
    u32  options   = win[15];

    i32   res;
    usize i;
    bool  any = child_pid == -1, has_children = false;

    if (status &&
        !process_check_addr(A.pr.curr->pid, (u32)status, PTE_U | PTE_V | PTE_R | PTE_W, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR;
    }

    if (!any && ((child_pid == KERNEL_PID) || (child_pid >= MAX_PROCESS) ||
                 (child_pid == A.pr.curr->pid))) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (!any &&
        (A.pr.proc[child_pid].state == FREE || A.pr.proc[child_pid].parent != A.pr.curr->pid)) {
        err = P_ERROR_NOT_CHILD;
        return (signed)A_ERROR_INVAL;
    }

    for (i = 1; i < MAX_PROCESS; i++) {
        if (A.pr.proc[i].state != FREE && A.pr.proc[i].parent == A.pr.curr->pid) {
            has_children = true;
            break;
        }
    }

    if (!has_children) {
        err = A_ERROR_NOCHILDREN;
        return (signed)A_ERROR_NOCHILDREN;
    }

    res = collect_pid(child_pid, status, any);

    if (res > 0) return res;

    if (options & WAIT_NOHANG) {
        return 0;
    } else if (options == WAIT_HANG) {
        A.pr.curr->waiting_on = child_pid;
        process_wait(A.pr.curr->pid);
        process_yield();
    } else {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    return (signed)A_ERROR_UNREACHEABLE;
}

static i32 ak_hook(u32 *win)
{
    u32 event_mask    = win[13];
    u32 event_handler = win[14];
    u32 result;

    _trace(0xe3, A.pr.curr->pid, event_mask, event_handler);

    if (A.pr.curr->pid == 0) {
        err = P_ERROR_IPC_PID0;
        return (signed)A_ERROR_IPC; // Do not hook the idle task
    }

    if ((A.pr.curr->flags & PROC_IPC) != PROC_IPC) {
        err = P_ERROR_IPC_NOINIT;
        return (signed)A_ERROR_IPC; // must initialize IPC.
    }

    result = A.pr.curr->event_mask;
    A.pr.curr->event_mask |= event_mask;
    A.pr.curr->event_handler = (void *)event_handler;
    return result;
}

static i32 ak_unhook(u32 *win)
{
    i32 result;
    u32 event_mask = win[13];

    _trace(0xe4, A.pr.curr->pid, event_mask);

    if (A.pr.curr->pid == 0) {
        err = P_ERROR_IPC_PID0;
        return (signed)A_ERROR_IPC;
    }

    if ((A.pr.curr->flags & PROC_IPC) != PROC_IPC) {
        err = P_ERROR_IPC_NOINIT;
        return (signed)A_ERROR_IPC; // must initialize IPC.
    }

    result = A.pr.curr->event_mask;
    A.pr.curr->event_mask &= ~event_mask;
    A.pr.curr->event_handler = NULL;
    return result;
}
static i32 ak_ipc_sub(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}
static i32 ak_ipc_unsub(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}

static i32 ak_ipc_init(u32 *win)
{
    i32  result;
    u8   hdrbuf[INBOX_HEADER_SIZE];
    u32 *inbox_page = alloc_pages_contiguous(A.pr.curr->pid, 1);

    if (!inbox_page) {
        // out of memory
        err = P_ERROR_IPC_INIT;
        return (signed)A_ERROR_OOM;
    }

    // the page tables should be mapped in the process' space per process_create
    map_pt_entry((u32 *)AKAI_PROCESS_PT3, AKAI_IPC_INBOX >> 12, (u32)inbox_page,
                 PTE_V | PTE_U | PTE_R | PTE_W);
    tlb_flush();
    memset((u8 *)AKAI_IPC_INBOX, 0, PAGE_SIZE);

    A.pr.curr->flags |= PROC_IPC;
    A.pr.curr->inbox = inbox_page;

    memset(hdrbuf, 0, INBOX_HEADER_SIZE);
    hdrbuf[INBOX_HEADER_QUEUE_MAX]     = (u8)INBOX_QUEUE_MAX >> 8;
    hdrbuf[INBOX_HEADER_QUEUE_MAX + 1] = (u8)INBOX_QUEUE_MAX;
    memcpy((u8 *)AKAI_IPC_INBOX, hdrbuf, INBOX_HEADER_SIZE);
    return (signed)A_OK;
}

static i32 ak_ipc_recv(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}
static i32 ak_ipc_send(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}

static i32 ak_dev_ctl(u32 *win)
{
    u32 devnum  = win[13];
    u32 command = win[14];
    u8 *buff    = (u8 *)win[15];
    u32 len     = win[16];

    u32  buff_end = (u32)buff + len - (len ? 1 : 0);
    bool vdev     = devnum >= _VDEV_START && devnum < _VDEV_NUM;
    bool proxy    = devnum >= PDEV_STDIN && devnum <= PDEV_STDERR;
    bool owned    = vdev || proxy;
    u32  addr;

    if (!owned && devnum >= _DEV_NUM) {
        _trace(0xFEDB, 0X1);
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (!owned && DEED_OWNER(&A.devices[devnum].deed) != A.pr.curr->pid) {
        err = A_ERROR_CLAIM;
        return (signed)A_ERROR_CLAIM;
    }

    if (len > 0) {
        if (len > 10 * 1024 * 1024) {
            // arbitrary limit of 10 megs per read
            _trace(0xFEDB, 0X2);

            err = P_ERROR_FILE_TOO_LARGE;
            return (signed)A_ERROR_INVAL;
        }

        if (buff + len < buff) {
            _trace(0xFEDB, 0X3);
            err = P_ERROR_FILE_TOO_LARGE;
            return (signed)A_ERROR_INVAL;
        }

        if (!process_check_addr(A.pr.curr->pid, (u32)buff, PTE_U | PTE_V | PTE_R | PTE_W, false) ||
            !process_check_addr(A.pr.curr->pid, (u32)buff_end, PTE_U | PTE_V | PTE_R | PTE_W,
                                false)) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_INVAL;
        }

        // Check page boundaries if the buffer spans more than two pages
        for (addr = ((u32)buff & ~(PAGE_SIZE - 1)) + PAGE_SIZE; addr < (u32)buff + len;
             addr += PAGE_SIZE) {
            if (!process_check_addr(A.pr.curr->pid, addr, PTE_U | PTE_V | PTE_R | PTE_W, false)) {
                err = P_ERROR_BAD_POINTER;
                return (signed)A_ERROR_INVAL;
            }
        }
    }

    _trace(0xdd1deda, devnum);
    if (proxy)
        return proxy_ctl(devnum, command, buff, len);
    else if (vdev)
        return vdev_ctl(devnum, command, buff, len);
    else
        return (signed)A.devices[devnum].ctl(command, buff, len);
}

static i32 ak_dev_in(u32 *win)
{
    u32 devnum = win[13];
    u8  port   = win[14];

    bool vdev  = devnum >= _VDEV_START && devnum < _VDEV_NUM;
    bool proxy = devnum >= PDEV_STDIN && devnum <= PDEV_STDERR;
    bool owned = vdev || proxy;

    if (!owned && devnum >= _DEV_NUM) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (!owned && DEED_OWNER(&A.devices[devnum].deed) != A.pr.curr->pid) {
        err = A_ERROR_CLAIM;
        return (signed)A_ERROR_CLAIM;
    }

    if (!owned && A.devices[devnum].ports < port) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (proxy)
        return proxy_in(devnum, port);
    else if (vdev)
        return vdev_in(devnum, port);
    else
        return (signed)A.devices[devnum].in(port);
}

static i32 ak_dev_out(u32 *win)
{
    u32 devnum = win[13];
    u8  port   = win[14];
    u8  val    = win[15];

    bool vdev  = devnum >= _VDEV_START && devnum < _VDEV_NUM;
    bool proxy = devnum >= PDEV_STDIN && devnum <= PDEV_STDERR;
    bool owned = vdev || proxy;

    if (!owned && devnum >= _DEV_NUM) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (!owned && DEED_OWNER(&A.devices[devnum].deed) != A.pr.curr->pid) {
        err = A_ERROR_CLAIM;
        return (signed)A_ERROR_CLAIM;
    }

    if (!owned && A.devices[devnum].ports < port) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (proxy) {
        _trace(0xd1d1d1, val);
        proxy_out(devnum, port, val);
    } else if (vdev)
        vdev_out(devnum, port, val);
    else
        A.devices[devnum].out(port, val);

    return (signed)A_OK;
}

static i32 ak_dev_claim(u32 *win)
{
    u32 devnum = win[13];

    bool vdev  = devnum >= _VDEV_START && devnum < _VDEV_NUM;
    bool proxy = devnum >= PDEV_STDIN && devnum <= PDEV_STDERR;
    bool owned = vdev || proxy;

    if (owned) return (signed)A_OK;

    if (devnum >= _DEV_NUM) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (!DEED_UNCLAIMED(&A.devices[devnum].deed)) {
        err = A_ERROR_CLAIM;
        return (signed)A_ERROR_CLAIM;
    }
    
    A.devices[devnum].deed.lineage[0] = A.pr.curr->pid;
    A.devices[devnum].deed.depth++;

    return (signed)A_OK;
}

static i32 ak_open(u32 *win)
{
    const char *path  = (const char *)win[13];
    int         flags = win[14];

    u32     new_open_files = A.pr.curr->open_files + 1;
    i16     fd             = -1;
    FIL    *f              = NULL;
    FRESULT res            = 0;
    int     fatfs_flags    = 0;
    usize   f_index;
    usize   i;

    if (new_open_files > MAX_OPEN_FILES) {
        err = P_ERROR_TOO_MANY_FILES;
        return (signed)A_ERROR_OOF;
    }

    for (i = 0; i < MAX_OPEN_FILES; i++) {
        if (A.pr.curr->fds[i] == FREE_FD) {
            fd = i;
            break;
        }
    }

    if (fd == -1) {
        err = P_ERROR_TOO_MANY_FILES;
        return (signed)A_ERROR_OOF;
    }

    for (i = 0; i < FILE_POOL_MAX; i++) {
        if (A.fp.refs[i] == 0) {
            f_index = i;
            f       = &A.fp.files[i];
            break;
        }
    }

    if (!f) {
        err = P_ERROR_FILE_POOL_EXHAUSTED;
        return (signed)A_ERROR_OOF;
    }

    if (!process_check_addr(A.pr.curr->pid, (u32)path, PTE_V | PTE_R | PTE_U, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    if (flags & O_READ) fatfs_flags |= FA_READ;
    if (flags & O_WRITE) fatfs_flags |= FA_WRITE;
    if (flags & O_OPEN_EXISTING) fatfs_flags |= FA_OPEN_EXISTING;
    if (flags & O_CREATE_ALWAYS) fatfs_flags |= FA_CREATE_ALWAYS;
    if (flags & O_OPEN_ALWAYS) fatfs_flags |= FA_OPEN_ALWAYS;
    if (flags & O_APPEND) fatfs_flags |= FA_OPEN_APPEND;

    if ((res = f_open(f, path, fatfs_flags)) != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    A.fp.refs[f_index]    = 1;
    A.pr.curr->fds[fd]    = f_index;
    A.pr.curr->open_files = new_open_files;
    return fd;
}

static i32 ak_dup(u32 *win)
{
    return (signed)A_ERROR;
}

static i32 ak_close(u32 *win)
{
    i16 fd = win[13];

    i32     new_open_files = A.pr.curr->open_files - 1;
    FIL    *f              = NULL;
    FRESULT res            = 0;
    usize   f_index;
    usize   i, refs;

    if (new_open_files < 0) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    f_index = A.pr.curr->fds[fd];

    if (f_index < 0 || f_index >= FILE_POOL_MAX) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    f    = &A.fp.files[f_index];
    refs = A.fp.refs[f_index];

    if (refs == 0) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (refs > 1) {
        A.fp.refs[f_index] = refs - 1;
    } else {
        res = f_close(f);
        if (res != FR_OK) {
            err = FS_ERROR | res;
            return (signed)A_ERROR;
        }

        A.fp.refs[f_index] = 0;
    }

    A.pr.curr->open_files = new_open_files;
    A.pr.curr->fds[fd]    = FREE_FD;
    return (signed)A_OK;
}

static i32 ak_unlink(u32 *win)
{
    // TODO: insecure becasuse of path
    const char *path = (const char *)win[13];
    u8          drive;
    FRESULT     res;

    if (!process_check_addr(A.pr.curr->pid, (u32)path, PTE_V | PTE_R | PTE_U, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    drive = path_get_drive(path);

    // how do I check if the file is opened?
    if (drive == 0 && !(A.pr.curr->pid <= 1)) {
        // Only the kernel or pid 1 (the shell/executor) can remove files from TPS A
        err = P_ERROR_NO_PERM;
        miniprint("FORBIDDEN, drive = %d, pid = %d\n", drive, A.pr.curr->pid);

        return (signed)A_ERROR_FORBIDDEN;
    }

    res = f_unlink(path);
    if (res != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    return (signed)A_OK;
}
static i32 ak_rename(u32 *win)
{
    // TODO: insecure becasuse of path
    const char *oldname = (const char *)win[13];
    const char *newname = (const char *)win[14];
    u8          drive;
    FRESULT     res;

    if (!process_check_addr(A.pr.curr->pid, (u32)oldname, PTE_V | PTE_R | PTE_U, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)newname, PTE_V | PTE_R | PTE_U, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    drive = path_get_drive(oldname);

    if (drive == 0 && !(A.pr.curr->pid <= 1)) {
        // Only the kernel or pid 1 (the shell/executor) can rename files from TPS A
        err = P_ERROR_NO_PERM;
        miniprint("FORBIDDEN, drive = %d, pid = %d\n", drive, A.pr.curr->pid);
        return (signed)A_ERROR_FORBIDDEN;
    }

    res = f_rename(oldname, newname);
    if (res != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    return (signed)A_OK;
}
static i32 ak_mkdir(u32 *win)
{
    // TODO: insecure becasuse of path
    const char *path = (const char *)win[13];
    u8          drive;
    FRESULT     res;

    if (!process_check_addr(A.pr.curr->pid, (u32)path, PTE_V | PTE_R | PTE_U, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    drive = path_get_drive(path);

    // how do I check if the file is opened?
    if (drive == 0 && !(A.pr.curr->pid <= 1)) {
        // Only the kernel or pid 1 (the shell/executor) can remove files from TPS A
        err = P_ERROR_NO_PERM;
        miniprint("FORBIDDEN, drive = %d, pid = %d\n", drive, A.pr.curr->pid);
        return (signed)A_ERROR_FORBIDDEN;
    }

    res = f_mkdir(path);
    if (res != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    return (signed)A_OK;
}

static i32 ak_chmod(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}
static i32 ak_utime(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}

static i32 ak_read(u32 *win)
{
    int fd   = win[13];
    u8 *buff = (u8 *)win[14];
    u32 btr  = win[15];

    i16     f_index = A.pr.curr->fds[fd];
    FIL    *f;
    FRESULT res = 0;
    UINT    br  = 0;
    usize   i;
    u32     addr;

    if (f_index == FREE_FD || A.fp.refs[f_index] == 0) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    f = &A.fp.files[f_index];

    if (btr > 10 * 1024 * 1024) {
        // arbitrary limit of 10 megs per read
        err = P_ERROR_FILE_TOO_LARGE;
        return (signed)A_ERROR_INVAL;
    }

    if (buff + btr < buff) {
        // arbitrary limit of 10 megs per read
        err = P_ERROR_FILE_TOO_LARGE;
        return (signed)A_ERROR_INVAL;
    }

    if (btr == 0) return 0;

    if (!process_check_addr(A.pr.curr->pid, (u32)buff, PTE_U | PTE_V | PTE_W, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)buff + btr - 1, PTE_U | PTE_V | PTE_W, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    // Check page boundaries if the buffer spans more than two pages
    for (addr = ((u32)buff & ~(PAGE_SIZE - 1)) + PAGE_SIZE; addr < (u32)buff + btr;
         addr += PAGE_SIZE) {
        if (!process_check_addr(A.pr.curr->pid, addr, PTE_U | PTE_V | PTE_W, false)) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_INVAL;
        }
    }

    res = f_read(f, buff, btr, &br);

    if (res != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    return br;
}
static i32 ak_write(u32 *win)
{
    int fd   = win[13];
    u8 *buff = (u8 *)win[14];
    u32 btw  = win[15];

    i16     f_index = A.pr.curr->fds[fd];
    FIL    *f;
    FRESULT res = 0;
    UINT    bw  = 0;
    usize   i;
    u32     addr;

    if (f_index == FREE_FD || A.fp.refs[f_index] == 0) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    f = &A.fp.files[f_index];

    if (btw > 10 * 1024 * 1024) {
        // arbitrary limit of 10 megs per write
        err = P_ERROR_FILE_TOO_LARGE;
        return (signed)A_ERROR_INVAL;
    }

    if (buff + btw < buff) {
        // arbitrary limit of 10 megs per write
        err = P_ERROR_FILE_TOO_LARGE;
        return (signed)A_ERROR_INVAL;
    }

    if (btw == 0) return 0;

    if (!process_check_addr(A.pr.curr->pid, (u32)buff, PTE_U | PTE_V | PTE_R, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)buff + btw - 1, PTE_U | PTE_V | PTE_R, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    // Check page boundaries if the buffer spans more than two pages
    for (addr = ((u32)buff & ~(PAGE_SIZE - 1)) + PAGE_SIZE; addr < (u32)buff + btw;
         addr += PAGE_SIZE) {
        if (!process_check_addr(A.pr.curr->pid, addr, PTE_U | PTE_V | PTE_R, false)) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_INVAL;
        }
    }

    res = f_write(f, buff, btw, &bw);

    if (res != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    return bw;
}
static i32 ak_seek(u32 *win)
{
    int fd     = win[13];
    i32 offset = win[14];
    int whence = win[15];

    i16     f_index = A.pr.curr->fds[fd];
    FIL    *f;
    FRESULT res    = 0;
    i32     target = 0;

    if (f_index == FREE_FD || A.fp.refs[f_index] == 0) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    f = &A.fp.files[f_index];

    if (whence == SEEK_SET)
        target = offset;
    else if (whence == SEEK_CUR)
        target = (i32)f_tell(f) + offset;
    else if (whence == SEEK_END)
        target = (i32)f_size(f) + offset;
    else {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (target < 0) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    res = f_lseek(f, (FSIZE_t)target);

    if (res != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    return f_tell(f);
}

static i32 ak_trunc(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}

static i32 ak_sync(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}

static i32 ak_set_preempt(u32 *win)
{
    u32 mode = win[13];

    if (mode > PREEMPT_ROBIN) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (mode == NO_PREEMPT) {
        timer_off();
    } else if (mode == PREEMPT_ROBIN) {
        timer_on();
    }

    return A_OK;
}

static i32 ak_opendir(u32 *win)
{
    DIR        *d     = (DIR *)win[13];
    const char *path  = (const char *)win[14];
    int         flags = win[15]; // ignored for now

    FRESULT res;

    if (flags != 0) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (!process_check_addr(A.pr.curr->pid, (u32)d, PTE_U | PTE_V | PTE_R | PTE_W, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)d + AKAI_DIR_SIZE - 1,
                            PTE_U | PTE_V | PTE_R | PTE_W, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    if (!process_check_addr(A.pr.curr->pid, (u32)path, PTE_V | PTE_R | PTE_U | PTE_W, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    res = f_opendir(d, path);

    if (res != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    return (signed)A_OK;
}

static i32 ak_closedir(u32 *win)
{
    DIR *d     = (DIR *)win[13]; //
    int  flags = win[14]; // ignored for now

    FRESULT res;

    if (flags != 0) {
        miniprint("closedir flags %d\n", flags);
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (!process_check_addr(A.pr.curr->pid, (u32)d, PTE_U | PTE_V | PTE_R | PTE_W, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)d + AKAI_DIR_SIZE - 1,
                            PTE_U | PTE_V | PTE_R | PTE_W, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    res = f_closedir(d);

    if (res != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    return (signed)A_OK;
}

static u32 fat_to_unix(u16 date, u16 time)
{
    u32 y_idx = (date >> 9) & 0x7F;
    u32 month = (date >> 5) & 0x0F;
    u32 day   = (date & 0x1F);
    u32 hour  = (time >> 11);
    u32 min   = (time >> 5) & 0x3F;
    u32 sec   = (time & 0x1F) * 2;

    u32              total_days;
    static const u16 mdays[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

    /* Get days for the year */
    total_days = days_start_of_year[y_idx];

    /* Add days for months */
    total_days += mdays[month - 1];

    /* Leap year adjustment: month > Feb AND current year is leap.
       In the range 1980-2107, year is leap if (y_idx % 4 == 0),
       EXCEPT for 2100 (y_idx == 120). */
    if (month > 2 && (y_idx % 4 == 0) && (y_idx != 120)) {
        total_days++;
    }

    total_days += (day - 1);

    return (total_days * 86400UL) + (hour * 3600UL) + (min * 60UL) + sec;
}

static u8 translate_attr(u8 fat_attr)
{
    u8 res = 0;
    if (fat_attr & AM_RDO) res |= AK_ATTR_READONLY;
    if (fat_attr & AM_HID) res |= AK_ATTR_HIDDEN;
    if (fat_attr & AM_SYS) res |= AK_ATTR_SYSTEM;
    if (fat_attr & AM_DIR) res |= AK_ATTR_DIR;
    if (fat_attr & AM_ARC) res |= AK_ATTR_ARCHIVE;
    return res;
}

static i32 ak_readdir(u32 *win)
{
    DIR                 *d     = (DIR *)win[13];
    struct AkaiDirEntry *info  = (struct AkaiDirEntry *)win[14];
    int                  flags = win[15]; // ignored for now

    FILINFO filinfo;
    FRESULT res;

    if (flags != 0) {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }

    if (!process_check_addr(A.pr.curr->pid, (u32)d, PTE_U | PTE_V | PTE_R | PTE_W, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)d + AKAI_DIR_SIZE - 1,
                            PTE_U | PTE_V | PTE_R | PTE_W, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    if (!process_check_addr(A.pr.curr->pid, (u32)info, PTE_U | PTE_V | PTE_R | PTE_W, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)info + AKAI_DIR_ENTRY_SIZE - 1,
                            PTE_U | PTE_V | PTE_R | PTE_W, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    res = f_readdir(d, &filinfo);

    if (res != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    memset(info, 0, AKAI_DIR_ENTRY_SIZE);
    ADIR_FSIZE(info->data)   = filinfo.fsize;
    ADIR_FMOD(info->data)    = fat_to_unix(filinfo.fdate, filinfo.ftime);
    ADIR_FCREAT(info->data)  = fat_to_unix(filinfo.crdate, filinfo.crtime);
    ADIR_FATTRIB(info->data) = translate_attr(filinfo.fattrib);
    ADIR_FS(info->data)      = d->obj.fs->fs_type;
    memcpy(ADIR_FNAME(info->data), filinfo.fname, sizeof(filinfo.fname));

    return (signed)A_OK;
}

static i32 ak_stat(u32 *win)
{
    const char          *path = (const char *)win[13];
    struct AkaiDirEntry *info = (struct AkaiDirEntry *)win[14];

    FILINFO filinfo;
    FRESULT res;

    if (!process_check_addr(A.pr.curr->pid, (u32)path, PTE_V | PTE_R | PTE_U | PTE_W, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    if (!process_check_addr(A.pr.curr->pid, (u32)info, PTE_U | PTE_V | PTE_R | PTE_W, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)info + AKAI_DIR_ENTRY_SIZE - 1,
                            PTE_U | PTE_V | PTE_R | PTE_W, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    res = f_stat(path, &filinfo);

    if (res != FR_OK) {
        if (res == FR_NO_FILE) {
            err = P_ERROR_NOENT;
            return (signed)A_ERROR;
        }

        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    memset(info, 0, AKAI_DIR_ENTRY_SIZE);
    ADIR_FSIZE(info->data)   = filinfo.fsize;
    ADIR_FMOD(info->data)    = fat_to_unix(filinfo.fdate, filinfo.ftime);
    ADIR_FCREAT(info->data)  = fat_to_unix(filinfo.crdate, filinfo.crtime);
    ADIR_FATTRIB(info->data) = translate_attr(filinfo.fattrib);
    ADIR_FS(info->data)      = 0xFF;
    memcpy(ADIR_FNAME(info->data), filinfo.fname, sizeof(filinfo.fname));

    return (signed)A_OK;
}

static i32 ak_chdir(u32 *win)
{
    const char *path = (const char *)win[13];
    DIR         d;
    FRESULT     res;

    u32 cluster;
    u8  drive;

    if (!process_check_addr(A.pr.curr->pid, (u32)path, PTE_V | PTE_R | PTE_U, false)) {
        return (signed)A_ERROR_INVAL;
    }

    res = f_opendir(&d, path);
    if (res != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }
    res = f_chdir(path);
    if (res != FR_OK) {
        f_closedir(&d);
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    cluster = d.obj.sclust;

    drive                         = path_get_drive(path);
    A.pr.curr->cwd_cluster[drive] = cluster;
    A.pr.curr->curr_drive         = drive;

    f_closedir(&d);
    return (signed)A_OK;
}

static i32 ak_getcwd(u32 *win)
{
    char *buff = (char *)win[13];
    u32   len  = win[14];

    FRESULT res;
    u32     addr;

    if (len > 10 * 1024 * 1024) {
        // arbitrary limit of 10 megs per read
        err = P_ERROR_FILE_TOO_LARGE;
        return (signed)A_ERROR_INVAL;
    }

    if (buff + len < buff) {
        // arbitrary limit of 10 megs per read
        err = P_ERROR_FILE_TOO_LARGE;
        return (signed)A_ERROR_INVAL;
    }

    if (len == 0) return 0;

    if (!process_check_addr(A.pr.curr->pid, (u32)buff, PTE_U | PTE_V | PTE_R | PTE_W, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)buff + len - 1, PTE_U | PTE_V | PTE_R | PTE_W,
                            false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    // Check page boundaries if the buffer spans more than two pages
    for (addr = ((u32)buff & ~(PAGE_SIZE - 1)) + PAGE_SIZE; addr < (u32)buff + len;
         addr += PAGE_SIZE) {
        if (!process_check_addr(A.pr.curr->pid, addr, PTE_U | PTE_V | PTE_R | PTE_W, false)) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_INVAL;
        }
    }

    res = f_getcwd(buff, len);
    if (res != FR_OK) {
        err = FS_ERROR | res;
        return (signed)A_ERROR;
    }

    return (signed)A_OK;
}

static i32 ak_mount(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}
static i32 ak_expand(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}
static i32 ak_forward(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}

// NOTE: for arcane reasons, sbrk returns (void*)-1. Lets keep to tradition
static i32 ak_sbrk(u32 *win)
{
    u32 increment     = win[13];
    u32 old_brk       = (u32)A.pr.curr->brk;
    u32 new_brk       = old_brk + increment;
    u32 curr_heap_top = PAGE_ALIGN_UP(old_brk);
    u32 new_heap_top  = PAGE_ALIGN_UP(new_brk);

    if (A.pr.curr == KERNEL_PID) return -1;

    if (increment == 0) return old_brk;

    if (new_brk < old_brk || increment > (8 * 1024 * 1024) || new_brk >= AKAI_PROCESS_END) {
        err = A_ERROR_INVAL;
        return -1;
    }

    if (new_heap_top > AKAI_PROCESS_END) {
        err = A_ERROR_OOM;
        return -1;
    }

    if (new_heap_top > curr_heap_top) {
        u32   vaddr = curr_heap_top;
        usize i;
        for (i = vaddr; i < new_heap_top; i += PAGE_SIZE) {
            u8 *frame = alloc_pages_contiguous(A.pr.curr->pid, 1);
            _trace(0xFDFDAA, i);
            if (!frame) {
                usize k;
rollback:
                for (k = vaddr; k < i; k += PAGE_SIZE) {
                    u8 *allocated_frame = active_phys_from_v(k);
                    unmap_active_pt_entry(k);
                    if (allocated_frame) free_page(allocated_frame);
                }
                err = A_ERROR_OOM;
                return -1;
            }
            if (!map_active_pt_entry((u32)frame, i, PTE_U | PTE_RW)) {
                free_page(frame);
                goto rollback;
            }
            memset((u8 *)i, 0, PAGE_SIZE);
        }
    }

    A.pr.curr->brk = (void *)new_brk;

    _trace(0xFDFDFD01, new_brk, old_brk);

    return old_brk;
}

static i32 ak_shm_make(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}
static i32 ak_shm_unmake(u32 *win)
{
    err = P_ERROR_NO_SYSCALL;
    return (signed)A_ERROR;
}

static i32 ak_error(u32 *win)
{
    ProcessPID pid = win[13];

    if (pid == KERNEL_PID)
        return err;
    else if (pid < MAX_PROCESS)
        return A.pr.proc[pid].last_error;
    else {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }
}

static i32 ak_getpid(u32 *win)
{
    (void)win;
    return A.pr.curr->pid;
}

static i32 ak_getppid(u32 *win)
{
    (void)win;
    return A.pr.curr->parent;
}

static i32 ak_abort(u32 *win)
{
    (void)win;
    process_terminate(A.pr.curr->pid, WARRANT_ABORT);
}

static i32 ak_time(u32 *win)
{
    (void)win;
    _sbd(REG_SYSTEM_YEAR, TALEA_SYSTEM_UNIXTIME_MODE);
    return _lwd(REG_SYSTEM_YEAR);
}

static i32 ak_clock(u32 *win)
{
    u32       *outuser   = (u32 *)win[13];
    u32       *outsystem = (u32 *)win[14];
    ProcessPID pid       = win[15];

    if (!process_check_addr(A.pr.curr->pid, (u32)outuser, PTE_U | PTE_V | PTE_R | PTE_W, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)outsystem, PTE_U | PTE_V | PTE_R | PTE_W, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR;
    }

    if (pid == KERNEL_PID) {
        *outuser   = A.pr.curr->user_ticks;
        *outsystem = A.pr.curr->system_ticks;
        return A.pr.curr->user_ticks + A.pr.curr->system_ticks;
    } else if (pid < MAX_PROCESS) {
        *outuser   = A.pr.proc[pid].user_ticks;
        *outsystem = A.pr.proc[pid].system_ticks;
        return A.pr.proc[pid].user_ticks + A.pr.proc[pid].system_ticks;
    } else {
        err = A_ERROR_INVAL;
        return (signed)A_ERROR_INVAL;
    }
}

static i32 ak_calendar(u32 *win)
{
    u32 *out1 = (u32 *)win[13];
    u32 *out2 = (u32 *)win[14];

    if (!process_check_addr(A.pr.curr->pid, (u32)out1, PTE_U | PTE_V | PTE_R | PTE_W, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)out2, PTE_U | PTE_V | PTE_R | PTE_W, false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR;
    }

    _sbd(REG_SYSTEM_YEAR, TALEA_SYSTEM_CALENDAR_MODE);
    *out1 = _lwd(REG_SYSTEM_YEAR);
    *out2 = _lhud(REG_SYSTEM_MINUTE);

    return A_OK;
}

static i32 ak_asm(u32 *win)
{
    const char *line      = (const char *)win[13];
    usize       line_len  = (usize)win[14];
    usize       curr_addr = (usize)win[15];
    u8         *out       = (u8 *)win[16];
    usize       out_sz    = (usize)win[17];

    usize addr;

    // check line
    if (!process_check_addr(A.pr.curr->pid, (u32)line, PTE_U | PTE_V | PTE_R, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)line + line_len - 1, PTE_U | PTE_V | PTE_R,
                            false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    // Check page boundaries if the buffer spans more than two pages
    for (addr = ((u32)line & ~(PAGE_SIZE - 1)) + PAGE_SIZE; addr < (u32)line + line_len;
         addr += PAGE_SIZE) {
        if (!process_check_addr(A.pr.curr->pid, addr, PTE_U | PTE_V | PTE_R, false)) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_INVAL;
        }
    }

    // check out
    if (!process_check_addr(A.pr.curr->pid, (u32)out, PTE_U | PTE_V | PTE_R | PTE_W, false) ||
        !process_check_addr(A.pr.curr->pid, (u32)out + out_sz - 1, PTE_U | PTE_V | PTE_R | PTE_W,
                            false)) {
        err = P_ERROR_BAD_POINTER;
        return (signed)A_ERROR_INVAL;
    }

    // Check page boundaries if the buffer spans more than two pages
    for (addr = ((u32)out & ~(PAGE_SIZE - 1)) + PAGE_SIZE; addr < (u32)out + out_sz;
         addr += PAGE_SIZE) {
        if (!process_check_addr(A.pr.curr->pid, addr, PTE_U | PTE_V | PTE_R | PTE_W, false)) {
            err = P_ERROR_BAD_POINTER;
            return (signed)A_ERROR_INVAL;
        }
    }

    return sirius_asm_line(line, line_len, curr_addr, out, out_sz);
}

static AkaiSyscall ak_syscalls[] = {
#define X(service, function, args, ret, doc) function,
    SYSCALL_LIST
#undef X
};

#define NUM_SYSCALLS (sizeof(ak_syscalls) / sizeof(AkaiSyscall))

i32 akai_syscall(u32 service)
{
    // parameters passed to syscall in window, registers x13 onwards
    u32 *win;
    i32  result = -1;

    u32 status = _lwd(AKAI_KERNEL_STATUS_SAVE);
    u32 pc     = _lwd(AKAI_KERNEL_PC_SAVE);

    _trace(0xFef0, service);
    save_ctx(A.pr.curr);
    _trace(0xFef1, A.pr.curr->ctx.regs[1]);

    win = A.pr.curr->ctx.regs;

    if (service < NUM_SYSCALLS && ak_syscalls[service] != NULL) {
        result = ak_syscalls[service](win);
    } else {
        result = (signed)A_ERROR;
        err    = P_ERROR_NO_SYSCALL;
    }

    _trace(0xEEAA, result, err);
    _trace(0x4504, status, pc);
    _swd(AKAI_KERNEL_STATUS_RESTORE, status);
    _swd(AKAI_KERNEL_PC_RESTORE, pc);

    return result;
}
