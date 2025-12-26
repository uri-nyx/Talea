#include "sas.h"
#include "a.out.h"

struct Token token_stream[SAS_MAX_TOKENS]; 

static char string_pool[SAS_STRING_POOL_SIZE];
static size_t current_string = 0;

#ifdef HAS_LIBC

void print_token(struct Token t) {
    switch (t.type)
    {
    case MNEMONIC: printf("%u\tT: %s\t(%x)\t[%s]\n", t.line, "MNEMONIC", t.value.opcode, t.lex); break;
    case IDENTIFIER: printf("%u\tT: %s\t(%s)\t[%s]\n", t.line, "IDENTIFIER", t.value.identifier, "="); break;
    case STRING: printf("%u\tT: %s\t(%s)\t[%s]\n", t.line, "STRING", t.value.string, "="); break;
    case DIRECTIVE  : printf("%u\tT: %s\t(%d)\t[%s]\n", t.line, "DIRECTIVE", t.value.directive, t.lex); break;
    case REGISTER : printf("%u\tT: %s\t(%d)\t[%s]\n", t.line, "REGISTER", t.value.reg, t.lex); break;
    case ADDRESS  : printf("%u\tT: %s\t(%x)\t[%s]\n", t.line, "ADDRESS", t.value.address, t.lex); break;
    case NUMBER  : printf("%u\tT: %s\t(%d)\t[%s]\n", t.line, "NUMBER", t.value.number, t.lex); break;
    case COMMA  : printf("%u\tT: %s\t(%s)\n", t.line, "COMMA", ","); break;
    case RPAREN  : printf("%u\tT: %s\t(%s)\n", t.line, "RPAREN", ")"); break;
    case LPAREN  : printf("%u\tT: %s\t(%s)\n", t.line, "LPAREN", "("); break;
    case COLON  : printf("%u\tT: %s\t(%s)\n", t.line, "COLON", ":"); break;
    case NEWLINE  : printf("%u\tT: %s\n", t.line, "NEWLINE"); break;
    case ENDOFILE: printf("%u\tT: %s\n", t.line, "EOF"); break;
    case TOKENERROR: printf("%u\tT: %s\t(%s)\t[%s]\n", t.line, "ERROR", t.value.string, t.lex); break;
    case UNEXPECTEDCHAR: printf("%u\tT: %s\t(%c)\n", t.line, "UNEXPECTED", t.value.unexpected); break;
    default: printf("UNREACHEABLE!!\n");
    }
}

#endif

/*TODO: use static allocation without libc*/
static char *copy_lexeme(struct Lexer *l) {
    char *s;
    size_t len;
    len = l->current - l->start;
    s = &string_pool[current_string];
    current_string += len + 1;
    if (current_string >= SAS_STRING_POOL_SIZE - 1) fatal("failed to copy lexeme: string pool out of memory");

    memcpy(s, &l->source[l->start], len);
    s[len] = 0;
    return s;
}

static int which_directive(struct Lexer *l, const char *s) {
    if (!strcmp(s, ".byte")) return BYTE;
    else if (!strcmp(s, ".half")) return HALF;
    else if (!strcmp(s, ".word")) return WORD;
    else if (!strcmp(s, ".text")) return TEXT;
    else if (!strcmp(s, ".data")) return DATA;
    else if (!strcmp(s, ".bss"))  return BSS;
    else if (!strcmp(s, ".align"))  return ALIGN;
    else if (!strcmp(s, ".format"))  return FORMAT;
    else if (!strcmp(s, ".org"))  return ORG;
    else if (!strcmp(s, ".const"))  return CONST;
    else return -1;
}

/* maybe an x-macro */

