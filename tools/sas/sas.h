#ifndef SAS_H
#define SAS_H

#define SAS_DEBUG
#define HAS_LIBC /* we have functioning libc */
#define COPY_LEXEMES /* we copy the lexemes to the tokens for better error messages*/


#ifdef HAS_LIBC
    #include <stdio.h> 
    /* These are freestanding headers, you can get them almost everywhere */
    #include <string.h>
    #include <ctype.h>
#endif

#ifdef SAS_AKAI
    #include <libc.h>
    #include <libc/ctype.h>
    #include <libc/stdlib.h>
    #include <libc/string.h>
    #include <libc/term.h>

    typedef usize size_t;
#endif

/* These is provided */
#include "a.out.h"

static const char char_to_int[128] = {
    /* 0-9 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0-15*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 16-31*/
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 32-47*/
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, /* 48-63 ('0' through '9')*/
    -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, /* 64-79 ('A' through 'O')*/
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1, /* 80-95 ('P' through 'Z')*/
    -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, /* 96-111 ('a' through 'o')*/
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1  /* 112-127 ('p' through 'z')*/
};

/* Special characters that are not tokens but lexically meaningful*/
#define SEMICOLON ';' /* Comment character */
#define QUOTE '"' /*String delimiter character*/

enum TokenType {
    MNEMONIC,
    IDENTIFIER,
    STRING,
    DIRECTIVE = '.',
    REGISTER ='%',
    ADDRESS = '$',
    NUMBER = '#',

    COMMA = ',',
    LPAREN = '(',
    RPAREN = ')',
    COLON = ':',


    NEWLINE = '\n',
    ENDOFILE,

    TOKENERROR,
    UNEXPECTEDCHAR
};


enum Directive {
    BYTE,
    HALF,
    WORD,
    TEXT,
    DATA,
    BSS,
    ALIGN,
    FORMAT,
    ORG,
    CONST,
};

enum FORMATS {AOUT, RAW, FORMATS_N};
static char *formats[FORMATS_N] = {
    "a.out", "raw"
};



enum Register {
    X0, X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16,
    X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31
};


enum Opcode {

/* Arithmetic ops */
    OP_ADD =   0x60,
    OP_SUB =   0x61,
    OP_SLL =   0x6D,
    OP_SRL =   0x6C,
    OP_SRA =   0x6B,
    OP_AND =   0x65,
    OP_OR =    0x64,
    OP_XOR =   0x66,
    OP_MUL =   0x63,
    OP_IMUL =  0x163,
    OP_MULIH = 0x51,
    OP_MULI =  0x50,
/*  OP_MULHU =        0x*/
    OP_DIV =  0x62,
    OP_UDIV = 0x162,
    OP_DIVI = 0x52,
/*
    OP_DIVU =         0x,
    OP_REM =          0x,
    OP_REMU =         0x,
*/

    OP_ADDI =  0x53,
    OP_MV =   0x153,
    OP_SLLI =  0x5a,
    OP_NOP =   0x60,
    OP_ANDI =  0x56,
    OP_ORI =   0x55,
    OP_XORI =  0x57,
    OP_SLTI =  0x5b,
    OP_SLTIU = 0x5c,
    OP_SRLI =  0x59,
    OP_SRAI =  0x58,

    OP_NOT =  0x67,
    OP_CTZ =  0x68,
    OP_CLZ =  0x69,
    OP_PCNT = 0x6a,
    OP_ROR =  0x6e,
    OP_ROL =  0x6f,

    /* Block ops */
    OP_COPY =   0x10,
    OP_COPYMD = 0x110,
    OP_COPYDM = 0x210,
    OP_COPYDD = 0x310,
    OP_SWAP =   0x11,
    OP_SWAPMD = 0x111,
    OP_SWAPDM = 0x211,
    OP_SWAPDD = 0x311,
    OP_FILL =   0x12,
    OP_FILLH =  0x112,
    OP_FILLSQ = 0x212,
    OP_FILLW =  0x312,
    OP_THRO =   0x13,
    OP_FROM =   0x14,

    /* Stack ops */
    OP_POPB =  0x15,
    OP_POPH =  0x16,
    OP_POP =   0x17,
    OP_PUSHB = 0x18,
    OP_PUSHH = 0x19,
    OP_PUSH =  0x1a,

    OP_SAVE =    0x1b,
    OP_RESTORE = 0x1c,
    OP_EXCH =    0x1d,

