.globl main
.globl _exit
.globl _syscall
.globl _trace
.text
_start:
    call main
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
    mv x14, x13 # u8 value
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
    