static int which_opcode(const char *s) {
    if (!strcmp(s, "add")) return OP_ADD ;
    else if (!strcmp(s, "sub")) return OP_SUB ;
    else if (!strcmp(s, "shll")) return OP_SLL ;
    else if (!strcmp(s, "shrl")) return OP_SRL ;
    else if (!strcmp(s, "shra")) return OP_SRA ;
    else if (!strcmp(s, "and")) return OP_AND ;
    else if (!strcmp(s, "or")) return OP_OR ;
    else if (!strcmp(s, "xor")) return OP_XOR ;
    else if (!strcmp(s, "slt")) return OP_SLT ;
    else if (!strcmp(s, "sltu")) return OP_SLTU ;
    else if (!strcmp(s, "not")) return OP_NOT ;
    else if (!strcmp(s, "ctz")) return OP_CTZ ;
    else if (!strcmp(s, "clz")) return OP_CLZ ;
    else if (!strcmp(s, "pcount")) return OP_PCNT ;
    else if (!strcmp(s, "ror")) return OP_ROR ;
    else if (!strcmp(s, "rol")) return OP_ROL ;
    else if (!strcmp(s, "beq")) return OP_BEQ ;
    else if (!strcmp(s, "bne")) return OP_BNE ;
    else if (!strcmp(s, "ble")) return OP_BLE ;
    else if (!strcmp(s, "bleu")) return OP_BLEU ;
    else if (!strcmp(s, "blt")) return OP_BLT ;
    else if (!strcmp(s, "bltu")) return OP_BLTU ;
    else if (!strcmp(s, "bge")) return OP_BGE ;
    else if (!strcmp(s, "bgeu")) return OP_BGEU ;
    else if (!strcmp(s, "bgt")) return OP_BGT ;
    else if (!strcmp(s, "bgtu")) return OP_BGTU ;
    else if (!strcmp(s, "jal")) return OP_JAL ;
    else if (!strcmp(s, "j")) return OP_J ;
    else if (!strcmp(s, "jalr")) return OP_JALR ;
    else if (!strcmp(s, "ret")) return OP_RET ;
    else if (!strcmp(s, "jr")) return OP_JR ;
    else if (!strcmp(s, "call")) return OP_CALL ;
    else if (!strcmp(s, "addi")) return OP_ADDI ;
    else if (!strcmp(s, "mv")) return OP_MV ;
    else if (!strcmp(s, "andi")) return OP_ANDI ;
    else if (!strcmp(s, "ori")) return OP_ORI ;
    else if (!strcmp(s, "xori")) return OP_XORI ;
    else if (!strcmp(s, "slti")) return OP_SLTI ;
    else if (!strcmp(s, "sltiu")) return OP_SLTIU ;
    else if (!strcmp(s, "shlli")) return OP_SLLI ;
    else if (!strcmp(s, "shrli")) return OP_SRLI ;
    else if (!strcmp(s, "shrai")) return OP_SRAI ;
    else if (!strcmp(s, "imul")) return OP_IMUL ;
    else if (!strcmp(s, "umul")) return OP_MUL ;
    else if (!strcmp(s, "mulih")) return OP_MULIH ;
    else if (!strcmp(s, "muli")) return OP_MULI ;
    else if (!strcmp(s, "idiv")) return OP_DIV ;
    else if (!strcmp(s, "udiv")) return OP_UDIV ;
    else if (!strcmp(s, "idivi")) return OP_DIVI ;
    else if (!strcmp(s, "lb")) return OP_LB ;
    else if (!strcmp(s, "lbd")) return OP_LBD ;
    else if (!strcmp(s, "lh")) return OP_LH ;
    else if (!strcmp(s, "lhd")) return OP_LHD ;
    else if (!strcmp(s, "lw")) return OP_LW ;
    else if (!strcmp(s, "lwd")) return OP_LWD ;
    else if (!strcmp(s, "lbu")) return OP_LBU ;
    else if (!strcmp(s, "lbud")) return OP_LBUD ;
    else if (!strcmp(s, "lhu")) return OP_LHU ;
    else if (!strcmp(s, "lhud")) return OP_LHUD ;
    else if (!strcmp(s, "la")) return OP_LA ;
    else if (!strcmp(s, "li")) return OP_LI ;
    else if (!strcmp(s, "lui")) return OP_LUI ;
    else if (!strcmp(s, "auipc")) return OP_AUIPC ;
    else if (!strcmp(s, "sb")) return OP_SB ;
    else if (!strcmp(s, "sh")) return OP_SH ;
    else if (!strcmp(s, "sw")) return OP_SW ;
    else if (!strcmp(s, "sbd")) return OP_SBD ;
    else if (!strcmp(s, "shd")) return OP_SHD ;
    else if (!strcmp(s, "swd")) return OP_SWD ;
    else if (!strcmp(s, "copy")) return OP_COPY ;
    else if (!strcmp(s, "copymd")) return OP_COPYMD ;
    else if (!strcmp(s, "copydm")) return OP_COPYDM ;
    else if (!strcmp(s, "copydd")) return OP_COPYDD ;
    else if (!strcmp(s, "swap")) return OP_SWAP ;
    else if (!strcmp(s, "swapmd")) return OP_SWAPMD ;
    else if (!strcmp(s, "swapdm")) return OP_SWAPDM ;
    else if (!strcmp(s, "swapdd")) return OP_SWAPDD ;
    else if (!strcmp(s, "fill")) return OP_FILL ;
    else if (!strcmp(s, "fillh")) return OP_FILLH ;
    else if (!strcmp(s, "fillsq")) return OP_FILLSQ ;
    else if (!strcmp(s, "fillw")) return OP_FILLW ;
    else if (!strcmp(s, "thro")) return OP_THRO ;
    else if (!strcmp(s, "from")) return OP_FROM ;
    else if (!strcmp(s, "popb")) return OP_POPB ;
    else if (!strcmp(s, "poph")) return OP_POPH ;
    else if (!strcmp(s, "pop")) return OP_POP ;
    else if (!strcmp(s, "pushb")) return OP_PUSHB ;
    else if (!strcmp(s, "pushh")) return OP_PUSHH ;
    else if (!strcmp(s, "push")) return OP_PUSH ;
    else if (!strcmp(s, "save")) return OP_SAVE ;
    else if (!strcmp(s, "restore")) return OP_RESTORE ;
    else if (!strcmp(s, "exch")) return OP_EXCH ;
    else if (!strcmp(s, "syscall")) return OP_CLL ;
    else if (!strcmp(s, "sysret")) return OP_MRET ;
    else if (!strcmp(s, "gsreg")) return OP_GSR ;
    else if (!strcmp(s, "ssreg")) return OP_SSR ;
    else if (!strcmp(s, "trace")) return OP_TRACE ;
    else if (!strcmp(s, "cli")) return OP_CLI ;
    else if (!strcmp(s, "sti")) return OP_STI ;
    else if (!strcmp(s, "mmu_toggle")) return OP_MMUTOG ;
    else if (!strcmp(s, "mmu_map")) return OP_MMUMAP ;
    else if (!strcmp(s, "mmu_unmap")) return OP_MMUUNMAP ;
    else if (!strcmp(s, "mmu_stat")) return OP_MMUSTAT ;
    else if (!strcmp(s, "mmu_getframe")) return OP_MMUGETFRAME ;
    else if (!strcmp(s, "mmu_setpt")) return OP_MMUSPT ;
    else if (!strcmp(s, "mmu_update")) return OP_MMUUPDATE ;
    else if (!strcmp(s, "mmu_switch")) return OP_MMUSW ;
    else if (!strcmp(s, "mmu_getpt")) return OP_MMUGPT ;
    else if (!strcmp(s, "umode_toggle")) return OP_UMODETOG ;
    else return -1;
}

