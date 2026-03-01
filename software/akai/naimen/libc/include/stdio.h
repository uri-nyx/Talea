#ifndef _STDIO_H
#define _STDIO_H

#ifndef _YVALS
#include "yvals.h"
#endif

/* Macros */

#ifndef NULL
#define NULL _NULL
#endif

#define _IOFBF       0
#define _IOLBF       1
#define _IONBF       2
#define BUFSIZ       512
#define EOF          -1
#define FILENAME_MAX _FNAMAX
#define FOPEN_MAX    _FOPMAX
#define L_tmpnam     _TNAMAX
#define TMP_MAX      6
#define SEEK_SET     0
#define SEEK_CUR    1
#define SEEK_END     2
#define stdin        _Files[0]
#define stdout       _Files[1]
#define stderr       _Files[2]

/* types */

#ifndef _SIZET
#define _SIZET
typedef _Sizet size_t;
#endif

typedef struct {
    unsigned long _Off;
} fpos_t;

typedef struct {
    unsigned short _Mode;
    short          _Handle;
    unsigned char *_Buf, *_Bend, *_Next;
    unsigned char *_Rend, *_Rsave, *_Wend;
    unsigned char  _Back[2], _Cbuf, _Nback;
    char          *_Tmpnam;
} FILE;

void   clearerr(FILE *stream);
int    fclose(FILE *stream);
int    feof(FILE *stream);
int    ferror(FILE *stream);
int    fflush(FILE *stream);
int    fgetc(FILE *stream);
int    fgetpos(FILE *stream, fpos_t *pos);
char  *fgets(char *s, int n, FILE *stream);
FILE  *fopen(const char *filename, const char *mode);
int    fprintf(FILE *stream, const char *format, ...);
int    fputc(int c, FILE *stream);
int    fputs(const char *s, FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
FILE  *freopen(const char *filename, const char *mode, FILE *stream);
int    fscanf(FILE *stream, const char *format, ...);
int    fseek(FILE *stream, long int offset, int whence);
int    fsetpos(FILE *stream, const fpos_t *pos);
long   ftell(FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int    getc(FILE *stream);
int    getchar(void);
char  *gets(char *s);
void   perror(const char *s);
int    printf(const char *format, ...);
int    putc(int c, FILE *stream);
int    putchar(int c);
int    puts(const char *s);
int    remove(const char *filename);
int    rename(const char *old, const char *newfile);
void   rewind(FILE *stream);
int    scanf(const char *format, ...);
void   setbuf(FILE *stream, char *buf);
int    setvbuf(FILE *stream, char *buf, int mode, size_t size);
int    sprintf(char *s, const char *format, ...);
int    sscanf(const char *s, const char *format, ...);
FILE  *tmpfile(void);
char  *tmpnam(char *s);
int    ungetc(int c, FILE *stream);
int    vfprintf(FILE *stream, const char *format, char *arg);
int    vprintf(const char *format, char *arg);
int    vsprintf(char *s, const char *format, char *arg);

long _Fgpos(FILE *stream, fpos_t *pos);
long _Fspos(FILE *stream, const fpos_t *pos, long offset, int whence);

extern FILE *_Files[FOPEN_MAX];

/* Macro overrides */
#define fgetpos(str, ptr)    (int)_Fgpos(str, ptr)
#define fseek(str, off, way) _Fspos(str, _NULL, off, way)
#define fsetpos(str, ptr)    _Fspos(str, ptr, 0L, 0)
#define ftell(str)           _Fgpos(str, _NULL)
#define getc(str)            ((str)->_Next < (str)->_Rend ? *(str)->_Next++ : (getc)(str))
#define getchar()            (_Files[0]->_Next < _Files[0]->_Rend ? *_Files[0]->_Next++ : (getchar)())
#define putc(c, str)         ((str)->_Next < (str)->_Wend ? (*(str)->_Next++ = c) : (putc)(c, str))
#define putchar(c)           (_Files[1]->_Next < _Files[1]->_Wend ? (*_Files[1]->_Next++ = c) : (putchar)(c))

#endif /* _STDIO_H */
