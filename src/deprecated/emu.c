/**
 * @file emu.c
 * @author Uri Nyx (urinyxr@dgmail.com)
 * @brief A simple emulator fo a fum terminal or tty attached to a Talea machine, without system monitor
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <termios.h>
#include "lib/devices.h"


#define HALT 0xfffe

struct termios orig_termios;

void die(const char *s) {
  perror(s);
  exit(1);
}

int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

void tty_printer(byte_t* out_port, byte_t* options_port, byte_t* status_port) {
    
    char to_print = (char)*out_port;

    if (*options_port > 0)
    {
        *status_port = 0xfe; //printing
        if (write(STDOUT_FILENO, &to_print, 1) == -1) die("write");
        *status_port = 0xff; //ready
    }
}

void tty_keyboard(byte_t* in_port, byte_t* status_port){

    
    if (kbhit()) {
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    *in_port = (unsigned)c;
    if(c) *status_port = 0xff;
    if (c == 'q') memory[HALT] = 0xff;
    }
}


int main(int argc, char **argv){
    
    if (argc > 3)
    {
        printf("Usage: %s <rom-image> <clock-speed-hz>", argv[0]);
        return 1;
    }
    
    int hz = atoi(argv[2]);
    const char* rom_file = argv[1];
    int cycles;
    st.pointer = 0;
    regs.status = 0x2;
    regs.pc = 0x000;

    rom_loader(rom_file);

    enableRawMode();

    while (!memory[HALT])
    {
        tty_keyboard(&memory[0x100], &memory[0x101]);
        tty_printer(&memory[0x200], &memory[0x201], &memory[0x10a]);

        cycle();
        cycles++;       
        //usleep(1000000 / (hz*1000000));
    }
    
    disableRawMode();
    printf("\n\r[Computation performed in %d cycles]", cycles);
}

