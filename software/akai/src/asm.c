#include "asm.h"
#include "kstring.h"

struct Token token_stream[SAS_MAX_TOKENS];
static u8   *output;
static usize outptr;
static usize outputsz;
static i32   asm_error = 0;

void error(int code)
{
    asm_error = code;
}

static usize emit_be32(unsigned long w)
{
    //miniprint("Emitting %x\n", w);
    if (outptr >= outputsz - 3) {
        //miniprint("Error!");
        error(ASM_ERROR_BUFFER);
        return 0;
    }

    output[outptr++] = (w >> 24) & 0xff;
    output[outptr++] = (w >> 16) & 0xff;
    output[outptr++] = (w >> 8) & 0xff;
    output[outptr++] = w & 0xff;

    return 4;
}

static int which_opcode(const char *s)
{
    if (!strcmp(s, "add"))
        return OP_ADD;
    else if (!strcmp(s, "sub"))
        return OP_SUB;
    else if (!strcmp(s, "shll"))
        return OP_SLL;
    else if (!strcmp(s, "shrl"))
        return OP_SRL;
    else if (!strcmp(s, "shra"))
        return OP_SRA;
    else if (!strcmp(s, "and"))
        return OP_AND;
    else if (!strcmp(s, "or"))
        return OP_OR;
    else if (!strcmp(s, "xor"))
        return OP_XOR;
    else if (!strcmp(s, "slt"))
        return OP_SLT;
    else if (!strcmp(s, "sltu"))
        return OP_SLTU;
    else if (!strcmp(s, "not"))
        return OP_NOT;
    else if (!strcmp(s, "ctz"))
        return OP_CTZ;
    else if (!strcmp(s, "clz"))
        return OP_CLZ;
    else if (!strcmp(s, "pcount"))
        return OP_PCNT;
    else if (!strcmp(s, "ror"))
        return OP_ROR;
    else if (!strcmp(s, "rol"))
        return OP_ROL;
    else if (!strcmp(s, "beq"))
        return OP_BEQ;
    else if (!strcmp(s, "bne"))
        return OP_BNE;
    else if (!strcmp(s, "ble"))
        return OP_BLE;
    else if (!strcmp(s, "bleu"))
        return OP_BLEU;
    else if (!strcmp(s, "blt"))
        return OP_BLT;
    else if (!strcmp(s, "bltu"))
        return OP_BLTU;
    else if (!strcmp(s, "bge"))
        return OP_BGE;
    else if (!strcmp(s, "bgeu"))
        return OP_BGEU;
    else if (!strcmp(s, "bgt"))
        return OP_BGT;
    else if (!strcmp(s, "bgtu"))
        return OP_BGTU;
    else if (!strcmp(s, "jal"))
        return OP_JAL;
    else if (!strcmp(s, "j"))
        return OP_J;
    else if (!strcmp(s, "jalr"))
        return OP_JALR;
    else if (!strcmp(s, "ret"))
        return OP_RET;
    else if (!strcmp(s, "jr"))
        return OP_JR;
    else if (!strcmp(s, "call"))
        return OP_CALL;
    else if (!strcmp(s, "addi"))
        return OP_ADDI;
    else if (!strcmp(s, "mv"))
        return OP_MV;
    else if (!strcmp(s, "andi"))
        return OP_ANDI;
    else if (!strcmp(s, "ori"))
        return OP_ORI;
    else if (!strcmp(s, "xori"))
        return OP_XORI;
    else if (!strcmp(s, "slti"))
        return OP_SLTI;
    else if (!strcmp(s, "sltiu"))
        return OP_SLTIU;
    else if (!strcmp(s, "shlli"))
        return OP_SLLI;
    else if (!strcmp(s, "shrli"))
        return OP_SRLI;
    else if (!strcmp(s, "shrai"))
        return OP_SRAI;
    else if (!strcmp(s, "imul"))
        return OP_IMUL;
    else if (!strcmp(s, "umul"))
        return OP_MUL;
    else if (!strcmp(s, "mulih"))
        return OP_MULIH;
    else if (!strcmp(s, "muli"))
        return OP_MULI;
    else if (!strcmp(s, "idiv"))
        return OP_DIV;
    else if (!strcmp(s, "udiv"))
        return OP_UDIV;
    else if (!strcmp(s, "idivi"))
        return OP_DIVI;
    else if (!strcmp(s, "lb"))
        return OP_LB;
    else if (!strcmp(s, "lbd"))
        return OP_LBD;
    else if (!strcmp(s, "lh"))
        return OP_LH;
    else if (!strcmp(s, "lhd"))
        return OP_LHD;
    else if (!strcmp(s, "lw"))
        return OP_LW;
    else if (!strcmp(s, "lwd"))
        return OP_LWD;
    else if (!strcmp(s, "lbu"))
        return OP_LBU;
    else if (!strcmp(s, "lbud"))
        return OP_LBUD;
    else if (!strcmp(s, "lhu"))
        return OP_LHU;
    else if (!strcmp(s, "lhud"))
        return OP_LHUD;
    else if (!strcmp(s, "la"))
        return OP_LA;
    else if (!strcmp(s, "li"))
        return OP_LI;
    else if (!strcmp(s, "lui"))
        return OP_LUI;
    else if (!strcmp(s, "auipc"))
        return OP_AUIPC;
    else if (!strcmp(s, "sb"))
        return OP_SB;
    else if (!strcmp(s, "sh"))
        return OP_SH;
    else if (!strcmp(s, "sw"))
        return OP_SW;
    else if (!strcmp(s, "sbd"))
        return OP_SBD;
    else if (!strcmp(s, "shd"))
        return OP_SHD;
    else if (!strcmp(s, "swd"))
        return OP_SWD;
    else if (!strcmp(s, "copy"))
        return OP_COPY;
    else if (!strcmp(s, "copybck"))
        return OP_COPYBCK;
    else if (!strcmp(s, "copymd"))
        return OP_COPYMD;
    else if (!strcmp(s, "copydm"))
        return OP_COPYDM;
    else if (!strcmp(s, "copydd"))
        return OP_COPYDD;
    else if (!strcmp(s, "swap"))
        return OP_SWAP;
    else if (!strcmp(s, "swapmd"))
        return OP_SWAPMD;
    else if (!strcmp(s, "swapdm"))
        return OP_SWAPDM;
    else if (!strcmp(s, "swapdd"))
        return OP_SWAPDD;
    else if (!strcmp(s, "fill"))
        return OP_FILL;
    else if (!strcmp(s, "fillh"))
        return OP_FILLH;
    else if (!strcmp(s, "fillsq"))
        return OP_FILLSQ;
    else if (!strcmp(s, "fillw"))
        return OP_FILLW;
    else if (!strcmp(s, "thro"))
        return OP_THRO;
    else if (!strcmp(s, "from"))
        return OP_FROM;
    else if (!strcmp(s, "popb"))
        return OP_POPB;
    else if (!strcmp(s, "poph"))
        return OP_POPH;
    else if (!strcmp(s, "pop"))
        return OP_POP;
    else if (!strcmp(s, "pushb"))
        return OP_PUSHB;
    else if (!strcmp(s, "pushh"))
        return OP_PUSHH;
    else if (!strcmp(s, "push"))
        return OP_PUSH;
    else if (!strcmp(s, "save"))
        return OP_SAVE;
    else if (!strcmp(s, "restore"))
        return OP_RESTORE;
    else if (!strcmp(s, "exch"))
        return OP_EXCH;
    else if (!strcmp(s, "syscall"))
        return OP_CLL;
    else if (!strcmp(s, "sysret"))
        return OP_MRET;
    else if (!strcmp(s, "gsreg"))
        return OP_GSR;
    else if (!strcmp(s, "ssreg"))
        return OP_SSR;
    else if (!strcmp(s, "trace"))
        return OP_TRACE;
    else if (!strcmp(s, "cli"))
        return OP_CLI;
    else if (!strcmp(s, "sti"))
        return OP_STI;
    else
        return -1;
}

