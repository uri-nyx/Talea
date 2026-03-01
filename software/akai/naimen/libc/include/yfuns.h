#ifndef _YFUNS
#define _YFUNS
/* process control functions */
#define _Envp         // TODO
#define _Exit(status) // TODO

/* stdio functions */
#define _Fclose(str) ak_close((str)->_Handle)
#define _Fread(str, buf, cnt)                                             \
    ((str)->_Handle >= 255 ? ak_dev_ctl((str)->_Handle, 1001, buf, cnt) : \
                             ak_read((str)->_Handle, buf, cnt))
#define _Fwrite(str, buf, cnt)                                            \
    ((str)->_Handle >= 255 ? ak_dev_ctl((str)->_Handle, 1002, buf, cnt) : \
                             ak_write((str)->_Handle, buf, cnt))

/* interface declarations */
extern const char C0environ; // TODO

int  ak_close(int fd);
void ak_exit(int);
int  ak_read(int, unsigned char *, int);
int  ak_write(int, const unsigned char *, int);
int  ak_dev_ctl(unsigned int devnum, unsigned int cmd, void *buf, unsigned int len);

#endif
