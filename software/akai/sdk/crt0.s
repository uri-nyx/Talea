.globl _program
.globl _exit
.globl _syscall
.globl main
.globl _trace
.text
_start:
    li x5, 0x11223344
    li x6, 0x55667788
    trace x5, x6, x2, x0
    call _program
    mv x12, x10
    call _exit

_exit:
    mv x13, x12
    mv x12, x0 # SYSCALL_EXIT
    syscall x0, 0x40

_syscall:
    syscall x0, 0x40
    ret
    
_trace:
    trace x12, x13, x14, x15
    ret

__float64_add:
__float64_sub:
__float64_mul:
__float64_div:
__float64_neg:
__float64_eq:
__float64_ne:
__float64_lt:
__float64_le:
__float64_gt:
__float64_ge:
__int32_to_float64:
__float64_to_int32:
__int64_to_float64:
__float64_to_int64:
__float64_to_float:
__float_to_float64:
__add64:
__sub64:
__and64:
__or64:
__xor64:
__not64:
__neg64:
    mv x10, x0
    mv x11, x0
    ret

.text
.globl ak_exit
ak_exit:
    mv x13, x12 # int exit_code
    li x12, 0
    syscall x0, 0x40 
    ret
    
.text
.globl ak_yield
ak_yield:

    li x12, 1
    syscall x0, 0x40 
    ret
    
.text
.globl ak_rfork
ak_rfork:
    mv x14, x13 # u32 heirloom
    mv x13, x12 # u32 flags
    li x12, 2
    syscall x0, 0x40 
    ret
    
.text
.globl ak_exec
ak_exec:
    mv x16, x15 # int flags
    mv x15, x14 # char** argv
    mv x14, x13 # int argc
    mv x13, x12 # const char* path
    li x12, 3
    syscall x0, 0x40 
    ret
    
.text
.globl ak_wait
ak_wait:
    mv x15, x14 # int options
    mv x14, x13 # int* status
    mv x13, x12 # int pid
    li x12, 4
    syscall x0, 0x40 
    ret
    
.text
.globl ak_ipc_init
ak_ipc_init:

    li x12, 9
    syscall x0, 0x40 
    ret
    
.text
.globl ak_dev_in
ak_dev_in:
    mv x14, x13 # u8 port
    mv x13, x12 # u32 devnum
    li x12, 12
    syscall x0, 0x40 
    ret
    
.text
.globl ak_dev_out
ak_dev_out:
    mv x15, x14 # u8 value
    mv x14, x13 # u8 port
    mv x13, x12 # u32 devnum
    li x12, 13
    syscall x0, 0x40 
    ret
    
.text
.globl ak_dev_ctl
ak_dev_ctl:
    mv x16, x15 # u32 len
    mv x15, x14 # void* buf
    mv x14, x13 # u32 cmd
    mv x13, x12 # u32 devnum
    li x12, 14
    syscall x0, 0x40 
    ret
    
.text
.globl ak_dev_claim
ak_dev_claim:
    mv x13, x12 # u32 devnum
    li x12, 15
    syscall x0, 0x40 
    ret
    
.text
.globl ak_open
ak_open:
    mv x14, x13 # int open_mode
    mv x13, x12 # const char* path
    li x12, 16
    syscall x0, 0x40 
    ret
    
.text
.globl ak_close
ak_close:
    mv x13, x12 # int fd
    li x12, 18
    syscall x0, 0x40 
    ret
    
.text
.globl ak_unlink
ak_unlink:
    mv x13, x12 # const char* path
    li x12, 19
    syscall x0, 0x40 
    ret
    
.text
.globl ak_rename
ak_rename:
    mv x14, x13 # const char* new_name
    mv x13, x12 # const char* old_name
    li x12, 20
    syscall x0, 0x40 
    ret
    
.text
.globl ak_mkdir
ak_mkdir:
    mv x13, x12 # const char* path
    li x12, 21
    syscall x0, 0x40 
    ret
    