static int end_of_source(struct Lexer *l) {
    return l->current >= l->source_len;
}

static void add_token(struct Lexer *l, struct Token *t) {
    if (l->current_token >= SAS_MAX_TOKENS) fatal("token buffer exhausted");

    l->token_stream[l->current_token].lex = t->lex;
    l->token_stream[l->current_token].type = t->type;
    l->token_stream[l->current_token].value = t->value;
    l->token_stream[l->current_token].line = t->line;

    l->current_token++;
}

static void add_error_token(struct Lexer *l, char * lex, char *repr) {
    if (l->current_token >= SAS_MAX_TOKENS) fatal("token buffer exhausted");

    l->token_stream[l->current_token].lex = lex;
    l->token_stream[l->current_token].type = TOKENERROR;
    l->token_stream[l->current_token].value.string = repr;
    l->token_stream[l->current_token].line = l->lineno;

    l->current_token++;
}

static void add_unexpected_char(struct Lexer *l, char lex, const char *repr) {
    if (l->current_token >= SAS_MAX_TOKENS) fatal("token buffer exhausted");

    l->token_stream[l->current_token].lex = "";
    l->token_stream[l->current_token].type = UNEXPECTEDCHAR;
    l->token_stream[l->current_token].value.unexpected = lex;
    l->token_stream[l->current_token].line = l->lineno;

    l->current_token++;
}

