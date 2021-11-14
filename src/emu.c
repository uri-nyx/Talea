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


#define HALT 0xffff

struct termios orig_termios;

void die(const char *s) {
  perror(s);
  exit(1);
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

void tty_printer(byte_t* out_port, byte_t* options_port, byte_t* status_port, char * path) {
    
    int fd;
    char formatted[5];
    byte_t to_print = *out_port;

    if (*options_port > 0)
    {
        *status_port = 0xfe; //printing
        FILE * fd = fopen(, "a"); //raw
        fputs(to_print, fd);
        fclose(fd);
        *status_port = 0xff; //ready
    }

void tty_keyboard(){
    
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') memory[HALT] = 0xff;
}


int main(int argc, char *argv){
    
    if (argc > 3)
    {
        printf("Usage: %s <rom-image> <clock-speed-hz>", argv[0]);
        return 1;
    }
    
    int hz = atoi(argv[2])
    st.pointer = 0;
    regs.status = 0x2;
    regs.pc = 0x300;

    rom_loader(argv[1]);

    enableRawMode();

    while (!memory[HALT])
    {
        //tty_printer()
        tty_keyboard()
        
        sleep(hz / 0.001)
    }
    
    disableRawMode();
}