.text
.globl ak_read
ak_read:
    mv x15, x14 # u32 count
    mv x14, x13 # void* buf
    mv x13, x12 # int fd
    li x12, 24
    syscall x0, 0x40 
    ret
    
.text
.globl ak_write
ak_write:
    mv x15, x14 # u32 count
    mv x14, x13 # void * buf
    mv x13, x12 # int fd
    li x12, 25
    syscall x0, 0x40 
    ret
    
.text
.globl ak_seek
ak_seek:
    mv x15, x14 # int whence
    mv x14, x13 # i32 offset
    mv x13, x12 # int fd
    li x12, 26
    syscall x0, 0x40 
    ret
    
.text
.globl ak_stat
ak_stat:
    mv x14, x13 # AkaiDirEntry* out_entry
    mv x13, x12 # const char* path
    li x12, 28
    syscall x0, 0x40 
    ret
    
.text
.globl ak_opendir
ak_opendir:
    mv x15, x14 # int flags
    mv x14, x13 # const char* path
    mv x13, x12 # AkaiDir* dir
    li x12, 30
    syscall x0, 0x40 
    ret
    
.text
.globl ak_closedir
ak_closedir:
    mv x14, x13 # int flags
    mv x13, x12 # AkaiDir* dir
    li x12, 31
    syscall x0, 0x40 
    ret
    
.text
.globl ak_readdir
ak_readdir:
    mv x15, x14 # int flags
    mv x14, x13 # AkaiDirEntry* out_entry
    mv x13, x12 # AkaiDir* dir
    li x12, 32
    syscall x0, 0x40 
    ret
    
.text
.globl ak_chdir
ak_chdir:
    mv x13, x12 # const char* path
    li x12, 33
    syscall x0, 0x40 
    ret
    
.text
.globl ak_getcwd
ak_getcwd:
    mv x14, x13 # u32 len
    mv x13, x12 # void* buf
    li x12, 34
    syscall x0, 0x40 
    ret
    
.text
.globl ak_sbrk
ak_sbrk:
    mv x13, x12 # u32 brk_increment
    li x12, 38
    syscall x0, 0x40 
    ret
    
.text
.globl ak_error
ak_error:
    mv x13, x12 # ProcessPID pid
    li x12, 41
    syscall x0, 0x40 
    ret
    
.text
.globl ak_getpid
ak_getpid:

    li x12, 42
    syscall x0, 0x40 
    ret
    
.text
.globl ak_abort
ak_abort:

    li x12, 43
    syscall x0, 0x40 
    ret
    
.text
.globl ak_time
ak_time:

    li x12, 44
    syscall x0, 0x40 
    ret
    
.text
.globl ak_clock
ak_clock:
    mv x15, x14 # ProcessPID pid
    mv x14, x13 # u32* system
    mv x13, x12 # u32* user
    li x12, 45
    syscall x0, 0x40 
    ret
    
.text
.globl ak_calendar
ak_calendar:
    mv x14, x13 # u32* calendar_2
    mv x13, x12 # u32* calendar_1
    li x12, 46
    syscall x0, 0x40 
    ret
    
.text
.globl ak_getppid
ak_getppid:

    li x12, 47
    syscall x0, 0x40 
    ret
    
.text
.globl ak_set_preempt
ak_set_preempt:
    mv x13, x12 # u32 mode
    li x12, 48
    syscall x0, 0x40 
    ret
    
.text
.globl ak_asm
ak_asm:
    mv x17, x16 # usize out_size
    mv x16, x15 # u8* out
    mv x15, x14 # usize curr_address
    mv x14, x13 # usize len
    mv x13, x12 # const char* line
    li x12, 49
    syscall x0, 0x40 
    ret
    .text
    .globl _fillw # extern void *_fillw(void *s, int c, size_t n);
_fillw:
    # fill buff, n, fill
    mv   x10, x12
    fillw x12, x14, x13 
    ret

    .globl _copybck # extern void *_copybck(void *src, void *dest, usize sz);
_copybck:
    mv x10, x13
    copybck x12, x13, x14
    ret
