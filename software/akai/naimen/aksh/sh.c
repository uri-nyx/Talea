#include <akai.h>

int tokenize(char *line, usize sz, char **out_argv)
{
    return 0;
}

void main()
{
    char line[256];
    u8   imode[1] = { IN_CANON | IN_ECHO | IN_CRNL };
    u8   omode[1] = { OUT_NLCR };
    char cls[]    = "\x1b[2J";
    int  res;

    res = ak_dev_ctl(PDEV_STDIN, PX_SETCANON, imode, 1);
    if (res) ak_exit(1);

    res = ak_dev_ctl(PDEV_STDOUT, PX_SETCANON, omode, 1);
    if (res) ak_exit(2);

    ak_dev_ctl(PDEV_STDOUT, PX_WRITE, cls, sizeof(cls) - 1);

    while (1) {
        ak_dev_out(PDEV_STDOUT, 0, '>');
        res = ak_dev_ctl(PDEV_STDIN, PX_READ, line, 256);
        if (res != 256) {
            _trace(0xc00da, res);
            ak_dev_ctl(PDEV_STDOUT, PX_WRITE, "Error reading line!\n", 20);
        } else {
            char **argv;
            usize  i;

            res = tokenize(line, res, argv); // tokenize uses the same line buffer

            for (i = 0; i < res; i++) {
                ak_dev_ctl(PDEV_STDOUT, PX_WRITE, argv[i], strlen(argv[i]));
                ak_dev_out(PDEV_STDOUT, 0, '\n');
            }
        }
    }
}