static char peek(struct Lexer *l) {
    if (end_of_source(l)) return '\0';
    else return l->source[l->current];
}

static long getnumber(struct Lexer *l, int *err) {
    long n = 0, positions = 0;
    char c;

    c = l->source[l->current++];
    
    if (c == '0' && peek(l) == 'x') {
        /* number is in hex */
        l->current++; /* consume 'x' */

        while (isxdigit(peek(l))) {
            n *= 16;
            c = l->source[l->current++];
            n += char_to_int[c];
            positions++;
        }

        if (positions > 10) warning(l->lineno, "Only up to 32 bit integers supported", NULL);
    } else if (isdigit(c)) {
        /* number is in decimal */
        positions = 1;
        n += char_to_int[c];
        while(isdigit(peek(l))) {
            n *= 10;
            c = l->source[l->current++];
            n += char_to_int[c];
            positions++;
        }
        if (positions > 8) warning(l->lineno, "Only up to 32 bit integers supported", NULL);
    } else {
        /* malformed number */
        l->current--; /* maybe edgecases */
        *err = 1;
        return n;

    } 

    if (positions <= 0) {
        *err = 1;
        return n;
    }

    *err = 0;
    return n;
}

static void string(struct Lexer *l) {
    /* do we need to allocate memory? */
    struct Token t;
    char *s;
    size_t i, len;

    while (peek(l) != '"' && !end_of_source(l)) {
      if (peek(l) == '\n') {
        add_error_token(l, "", "Multiline strings are not supported");
        l->lineno++;
        return;
      }

      l->current++;
    }

    if (end_of_source(l)) {
        add_error_token(l, "", "Unterminated string at EOF");

        return;
    }

    /* The closing " */
    l->current++;

    /* Trim the surrounding quotes */
    s = &string_pool[current_string];
    len = (l->current - 1) - (l->start + 1);
    current_string += len +1;
    if (current_string >= SAS_STRING_POOL_SIZE + 1) fatal("failed to allocate memory for string: string pool exhausted.");

    for (i = 0; i < (l->current - 1) - (l->start + 1); i++) s[i] = l->source[i + l->start + 1];
    s[((l->current - 1) - (l->start + 1))] = 0;


    t.lex = s;
    t.type = STRING;
    t.value.string = s;
    t.line = l->lineno;

    add_token(l, &t);
}

static void number(struct Lexer *l) {
    /*FIXME: THIS CAN RETURN AN INVALID NUMBER */
    struct Token t;
    char c;
    long n = 0;
    int negative = 0, err;

  
    if (peek(l) == '-') {
        negative = 1; /* negative numbers */
        c = l->source[l->current++]; /* consume '-' */
    }

    n = getnumber(l, &err);

    #ifdef COPY_LEXEMES
        t.lex = copy_lexeme(l);
        if (err) {
            add_error_token(l, t.lex, "Expected a number");
            return;
        }
    #endif
    #ifndef COPY_LEXEMES
        t.lex = NULL;
        if (err) {
            add_error_token(l, "", "Expected a number");
            return;
        }
    #endif
    
    t.type = NUMBER;
    t.value.number = negative? -n : n;
    t.line = l->lineno;

    add_token(l, &t);
}

