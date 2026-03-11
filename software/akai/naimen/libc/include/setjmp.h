#ifndef _SETJMP_H
#define _SETJMP_H

#define setjmp(env) __setjmp(env)

typedef struct {
    unsigned int ra; /* x1 */
    unsigned int sp; /* x2 */
    unsigned int fp; /* x8 */
    unsigned int x9;
    unsigned int x18;
    unsigned int x19;
    unsigned int x20;
    unsigned int x21;
    unsigned int x22;
    unsigned int x23;
    unsigned int x24;
    unsigned int x25;
    unsigned int x26;
    unsigned int x27;
} jmp_buf[1];

extern void longjmp(jmp_buf, int);

#endif /* _SETJMP_H */
