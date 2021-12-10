#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>

/* reads from keypress, doesn't echo */
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

int main(){
    int fd;
    char in;
    char status;
    char * fifo = "/tmp/fifo";
    mkfifo(fifo, 0666);

    do
    {
        
        in = getch();
        putchar(in);
        printf("in: %d", in);
        status = 0xff;
        unsigned char sent[2] = {in, status};
        fd = open(fifo, O_WRONLY);
        write(fd, &sent, 2);
        
        
    } while (in != '-');

    close(fd);
    unlink(fifo);
    return 0;
}