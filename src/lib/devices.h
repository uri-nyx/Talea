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
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

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
 * @param fd a file descriptor to print the formatted byte
 */
void printer(byte_t* out_port, byte_t* options_port, byte_t* status_port, FILE* fd) {

}

/** 
    @brief loads a raw binary file into rom (0x0300 - 0x0bff)
 * 
 * @param fname the filename 
 */
void rom_loader(const char * fname){
    byte_t rom[0x8ff];
    FILE *fp = fopen(fname, "rb"); //opens the file to read binary
    fread(rom, 1, 0x8ff, fp);
    fclose(fp);

    for (size_t i = 0; i < 0x8ff; i++)
    {
        memory[i + 0x300] = rom[i];
    }
    
}