static int end_of_source(struct Lexer *l)
{
    return l->current >= l->source_len;
}

static void add_token(struct Lexer *l, struct Token *t)
{
    if (l->current_token >= SAS_MAX_TOKENS) error(ASM_ERROR_OOM);

    l->token_stream[l->current_token].type  = t->type;
    l->token_stream[l->current_token].value = t->value;

    //miniprint("ADDED token [%d] type %d\n", l->current_token, t->type);

    l->current_token++;

}

static void add_error_token(struct Lexer *l, char *lex)
{
    if (l->current_token >= SAS_MAX_TOKENS) error(ASM_ERROR_OOM);

    l->token_stream[l->current_token].type         = TOKENERROR;
    l->token_stream[l->current_token].value.number = 0;

    l->current_token++;

    //miniprint("ADDED error token [%d]\n", l->current_token);
}

static void add_unexpected_char(struct Lexer *l, char lex)
{
    if (l->current_token >= SAS_MAX_TOKENS) error(ASM_ERROR_OOM);

    l->token_stream[l->current_token].type             = UNEXPECTEDCHAR;
    l->token_stream[l->current_token].value.unexpected = lex;

    //miniprint("ADDED unexpected token [%d] %d\n", l->current_token, lex);
    l->current_token++;
}

static char peek(struct Lexer *l)
{
    if (end_of_source(l))
        return '\0';
    else
        return l->source[l->current];
}

