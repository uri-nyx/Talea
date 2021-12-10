/**
 * @file devices.h
 * @author Uri Nyx (urinyxr@gmail.com)
 * @brief Dfines little utilities implemented as hardware for the usage of Talea.
 *  Namely, a simple Teletype, a printer(that can be joined or not to the latter), 
 *  a crystal cube cluster driver (mass storage) an audio chip, and a rom-loader. 
 * @version 0.1
 * @date 2021-11-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include "talea.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define MODE_d 0x01
#define MODE_c 0x02
#define MODE_r 0x03
/**
 * @brief Gets a keystroke and feeds it into in_port, and posts its status to status_port
 * 
 * @param in_port a pointer to an input port on Talea
 * @param status_port a pointer to an input port on Talea
 * @param path a path from which to read the keystroke
 */
void teletype(byte_t* in_port, byte_t* status_port, char * path){
    int fd;
    unsigned char in[2];

    fd = open(path, O_RDONLY | O_NONBLOCK);
    read(fd, &in, 2);
    if (in[1] == 0xff)
    {
        *in_port = in[0];
        *status_port = in[1];
    }
    
}

/**
 * @brief Gets the byte from out port, formats it based on the options, and posts its status
 * 
 * @param out_port a pointer to an output port of Talea
 * @param options_port a pointer to an output port of Talea
 * @param status_port a pointer to an input port of Talea
 * @param path a path to a file to print the formatted byte
 */
void printer(byte_t* out_port, byte_t* options_port, byte_t* status_port, char * path) {
    
    int fd;
    char formatted[5];
    byte_t to_print = *out_port;

    if (*options_port > 0)
    {
        *status_port = 0xfe; //printing
        switch (*options_port)
        {
        case MODE_d:
            sprintf(formatted, "%d", to_print);
            break;
        
        case MODE_c:
            sprintf(formatted, "%c", to_print);
            break;
        case MODE_r:
            formatted[0] = to_print;
            break;
        }
        FILE * fd = fopen(path, "a");
        fputs(formatted, fd);
        fclose(fd);
        *status_port = 0x00; //ready


    }
    


}

/** 
    @brief loads a raw binary file into rom (0x0300 - 0x0bff)
 * 
 * @param fname the filename 
 */
void rom_loader(const char * fname){
    byte_t rom[0xc00];
    FILE *fp = fopen(fname, "rb"); //opens the file to read binary
    fread(rom, 1, 0xc00, fp);
    fclose(fp);

    for (size_t i = 0; i < 0xc00; i++)
    {
        memory[i + 0x400] = rom[i];
    }
    
}

void tty_monitor(char* path_in, char* path_out, byte_t* status) {
    int from_monitor, to_monitor;
    FILE* reset_file;
    unsigned char in[3];
    byte_t order;
    byte_t a, b;

    enum orders {
        RESUME = 1,
        PAUSE,
        STEP,
        SETMEM,
        SETREG,
        CORE,
        STACK,
        CONTROL
    };

    from_monitor = open(path_in, O_RDONLY | O_NONBLOCK);
    to_monitor = open(path_out, O_WRONLY | O_NONBLOCK | O_CREAT, 0777);

    read(from_monitor, &in, 3);
    close(from_monitor);

    order = in[0];
    a = in[1];
    b = in[2];

    int array_size;
    array_size = b - a;
    byte_t coredump[array_size];
    byte_t control_panel[11] = {
        regs.pc, regs.status, st.pointer,
        regs.general[0], regs.general[1],
        regs.general[2], regs.general[3],
        regs.general[4], regs.general[5],
        regs.general[6], regs.general[7]
    };
    
    switch (order)
        {
        case RESUME:
            
            *status = 0;
            break;
        case PAUSE:
            
            *status = 1;
            break;
        case STEP:
            
            cycle();
            break;
        case SETMEM:
            
            memory[a] = b;
            break;
        case SETREG:
            
            switch (a)
            {
            case 8:
                regs.pc = b ;
                break;
            case 9:
                st.pointer = b;
                break;
            case 10:
                regs.status = b;
                break;
            default:
                regs.general[a] = b;
                break;
            }
            break;
        case CORE:
            

            for (size_t i = 0; i < array_size; i++)
            {
                coredump[i] = memory[i + a];
            }

            write(to_monitor, coredump, array_size);
            break;
        case STACK:
            
            write(to_monitor, &st.pointer, 1);
            write(to_monitor, st.stack, 255);
            break;
        case CONTROL:
            
            write(to_monitor, control_panel, 11);
            break;
        case 255:
            break;
        }

        close(to_monitor);
    
}