static void address(struct Lexer *l) {
    /*FIXME: THIS CAN RETURN AN INVALID NUMBER */
    struct Token t;
    unsigned long a = 0;
    int err;

    a = getnumber(l, &err);

    #ifdef COPY_LEXEMES
        t.lex = copy_lexeme(l);
        if (err) {
            add_error_token(l, t.lex, "Expected an address (unsigned number)");
            return;
        }
    #endif
    #ifndef COPY_LEXEMES
        t.lex = NULL;
        if (err) {
            add_error_token(l, "", "Expected an address (unsigned number)");
            return;
        }
    #endif
    
    t.type = ADDRESS;
    t.value.address = a;
    t.line = l->lineno;

    add_token(l, &t);
}

static void reg(struct Lexer *l) {
    struct Token t;
    enum Register r;
    int err, err2 = 0;

    if (peek(l) != 'x') err2 = 1;
    else l->current++;

    r = getnumber(l, &err);
    
    #ifdef COPY_LEXEMES
        t.lex = copy_lexeme(l);
        if (err || err2) {
            add_error_token(l, t.lex, "Register syntax is %x(0-31) got");
            return;
        }
    #endif
    #ifndef COPY_LEXEMES
        t.lex = NULL;
        if (err || err2) {
            add_error_token(l, t.lex, "Register syntax is %x(0-31) got");
            return;
        }
    #endif
    
    if (r < 0 || r > 31) {
        add_error_token(l, t.lex, "Register must be between x0 and x31 got");
        return;
    }

    t.type = REGISTER;
    t.value.reg = r;
    t.line = l->lineno;

    add_token(l, &t);
}

static void identifier(struct Lexer *l) {
    struct Token t;
    int op;

    while (isalnum(peek(l)) || peek(l) == '_') l->current++;

    t.lex = copy_lexeme(l);

    op = which_opcode(t.lex);
    t.type = (op == -1) ? IDENTIFIER : MNEMONIC;
    if (op == -1) t.value.identifier = t.lex;
    else t.value.opcode = op;

    t.line = l->lineno;

    add_token(l, &t);
}

static void directive(struct Lexer *l) {
    struct Token t;

    while (isalnum(peek(l)) || peek(l) == '_') l->current++;

    t.lex = copy_lexeme(l);
    t.type = DIRECTIVE;
    t.value.directive = which_directive(l, t.lex);

    if (t.value.directive == -1) {
        add_error_token(l, t.lex, "Unknown directive");
        return;
    }

    t.line = l->lineno;

    add_token(l, &t);
}

static void scan_token(struct Lexer *l) {
    static int unexpected = 0;
    struct Token t;
    char c = l->source[l->current++];

    switch (c)
    {
    case SEMICOLON:
        while (peek(l) != NEWLINE && !end_of_source(l)) l->current++;
        break;
    case NEWLINE: /* increment line number when assigning */
    case COMMA:
    case COLON:
    case LPAREN:
    case RPAREN:
        /* These tokens are only one character, so c == type. 
        value is therefore null */
        t.lex = NULL;
        t.type = c;
        t.line = (c == NEWLINE) ? l->lineno++ : l->lineno;

        add_token(l, &t);
        break;

    case QUOTE: string(l); break;
    case NUMBER: number(l); break;
    case ADDRESS: address(l); break;
    case REGISTER: reg(l); break;
    case DIRECTIVE: directive(l); break;
    

    case ' ':
    case '\t':
    case '\r':
        /* Ignore whitespace */
        break;
    default:
        if (isalpha(c) || c == '_') {
            identifier(l);
            break;
        } else {
            add_unexpected_char(l, c, "Unexpected");
            return;
        }
    }

    return;
}

/* should return errors in some way*/
void lex(struct Lexer *l) {
    struct Token t;

    while (!end_of_source(l)) {
        /* We are at the beginning of the next lexeme. */
        l->start = l->current;
        scan_token(l);
    }

    t.lex = NULL;
    t.type = ENDOFILE;
    t.value.string = NULL;
    t.line = l->lineno;

    add_token(l, &t);
}