static bool isxdigit(int c)
{
    return (((c) >= '0' && (c) <= '9') || (((c)) >= 'a' && ((c)) <= 'f') || (c >= 'A' && c <= 'F'));
}

static bool isdigit(int c)
{
    return (c) >= '0' && (c) <= '9';
}

static bool isalpha(int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

static long getnumber(struct Lexer *l, int *err)
{
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
    } else if (isdigit(c)) {
        /* number is in decimal */
        positions = 1;
        n += char_to_int[c];
        while (isdigit(peek(l))) {
            n *= 10;
            c = l->source[l->current++];
            n += char_to_int[c];
            positions++;
        }
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

static void number(struct Lexer *l)
{
    /*FIXME: THIS CAN RETURN AN INVALID NUMBER */
    struct Token t;
    char         c;
    long         n        = 0;
    int          negative = 0, err;

    if (peek(l) == '-') {
        negative = 1;                       /* negative numbers */
        c        = l->source[l->current++]; /* consume '-' */
    }

    n = getnumber(l, &err);

    if (err) {
        add_error_token(l, "");
        return;
    }

    t.type         = NUMBER;
    t.value.number = negative ? -n : n;

    add_token(l, &t);
}

static void address(struct Lexer *l)
{
    /*FIXME: THIS CAN RETURN AN INVALID NUMBER */
    struct Token  t;
    unsigned long a = 0;
    int           err;

    a = getnumber(l, &err);

    if (err) {
        add_error_token(l, "");
        return;
    }

    t.type          = ADDRESS;
    t.value.address = a;

    add_token(l, &t);
}

static void reg(struct Lexer *l)
{
    struct Token  t;
    enum Register r;
    int           err, err2 = 0;

    if (peek(l) != 'x')
        err2 = 1;
    else
        l->current++;

    r = getnumber(l, &err);

    if (err || err2) {
        add_error_token(l, "");
        return;
    }

    if (r < 0 || r > 31) {
        add_error_token(l, "");
        return;
    }

    t.type      = REGISTER;
    t.value.reg = r;

    add_token(l, &t);
}


static void identifier(struct Lexer *l) {
    struct Token t;
    int op;
    char str[32];
    usize len;

    while (isalnum(peek(l)) || peek(l) == '_') l->current++;

    len = l->current - l->start;
    if (len >= 32) error(ASM_ERROR_OOM);

    memcpy(str, &l->source[l->start], len);
    //miniprint("ASM got '%s'\n", str);
    op = which_opcode(str);
    t.type = (op == -1) ? TOKENERROR : MNEMONIC;
    t.value.opcode = op;

    add_token(l, &t);
}


static void scan_token(struct Lexer *l)
{
    static int   unexpected = 0;
    struct Token t;
    char         c = l->source[l->current++];

    switch (c) {
    case NEWLINE: /* increment line number when assigning */
    case COMMA:
    case LPAREN:
    case RPAREN:
        /* These tokens are only one character, so c == type.
        value is therefore null */
        t.type = c;

        add_token(l, &t);
        break;
    case NUMBER: number(l); break;
    case ADDRESS: address(l); break;
    case REGISTER: reg(l); break;

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
            add_unexpected_char(l, c);
            return;
        }
    }

    return;
}

/* should return errors in some way*/
void lex(struct Lexer *l)
{
    struct Token t;

    while (!end_of_source(l)) {
        /* We are at the beginning of the next lexeme. */
        l->start = l->current;
        scan_token(l);
    }

    t.type = ENDOFILE;

    add_token(l, &t);
}

// parser

static usize tok_current  = 0;

static struct Symbol *search_symbol(struct Symbol *table, const char *name);

static int insrange(int bits, int val)
{
    int msb = 1 << (bits - 1);
    int ll  = -msb;
    return ((val <= (msb - 1) && val >= ll) ? 1 : 0);
}

static int inurange(int bits, int val)
{
    int msb = 1 << bits;
    int ll  = 0;
    return ((val <= (msb - 1) && val >= ll) ? 1 : 0);
}

static usize align(usize bytes, usize addr)
{
    return ((addr + (bytes - 1)) & ~(bytes - 1)) - addr;
}

static void skip_until_newline(struct Token *stream)
{
    while (stream[tok_current++].type != NEWLINE);
    tok_current--;
    return;
}

/* expect consumes a token */
static int expect(struct Token *stream, enum TokenType t, const char *msg)
{
    struct Token tok;
    static char  unexpected[32];
    usize        i = 0;

    tok = stream[tok_current++];

    if (tok.type == t) {
        return 1;
    }

    if (tok.type == UNEXPECTEDCHAR) {
        while (tok.type == UNEXPECTEDCHAR) {
            if (i < 32) unexpected[i++] = tok.value.unexpected;
            tok = stream[tok_current++];
        }
        if (i < 32) unexpected[i] = 0;
        tok_current--;
        error(-200);
        return 0;
    }

    if (tok.type == TOKENERROR) {
        /*TODO group multiple errors*/
        error(ASM_ERROR_TOKEN);
        return 0;
    }

    error(-t);
    //miniprint("ASM expected %d, got %d\n", t, tok.type);
    tok_current--;
    return 0;
}

/* match does not consume a token */
static int match(struct Token *stream, enum TokenType t)
{
    struct Token tok;

    tok = stream[tok_current];

    if (stream[tok_current].type == t) return 1;
    return 0;
}

static struct Token *expect_and_get(struct Token *stream, enum TokenType t, const char *msg)
{
    if (expect(stream, t, msg)) {
        return &stream[tok_current - 1];
    } else {
        return NULL;
    }
}

/* immediate is either NUMBER, ADDRESS, or IDENTIFIER */
static struct Token *expect_and_get_immediate(struct Token *stream, const char *msg)
{
    if (match(stream, NUMBER)) {
        return &stream[tok_current++];
    }

    if (match(stream, ADDRESS)) {
        return &stream[tok_current++];
    }

    error(-201);
    return NULL;
}

static usize do_op(struct Token *stream)
{
    struct Token *op;
    usize         instruction_size = 0;

    op = &stream[tok_current++]; /*consume label token*/

    /* switch with all ops. some table depending on the types or something */
    switch (op->value.opcode) {
    /* format3 */
    case OP_ADD:
    case OP_SUB:
    case OP_SLL:
    case OP_SRL:
    case OP_SRA:
    case OP_AND:
    case OP_OR:
    case OP_XOR:
    case OP_SLT:
    case OP_SLTU:
    case OP_ROR:
    case OP_ROL:
    case OP_SAVE:
    case OP_RESTORE: {
        struct Token *rd, *rs1, *rs2;
        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        if (rd && rs1 && rs2)
            instruction_size += emit_be32((op->value.opcode & 0x7F) << 25 | rd->value.reg << 20 |
                                          rs1->value.reg << 15 | rs2->value.reg << 10);
        break;
    }
    /* format2NOIM */
    case OP_NOT:
    case OP_CTZ:
    case OP_CLZ:
    case OP_PCNT:
    case OP_THRO:
    case OP_FROM:
    case OP_POPB:
    case OP_POPH:
    case OP_POP:
    case OP_PUSHB:
    case OP_PUSHH:
    case OP_PUSH:
    case OP_EXCH: {
        /* opcode with two register operands */

        struct Token *rd, *rs1;

        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");

        if (rd && rs1)
            instruction_size += emit_be32((op->value.opcode << 25) | rd->value.reg << 20 |
                                          rs1->value.reg << 15 | 0);
        break;
    }
    /* format2B */
    case OP_BEQ:
    case OP_BNE:
    case OP_BLE:
    case OP_BLEU:
    case OP_BLT:
    case OP_BLTU:
    case OP_BGE:
    case OP_BGEU:
    case OP_BGT:
    case OP_BGTU: {
        /* branch instructions with a 17 bit signed relative address */
        /* HEWLP BRANCHES ARE SCARY */
        struct Token *rs1, *rs2, *tmp, *immed;
        long          value, opcode;

        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        immed =
            expect_and_get_immediate(stream, "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");

        value = (immed->type == NUMBER) ? immed->value.number : immed->value.address;
        value /= 2;

        opcode = op->value.opcode;
        if (opcode & 0x100) {
            tmp = rs2;
            rs2 = rs1;
            rs1 = tmp;
            opcode &= 0xFF;
        }

        if (rs1 && rs2 && immed)
            instruction_size += emit_be32((opcode & 0xFF) << 25 | rs1->value.reg << 20 |
                                          rs2->value.reg << 15 | (value >> 2) & 0x7FFF);

        break;
    }
    /* format1JAL */
    case OP_JAL:
    case OP_J: {
        struct Token *rd, *immed;
        enum Register reg;
        long          value;
        /* jal opcode with one register operands and label (how many bits, 20 or 22?)*/
        /* jal lab; jal #imm; jal $imm;*/
        /* jal %xrd lab; jal %xrd #imm; jal %xrd $imm;*/
        /* j is an alias for jal %x0, lab */

        if (op->value.opcode != OP_J && match(stream, REGISTER)) {
            rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
            expect(stream, COMMA, "Expected comma, instead got ");

            reg = rd->value.reg;
        } else {
            reg = (op->value.opcode == OP_J) ? X0 : X1;
        }

        immed =
            expect_and_get_immediate(stream, "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");

        value = (immed->type == NUMBER) ? immed->value.number : immed->value.address;

        if (immed) {
            instruction_size +=
                emit_be32(((op->value.opcode & 0xFF) << 25) | (reg << 20) | (value & 0xFFFFF));
        }

        break;
    }
    case OP_JALR:
    case OP_RET:
    case OP_JR: {
        /* opcode with two register operands and immediate */
        /*ret and jr are alises of jalr*/

        struct Token *rd, *rs1, *immed;
        unsigned long value, opcode;

        opcode = op->value.opcode;

        if (opcode & 0x100) {
            instruction_size += emit_be32(((opcode & 0xFF) << 25) | X0 << 20 | X1 << 15 | 0 >> 2);
            break;
        } else if (opcode & 0x200) {
            rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");

            if (rs1)
                instruction_size +=
                    emit_be32(((opcode & 0xFF) << 25) | X0 << 20 | rs1->value.reg << 15 | 0 >> 2);
            break;
        }

        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        immed =
            expect_and_get_immediate(stream, "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");
        expect(stream, LPAREN, "Expected LPAREN '(', instead got ");
        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, RPAREN, "Expected RPAREN ')', instead got ");

        value = (immed->type == NUMBER) ? immed->value.number : immed->value.address;
        value &= 0x3FFFF;

        if (rd && rs1 && immed)
            instruction_size += emit_be32(((opcode & 0xFF) << 25) | rd->value.reg << 20 |
                                          rs1->value.reg << 15 | value >> 2);

        break;
    }
    case OP_CALL: {
        /* far subroutine call */
        error(-202);
        // FIXME: this is ugly, but call does not make sense without labels

        break;
    }
    /* format2 */
    case OP_ADDI:
    case OP_MV:
    case OP_ORI:
    case OP_XORI:
    case OP_SLTI:
    case OP_SLTIU:
    case OP_SLLI:
    case OP_SRLI:
    case OP_SRAI:
    case OP_MULIH:
    case OP_MULI:
    case OP_DIVI: {
        /* opcode with two register operands and immediate */

        struct Token *rd, *rs1, *immed = NULL;

        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");

        if (op->value.opcode != OP_MV) {
            /* mv is an alias for addi %x1, %x2, #0 */
            expect(stream, COMMA, "Expected comma, instead got ");
            immed = expect_and_get_immediate(stream,
                                             "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");
        } else {
            if (rd && rs1)
                instruction_size += emit_be32(((op->value.opcode & 0xFF) << 25) |
                                              rd->value.reg << 20 | rs1->value.reg << 15 | 0);
            break;
        }

        if (rd && rs1 && immed) {
            unsigned long value;
            value = immed->type == NUMBER ? immed->value.number : immed->value.address;

            instruction_size += emit_be32((op->value.opcode << 25) | rd->value.reg << 20 |
                                          rs1->value.reg << 15 | value & 0x7FFF);
        }

        break;
    }
    /* format4 */
    case OP_IMUL:
    case OP_MUL:
    case OP_DIV:
    case OP_UDIV:
    case OP_TRACE: {
        /* opcode with three register operands */

        struct Token *rd, *rs1, *rs2, *rs3;
        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs3 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        if (rd && rs1 && rs2 && rs3)
            instruction_size += emit_be32(
                (op->value.opcode & 0x7F) << 25 | rd->value.reg << 20 | rs1->value.reg << 15 |
                rs2->value.reg << 10 | rs3->value.reg << 5 |
                ((op->value.opcode & 0x100) ? 1 : 0 /* handle udiv and umul encodings */));
        break;
    }
    /* format2L */
    case OP_LB:
    case OP_LBD:
    case OP_LH:
    case OP_LHD:
    case OP_LW:
    case OP_LWD:
    case OP_LBU:
    case OP_LBUD:
    case OP_LHU:
    case OP_LHUD: {
        /* load opcode with two one register operand and immediate (l rd, imm(rs1); l rd, label) */
        struct Token *rd, *rs1, *immed;
        unsigned long value;

        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");

        if (match(stream, NUMBER)) {
            immed = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got");
            expect(stream, LPAREN, "Expected LPAREN '(', instead got ");
            rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
            expect(stream, RPAREN, "Expected RPAREN ')', instead got ");
            if (rd && rs1 && immed) {
                instruction_size +=
                    emit_be32(op->value.opcode << 25 | rd->value.reg << 20 | rs1->value.reg << 15 |
                              ((immed->value.number) & 0x7FFF));
            }
        } else {
            error(-300);
        }

        break;
    }
    /* format la */
    case OP_LA: {
        /* load address */
        error(-203); // LA does not make sense without labels
        break;
    }
    /* format li */
    case OP_LI: {
        struct Token *rd, *immed;

        /* opcode with one register operands and immediate */

        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        immed =
            expect_and_get_immediate(stream, "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");

        if (rd && immed) {
            unsigned long value;

            value = immed->type == NUMBER ? immed->value.number : immed->value.address;

            if (insrange(15, value)) {
                instruction_size += emit_be32(OP_ADDI << 25 | rd->value.reg << 20 | value & 0x7FFF);
                break;
            } else {
                instruction_size +=
                    emit_be32(OP_LUI << 25 | rd->value.reg << 20 | (value >> 12) & 0xFFFFF);
            }

            if (rd->value.reg != 0) {
                instruction_size += emit_be32(OP_ADDI << 25 | rd->value.reg << 20 |
                                              rd->value.reg << 15 | value & 0xFFF);
            }
        }
        break;
    }
    /* format1U */
    case OP_LUI:
    case OP_AUIPC: {
        struct Token *rd, *immed;

        /* opcode with one register operand and immediate */

        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        immed =
            expect_and_get_immediate(stream, "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");

        if (rd && immed) {
            unsigned long value;

            value = immed->type == NUMBER ? immed->value.number : immed->value.address;

            instruction_size +=
                emit_be32((op->value.opcode << 25) | rd->value.reg << 20 | value & 0xFFFFF);
        }

        break;
    }
    /* format2S */
    case OP_SB:
    case OP_SH:
    case OP_SW:
    case OP_SBD:
    case OP_SHD:
    case OP_SWD: {
        /* Store instructions with a 15 bit signed operand */
        /* s %rs1, #imm(%rs2)*/
        /* s %rs1, lab, %rs2*/

        struct Token *rs1, *rs2, *immed;

        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");

        if (match(stream, NUMBER)) {
            immed = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got");
            expect(stream, LPAREN, "Expected LPAREN '(', instead got ");
            rs2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
            expect(stream, RPAREN, "Expected RPAREN ')', instead got ");
            if (rs1 && rs2 && immed) {
                instruction_size +=
                    emit_be32(op->value.opcode << 25 | rs1->value.reg << 20 | rs2->value.reg << 15 |
                              ((immed->value.number) & 0x7FFF));
            }
        } else {
            error(-301);
        }
        break;
    }
    /* format3CFS */
    case OP_COPY:
    case OP_COPYBCK:
    case OP_COPYMD:
    case OP_COPYDM:
    case OP_COPYDD:
    case OP_SWAP:
    case OP_SWAPMD:
    case OP_SWAPDM:
    case OP_SWAPDD:
    case OP_FILL:
    case OP_FILLH:
    case OP_FILLSQ:
    case OP_FILLW: {
        /* opcode with three register operands (encoding for memory addressing)*/

        struct Token *rd, *rs1, *rs2;
        int           direction;

        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");

        direction = (op->value.opcode & 0xf00) >> 8;

        if (rd && rs1 && rs2)
            instruction_size +=
                emit_be32((op->value.opcode & 0x7F) << 25 | rd->value.opcode << 20 |
                          rs1->value.opcode << 15 | rs2->value.opcode << 10 | direction & 0x7);
        break;
    }
    /* formatSCALL */
    case OP_CLL: {
        struct Token *rs1, *immed;

        /* Syscall format */
        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        immed = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got");

        if (rs1 && immed)
            instruction_size += emit_be32(op->value.opcode << 25 | rs1->value.reg << 20 |
                                          immed->value.number & 0xFF);
        break;
    }
    /* format SRET */
    case OP_MRET:
    case OP_CLI:
    case OP_STI: {
        int cli_sti;
        cli_sti = (op->value.opcode & 0xf00) >> 8;
        instruction_size += emit_be32(op->value.opcode << 25 | cli_sti << 20);
        break;
    }
    /* formatSREG */
    case OP_GSR:
    case OP_SSR: {
        struct Token *rd;

        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");

        if (rd) instruction_size += emit_be32(op->value.opcode << 25 | rd->value.reg << 20);
        break;
    }
    default: error(ASM_ERROR_NOINST); break;
    }

    return instruction_size;
}