    /* Branches */
    OP_BEQ =  0x30,
    OP_BNE =  0x31,
    OP_SLT =  0x1e,
    OP_SLTU = 0x1f,
    OP_BLT =  0x32,
    OP_BGT =  0x132,
    OP_BGE =  0x33,
    OP_BLE =  0x133,
    OP_BLTU = 0x34,
    OP_BGTU = 0x134,
    OP_BGEU = 0x35,
    OP_BLEU = 0x135,

    /* Loads */
    OP_LB =   0x42,
    OP_LH =   0x46,
    OP_LW =   0x4A /* The docs are wrong */,
    OP_LBU =  0x43,
    OP_LHU =  0x47,
    OP_LBD =  0x44,
    OP_LBUD = 0x45,
    OP_LHD =  0x48,
    OP_LHUD = 0x49,
    OP_LWD =  0x4B /* The docs are wrong */,

    OP_LA =    0x122,
    OP_LI =    0x121,
    OP_LUI =   0x21,
    OP_AUIPC = 0x22,

    /* Stores */
    OP_SB =  0x70,
    OP_SH =  0x72,
    OP_SW =  0x74,
    OP_SBD = 0x71,
    OP_SHD = 0x73,
    OP_SWD = 0x75,

    /* Jumps */
    OP_J =  0x120,
    OP_JAL =  0x20,
    OP_RET = 0x141,
    OP_JR  = 0x241,
    OP_JALR = 0x41,
    OP_CALL = 0x00,

    /* System */
    OP_CLL =   0x02,
    OP_MRET =  0x06,
    OP_GSR =   0x03,
    OP_SSR =   0x04,
    OP_TRACE = 0x05,
    OP_CLI =   0x106,
    OP_STI =   0x206,

    /* MMU */
    OP_MMUTOG =      0x07,
    OP_MMUMAP =      0x08,
    OP_MMUUNMAP =    0x09,
    OP_MMUSTAT =     0x0a,
    OP_MMUGETFRAME = 0x10a,
    OP_MMUSPT =      0x0b,
    OP_MMUUPDATE =   0x0c,
    OP_MMUSW =       0x0e,
    OP_MMUGPT =      0x0f,
    OP_UMODETOG =    0x0d
};


struct Token
{
    char *lex;
    enum TokenType type;

    union
    {
        long number;
        unsigned long address;
        char *string;
        char *identifier;
        enum Directive directive;
        enum Register reg;
        enum Opcode opcode;
        char unexpected;
    } value;

    unsigned int line;
};


struct Lexer {
    const char *source;
    size_t source_len;
    
    size_t start, current, lineno;

    struct Token *token_stream;
    size_t current_token;
};

enum Section {
    SECTION_TEXT,
    SECTION_DATA,
    SECTION_BSS
};

struct Symbol {
    char *name;
    int defined;
    enum {SYM_LABEL, SYM_CONST} type;
    enum Section section;
    unsigned long value;
};

struct Patch {
    char *name;
    size_t lineno;
    size_t section_start;
    RelocRecord reloc;
};

/* MEMORY */
#ifndef SAS_MAX_TOKENS
    #define SAS_MAX_TOKENS 10000
#endif

#ifndef SAS_MAX_SYMS
    #define SAS_MAX_SYMS 10000
#endif

#ifndef SAS_MAX_PATCHES
    #define SAS_MAX_PATCHES 10000
#endif

#ifndef SAS_STRING_POOL_SIZE
    #define SAS_STRING_POOL_SIZE 10000
#endif

#ifndef SAS_MAX_TEXT_SIZE
    #define SAS_MAX_TEXT_SIZE (256 * 1024)
#endif

#ifndef SAS_MAX_DATA_SIZE
    #define SAS_MAX_DATA_SIZE (256 * 1024)
#endif

#ifndef SAS_MAX_OBJ_SIZE
    #define SAS_MAX_OBJ_SIZE (752 * 1024)
#endif

#ifndef SAS_MAX_SOURCE_SIZE
    #define SAS_MAX_SOURCE_SIZE (1024 * 1024)
#endif



extern struct Token token_stream[SAS_MAX_TOKENS];

void warning (unsigned int line, const char *msg, const char *offending);
void error (unsigned int line, const char *msg, const char *offending);
void fatal(const char *msg);

void lex(struct Lexer *l);
unsigned char *assemble(const char *asm_source, size_t source_len, size_t *object_size);


  
#endif /* SAS_H */
