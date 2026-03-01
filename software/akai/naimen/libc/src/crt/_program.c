extern int main(int argc, char *argv[]);
extern int _Initio(void);
extern void exit(int);

int _program(int argc, char *argv[]) {
    int ret = 0;
    _trace(0x8da8da, 0x100120);
    _Initio();
    _trace(0x8da8da, 0x100123);
    ret = main(argc, argv);
    exit(ret);
    return ret;
}