static usize instruction(struct Token *stream)
{
    usize instruction_size = 0;

    //miniprint("Assembling, tok %d\n", stream[tok_current].type);

    switch (stream[tok_current].type) {
    case NEWLINE:
        /* empty line*/
        tok_current++; /*consume token*/
        return instruction_size;
    case MNEMONIC: instruction_size = do_op(stream); break;
    default:
        error(-1);
        tok_current++; /*consume token*/
        break;
    }

    expect(stream, NEWLINE, "Expected NEWLINE, instead got ");

    return instruction_size;
}

static usize parse_and_gen(struct Token *stream)
{
    usize sz = 0;

    while (stream[tok_current].type != ENDOFILE) {
        sz += instruction(stream);
    }

    return sz;
}

i32 sirius_asm_line(const char *line, usize line_len, usize curr_addr, u8 *out, usize out_sz)
{
    struct Lexer lexer;
    usize        sz;

    lexer.source     = line;
    lexer.source_len = line_len;
    lexer.start = lexer.current = 0;
    lexer.token_stream          = token_stream;
    lexer.current_token         = 0;

    output   = out;
    outputsz = out_sz;
    outptr   = 0;
    tok_current = 0;

    lex(&lexer);

    sz = parse_and_gen(token_stream);

    if (asm_error) return asm_error;
    return sz;
}
