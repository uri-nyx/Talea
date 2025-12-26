#include "sas.h"

static int object_file_format = RAW; /* raw binary by default */
static size_t tok_current = 0;
static unsigned char text_stream[SAS_MAX_TEXT_SIZE];
static size_t text_pointer = 0;
static unsigned char data_stream[SAS_MAX_DATA_SIZE];
static size_t data_pointer = 0;

static int base_address_set = 0;
static size_t base_address  = 0;

static unsigned char assembled[SAS_MAX_OBJ_SIZE];

static enum Section curr_section = SECTION_TEXT;
static size_t sections[3] = {0 , 0, 0};

static struct Symbol symtable[SAS_MAX_SYMS]; /*enough? TODO: an arena maybe?*/
static size_t symbol_current = 0;

static struct Patch patches[SAS_MAX_SYMS]; /*enough? TODO: an arena maybe?*/
static size_t patch_current = 0;

static struct Symbol *search_symbol(struct Symbol *table, const char* name);

/*addFixup(v.sym, currSeg, segPtr[currSeg], METHOD_R15, v.con);*/
static void add_patch(struct Patch *patches, size_t lineno, char *name, enum Section section, size_t offset, int method, int value) {
    if (patch_current >= SAS_MAX_PATCHES) fatal("patch buffer exhausted");

    patches[patch_current].name = name; /*The symbol name*/
    patches[patch_current].lineno = lineno; /* The line where this was made*/
    patches[patch_current].section_start = section; 
    patches[patch_current].reloc.base = 0; /*Dummy value?*/
    patches[patch_current].reloc.offset = offset;
    patches[patch_current].reloc.value = value;
    patches[patch_current].reloc.method = method;
    patch_current++;
}

static void fill_patches(struct Patch *patches, struct Symbol *symtable) {
    /* Do this without globals...uff*/
    struct Symbol *sym;
    struct Patch *patch;
    size_t i, section_start;

    if (object_file_format == RAW) {
        for (i = 0; i<patch_current; i++) {
            /*this is naive, but probably slowww*/
            patch = &patches[i];
            if (!(sym = search_symbol(symtable, patch->name)) && sym->defined) {
                error(patch->lineno, "Undefined reference", patch->name);
                continue;
            } 

            patch->reloc.value = sym->value;
            patch->reloc.base = (sym->section == SECTION_TEXT)? 0 : sections[SECTION_TEXT];

            section_start = (patch->section_start == SECTION_TEXT)? 0 : sections[SECTION_TEXT]; /*data starts after text*/
            patch->section_start = section_start;
        }
    } else if (object_file_format == AOUT) {
        /*TODO: a.out*/
    } else {
        /* unreacheable */
        fatal("Invalid file format");
    }
}

static void apply_patch_raw(struct Patch *patch, unsigned char* obj) {
    unsigned long final, value;

    switch (patch->reloc.method)
    {
    case METHOD_W32: {
        final = patch->reloc.value + patch->reloc.base;
        break;
    }
    case METHOD_R15: {
        value = (patch->reloc.value - patch->reloc.offset);
        if ((value & 3)) {
                error(patch->lineno, "branch not word aligned", NULL);
                return;
        }

        final = obj[patch->section_start + patch->reloc.offset] << 24;
        final |= obj[patch->section_start + patch->reloc.offset + 1] << 16;
        final |= obj[patch->section_start + patch->reloc.offset + 2] <<  8;
        final |= obj[patch->section_start + patch->reloc.offset + 3] <<  0;

        value &= 0x1FFFF;

        final = final | (value >> 2); /* I think I don't have to convert final to LE*/
        
        break;
    }

    case METHOD_JALR12: {
        value = (patch->reloc.value - patch->reloc.offset);
        if ((value & 3)) {
            error(patch->lineno, "branch not word aligned", NULL);
            return;
        }
        
        final = obj[patch->section_start + patch->reloc.offset] << 24;
        final |= obj[patch->section_start + patch->reloc.offset + 1] << 16;
        final |= obj[patch->section_start + patch->reloc.offset + 2] <<  8;
        final |= obj[patch->section_start + patch->reloc.offset + 3] <<  0;

        value += 4;
        value &= 0xFFF;

        final = final | (value >> 2);
        break;
    }
    case METHOD_RL12: {
        value = patch->reloc.value - patch->reloc.offset + 4 
              + patch->reloc.base - patch->section_start;

        final = obj[patch->section_start + patch->reloc.offset] << 24;
        final |= obj[patch->section_start + patch->reloc.offset + 1] << 16;
        final |= obj[patch->section_start + patch->reloc.offset + 2] <<  8;
        final |= obj[patch->section_start + patch->reloc.offset + 3] <<  0;

        value &= 0xFFF;
        final = final | value;

        break;
    }

    case METHOD_RH20: {
        value = patch->reloc.value - patch->reloc.offset
              + patch->reloc.base - patch->section_start;

        final = obj[patch->section_start + patch->reloc.offset] << 24;
        final |= obj[patch->section_start + patch->reloc.offset + 1] << 16;
        final |= obj[patch->section_start + patch->reloc.offset + 2] <<  8;
        final |= obj[patch->section_start + patch->reloc.offset + 3] <<  0;

        value = value >> 12;
        final = final | (value & 0xFFFFF);

        break;
    }
    case METHOD_RS12: {
        value = patch->reloc.value - patch->reloc.offset + 4 
              + patch->reloc.base - patch->section_start;
            
        final = obj[patch->section_start + patch->reloc.offset] << 24;
        final |= obj[patch->section_start + patch->reloc.offset + 1] << 16;
        final |= obj[patch->section_start + patch->reloc.offset + 2] <<  8;
        final |= obj[patch->section_start + patch->reloc.offset + 3] <<  0;

        value &= 0x7FFF;

        final = final | value;

        break;
    }
    case METHOD_J20: {

        value = (patch->reloc.value - patch->reloc.offset);
        if ((value & 3)) {
            error(patch->lineno, "branch not word aligned", NULL);
            return;
        }

        final = obj[patch->section_start + patch->reloc.offset] << 24;
        final |= obj[patch->section_start + patch->reloc.offset + 1] << 16;
        final |= obj[patch->section_start + patch->reloc.offset + 2] <<  8;
        final |= obj[patch->section_start + patch->reloc.offset + 3] <<  0;

        value &= 0x3FFFFF;

        final = final | (value >> 2);

        break;
    }
    default:
        break;
    }

    obj[patch->section_start + patch->reloc.offset + 0] = (final >> 24) & 0xFF;
    obj[patch->section_start + patch->reloc.offset + 1] = (final >> 16) & 0xFF;
    obj[patch->section_start + patch->reloc.offset + 2] = (final >> 8) & 0xFF;
    obj[patch->section_start + patch->reloc.offset + 3] = (final >> 0) & 0xFF;
}

static void print_sym(struct Symbol *sym) {
    char *defined, *section;
    defined = sym->defined ? "defined" : "undef";
    section = (sym->section == SECTION_TEXT) ? "text" : 
              (sym->section == SECTION_DATA) ? "data" :
              "bss";

    switch (sym->type)
    {
    case SYM_LABEL: printf("LABEL\t%s\t%s\t%s\t[%lu]\n", sym->name, section, defined, sym->value); break;
    case SYM_CONST: printf("CONST\t%s\t%s\t%s\t[%lu]\n", sym->name, section, defined, sym->value); break;
    default: printf("UNKNOW SYMBOL TYPE\n"); break;
    }
}

static void patch(struct Patch *patches, unsigned char* obj) {
    size_t i;
    struct Patch *patch;

    if (object_file_format == RAW) {
        for (i = 0; i< patch_current; i++) {
            /*this is naive, but probably slowww*/
            patch = &patches[i];
            apply_patch_raw(patch, obj);
        }
    } else if (object_file_format == AOUT) {
        /*TODO: a.out*/
    } else {
        /* unreacheable */
        fatal("Invalid file format");
    }
}

static void print_symtable(struct Symbol *table) {
    size_t i = 0;
    for (i = 0; i < symbol_current; i++) {
        print_sym(&table[i]);
    }
}

static struct Symbol *search_symbol(struct Symbol *table, const char* name) {
    size_t i = 0;
    for (i = 0; i < symbol_current; i++) {
        if (!strcmp(table[i].name, name)) return &table[i];
    }

    return NULL; /* no match found */
}

static int add_undefined_symbol(struct Symbol *table, char* name, int type) {
    if (search_symbol(table, name)) return 0; /* already defined */
    if (symbol_current >= SAS_MAX_SYMS) fatal("symbol table exhausted");
    table[symbol_current].name = name; /* this leaks? there's references in the Token*/
    table[symbol_current].defined = 0;
    table[symbol_current].type = type;
    table[symbol_current].section = curr_section;
    table[symbol_current].value = 0;
    symbol_current++;
    return 1;
}

static int add_defined_symbol(struct Symbol *table, char *name, int type, unsigned long value) {
    if (search_symbol(table, name)) return 0; /* already defined */
    if (symbol_current >= SAS_MAX_SYMS) fatal("symbol table exhausted");
    table[symbol_current].name = name; /* this leaks? there's references in the Token*/
    table[symbol_current].defined = 1;
    table[symbol_current].type = type;
    table[symbol_current].section = curr_section;
    table[symbol_current].value = value;
    symbol_current++;
    return 1;
}

static size_t emit_be8(unsigned char b) {

    switch (curr_section)
    {
    case SECTION_TEXT:
        if (text_pointer >= SAS_MAX_TEXT_SIZE - 1) fatal("text buffer exhausted");
        text_stream[text_pointer++] = b & 0xff;
        break;
    case SECTION_DATA:
        if (data_pointer >= SAS_MAX_DATA_SIZE - 1) fatal("data buffer exhausted");
        data_stream[data_pointer++] = b & 0xff;
        break;
    case SECTION_BSS:
        break;
    default:
        fatal("Unreacheable: section to emit byte unknown. failing");
        break;
    }
    
    return 1;
}

static size_t emit_be16(unsigned short h) {

    switch (curr_section)
    {
    case SECTION_TEXT:
        if (text_pointer >= SAS_MAX_TEXT_SIZE - 1) fatal("text buffer exhausted");
        text_stream[text_pointer++] = (h >> 8) & 0xff;
        text_stream[text_pointer++] = h & 0xff;
        break;
    case SECTION_DATA:
        if (data_pointer >= SAS_MAX_DATA_SIZE - 1) fatal("data buffer exhausted");
        data_stream[data_pointer++] = (h >> 8) & 0xff;
        data_stream[data_pointer++] = h & 0xff;
        break;
    case SECTION_BSS:
        break;
    default:
        fatal("Unreacheable: section to emit byte unknown. failing");
        break;
    }

    return 2;
}


static size_t emit_be32(unsigned long w) {

    switch (curr_section)
    {
    case SECTION_TEXT:
        if (text_pointer >= SAS_MAX_TEXT_SIZE - 1) fatal("text buffer exhausted");
        text_stream[text_pointer++] = (w >> 24) & 0xff;
        text_stream[text_pointer++] = (w >> 16) & 0xff;
        text_stream[text_pointer++] = (w >> 8)  & 0xff;
        text_stream[text_pointer++] =  w & 0xff;
        break;
    case SECTION_DATA:
        if (data_pointer >= SAS_MAX_DATA_SIZE - 1) fatal("data buffer exhausted");
        data_stream[data_pointer++] = (w >> 24) & 0xff;
        data_stream[data_pointer++] = (w >> 16) & 0xff;
        data_stream[data_pointer++] = (w >> 8)  & 0xff;
        data_stream[data_pointer++] =  w & 0xff;
        break;
    case SECTION_BSS:
        break;
    default:
        fatal("Unreacheable: section to emit byte unknown. failing");
        break;
    }

    return 4;
}

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

static size_t align(size_t bytes, size_t addr) {
    return ((addr + (bytes - 1)) & ~(bytes - 1)) - addr;
}

static void skip_until_newline(struct Token *stream) {
    while (stream[tok_current++].type != NEWLINE);
    tok_current--;
    return;
}


/* expect consumes a token */
static int expect(struct Token *stream, enum TokenType t, const char *msg) {
    struct Token tok;
    static char unexpected[32];
    size_t i = 0;

    tok = stream[tok_current++];

    /* patch a constant */
    if ((t == NUMBER || t == ADDRESS) && tok.type == IDENTIFIER) {
        struct Symbol *sym;
        if ((sym = search_symbol(symtable, tok.value.identifier))) {
            if (sym->type == SYM_CONST || sym->defined) {
                stream[tok_current -1 ].type = t;
                if (t == NUMBER) stream[tok_current -1 ].value.number = sym->value;
                else stream[tok_current -1 ].value.address = sym->value;
                return 1;
            }
        }
    }

    if (tok.type == t) {
        return 1;
    } 

    if (tok.type == UNEXPECTEDCHAR) {
        while (tok.type == UNEXPECTEDCHAR) {
            if (i<32) unexpected[i++] = tok.value.unexpected;
            tok = stream[tok_current++];
        }
        if (i < 32) unexpected[i] = 0;
        tok_current--;
        if (msg) error(stream[tok_current-1].line, msg, unexpected);
        return 0;
    }

    if (tok.type == TOKENERROR) {
        /*TODO group multiple errors*/
        if (msg) error(tok.line, tok.value.string, tok.lex);
        if (msg) error(tok.line, msg, tok.lex);
        return 0;
    }

    if (msg) error(tok.line, msg, tok.lex);
    tok_current--;
    return 0;
}

/* match does not consume a token */
static int match(struct Token *stream, enum TokenType t) {
    struct Token tok;

    tok = stream[tok_current];
    /* patch a constant */
    if ((t == NUMBER || t == ADDRESS) && tok.type == IDENTIFIER) {
        struct Symbol *sym;
        if ((sym = search_symbol(symtable, tok.value.identifier))) {
            if (sym->type == SYM_CONST && sym->defined) {
                stream[tok_current].type = t;
                if (t == NUMBER) stream[tok_current].value.number = sym->value;
                else stream[tok_current].value.address = sym->value;
                return 1;
            }
        }
    }

    if (stream[tok_current].type == t) return 1;
    return 0;
}


static struct Token *expect_and_get(struct Token *stream, enum TokenType t, const char *msg) {
    if (expect(stream, t, msg)) {
        return &stream[tok_current - 1];
    } else {
        return NULL;
    } 
}

/* immediate is either NUMBER, ADDRESS, or IDENTIFIER */
static struct Token *expect_and_get_immediate(struct Token *stream, const char *msg) {
    if (match(stream, NUMBER)) {
        return &stream[tok_current++];
    }

    if (match(stream, ADDRESS)) {
        return &stream[tok_current++];
    }

    if (match(stream, IDENTIFIER)) {
        return &stream[tok_current++];
    }

    error(stream[tok_current++].line, msg, stream[tok_current++].lex);
    return NULL;
}



static size_t do_op(struct Token *stream) {
    struct Token *op;
    size_t instruction_size = 0;
    op = &stream[tok_current++]; /*consume label token*/

    if (curr_section != SECTION_TEXT) {
        error(stream[tok_current].line, "Can only emit instructions in TEXT section", stream[tok_current].lex);
        skip_until_newline(stream);
        return 0;
    }

    /* switch with all ops. some table depending on the types or something */
    switch (op->value.opcode)
    {
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
    case OP_RESTORE: 
    {
        struct Token *rd, *rs1, *rs2;
        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        if (rd && rs1 && rs2)
            instruction_size += emit_be32((op->value.opcode & 0x7F) << 25 | rd->value.reg << 20 | rs1->value.reg << 15 | rs2->value.reg << 10);
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
            instruction_size += emit_be32((op->value.opcode << 25) | rd->value.reg << 20 | rs1->value.reg << 15 | 0);
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
        long value, opcode;

        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        immed = expect_and_get_immediate(stream, "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");

        /* if (token == TOK_DOTRELADR)
                getToken(); what is this

        
        if (v.sym == NULL) {
                immed = v.con / 2;
        } else {
                addFixup(v.sym, currSeg, segPtr[currSeg], METHOD_R15, v.con);
                immed = 0;
        }
        */

        if (immed->type == IDENTIFIER) {
            /* RELOCATIONS */
            value = 0;
            add_patch(patches, immed->line,  immed->value.identifier, curr_section, sections[curr_section], METHOD_R15, value); /*is this value right?*/
        } else {
            value = (immed->type == NUMBER) ? immed->value.number : immed->value.address;
            value /= 2;
        }

        opcode = op->value.opcode;
        if (opcode & 0x100) {
                tmp  = rs2;
                rs2 = rs1;
                rs1 = tmp;
                opcode &= 0xFF;
        }
        
        if (rs1 && rs2 && immed)
            instruction_size += emit_be32((opcode & 0xFF) << 25 | rs1->value.reg << 20 | rs2->value.reg << 15 |
                 (value >> 2) & 0x7FFF);

        break;
    }
    /* format1JAL */
    case OP_JAL:
    case OP_J: {
        struct Token   *rd, *immed;
        enum Register reg;
        long value;
        /* jal opcode with one register operands and label (how many bits, 20 or 22?)*/
        /* jal lab; jal #imm; jal $imm;*/
        /* jal %xrd lab; jal %xrd #imm; jal %xrd $imm;*/
        /* j is an alias for jal %x0, lab */

        if (op->value.opcode != OP_J && match(stream, REGISTER)) {
            rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
            expect(stream, COMMA, "Expected comma, instead got ");
            
            reg = rd->value.reg;
        } else {
            reg = (op->value.opcode == OP_J)? X0 : X1;
        }

        immed = expect_and_get_immediate(stream, "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");

        if (immed->type == IDENTIFIER) {
            value = 0;
            add_patch(patches, immed->line,  immed->value.identifier, curr_section, sections[curr_section], METHOD_J20, value); /*is this value right?*/
        } else {
            value = (immed->type == NUMBER) ? immed->value.number : immed->value.address;
        }
        
        if (immed) {
            instruction_size += emit_be32(((op->value.opcode & 0xFF) << 25) | (reg << 20) | (value & 0xFFFFF));
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
        } else if  (opcode & 0x200) {
            rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");

            if (rs1)
                instruction_size += emit_be32(((opcode & 0xFF) << 25) | X0 << 20 | rs1->value.reg << 15 | 0 >> 2);
            break;
        }

        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        immed = expect_and_get_immediate(stream, "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");
        expect(stream, LPAREN, "Expected LPAREN '(', instead got ");
        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, RPAREN, "Expected RPAREN ')', instead got ");

        if (immed->type == IDENTIFIER) {
            error(immed->line, "jalr instruction does not support labels", immed->lex);
            skip_until_newline(stream);
            break;
        }

        value = (immed->type == NUMBER) ? immed->value.number : immed->value.address;
        value &= 0x3FFFF;

        if (rd && rs1 && immed)
            instruction_size += emit_be32(((opcode & 0xFF) << 25) | rd->value.reg << 20 | rs1->value.reg << 15 | value >> 2);

        break; 
    }
    case OP_CALL: {
        /* far subroutine call */

        struct Token *immed;

        immed = expect_and_get(stream, IDENTIFIER, "Expected IDEN, instead got");
        if (immed) {
            add_patch(patches, immed->line,  immed->value.identifier, curr_section, sections[curr_section], METHOD_RH20, 0); /*is this value right?*/
            instruction_size += emit_be32(OP_AUIPC << 25 | X1 << 20 | 0 /*immed value*/);
            add_patch(patches, immed->line,  immed->value.identifier, curr_section, sections[curr_section] + 4, METHOD_JALR12, 0); /*is this value right?*/
            instruction_size += emit_be32(OP_JALR << 25 | X1 << 20 | X1 << 15 |  0 /*immed value*/);
        }
   
        break;
    }
    /* format2 */
    case OP_ADDI:
    case OP_MV:
    case OP_ORI:
    case OP_ANDI:
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
            immed = expect_and_get_immediate(stream, "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");
        } else {
            if(rd && rs1) 
                instruction_size += emit_be32(((op->value.opcode & 0xFF) << 25) | rd->value.reg << 20 | rs1->value.reg << 15 | 0);
            break;
        }

        if(rd && rs1 && immed) {
            unsigned long value;

            if (immed->type == IDENTIFIER) {
                error(immed->line, "Cannot use non constant identifiers as immediate value", immed->lex);
                skip_until_newline(stream);
                break;
            }
            else value = immed->type == NUMBER ? immed->value.number : immed->value.address;

            if ((immed->type == NUMBER && ((signed)value < -16384) || (signed)value > 16383) 
            || (immed->type == ADDRESS && value > 0x7FFF)) 
                warning(immed->line, "Immediate is only 15 bits, it will be truncated.", immed->lex);

            instruction_size += emit_be32((op->value.opcode << 25) | rd->value.reg << 20 | rs1->value.reg << 15 | value & 0x7FFF);
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
            instruction_size += emit_be32((op->value.opcode & 0x7F) << 25 | rd->value.reg << 20 
                | rs1->value.reg << 15 | rs2->value.reg << 10 | rs3->value.reg << 5 |
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
    case OP_LHUD:
    {
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
                if (immed->value.number < -16384 || immed->value.number > 16383) 
                    warning(immed->line, "Immediate is only 15 bits, it will be truncated.", immed->lex);
                instruction_size += emit_be32(op->value.opcode << 25 | rd->value.reg << 20 | rs1->value.reg << 15 | ((immed->value.number) & 0x7FFF));
            }
        } else if (match(stream, IDENTIFIER)) {
            immed = expect_and_get(stream, IDENTIFIER, "Expected IDENT, instead got");
            add_patch(patches, immed->line,  immed->value.identifier, curr_section, sections[curr_section], METHOD_RH20, 0); /*is this value right?*/
            instruction_size += emit_be32(OP_AUIPC << 25 | rd->value.reg << 20 | 0);
            add_patch(patches, immed->line,  immed->value.identifier, curr_section, sections[curr_section] + 4, METHOD_RL12, 0); /*is this value right?*/
            rs1 = rd;
            instruction_size += emit_be32(op->value.opcode << 25 | rd->value.reg << 20 | rs1->value.reg << 15 | ((0) & 0x7FFF));
        } else {
            error(op->line, "Expected either format 'l %rd, #immed(%rs1)' or 'l %rd, ident' at", stream[tok_current++].lex);
        }

        break;
    }
    /* format la */
    case OP_LA:  {
        /* load address */
        struct Token *rd, *immed;

        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        immed = expect_and_get(stream, IDENTIFIER, "Expected IDEN, instead got");

        if (rd && immed) {
            add_patch(patches, immed->line,  immed->value.identifier, curr_section, sections[curr_section], METHOD_RH20, 0); /*is this value right?*/
            instruction_size += emit_be32(OP_AUIPC << 25 | rd->value.reg << 20 | /*immed value*/ 0);
            add_patch(patches, immed->line,  immed->value.identifier, curr_section, sections[curr_section] + 4, METHOD_RL12, 0); /*is this value right?*/
            instruction_size += emit_be32(OP_ADDI << 25 | rd->value.reg << 20 | rd->value.reg << 15 | /*immed value*/ 0);
        }
        break;
    }
    /* format li */
    case OP_LI: {
        struct Token *rd, *immed;

        /* opcode with one register operands and immediate */
 
        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        immed = expect_and_get_immediate(stream, "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");

        if(rd && immed) {
            unsigned long value;
            if (immed->type == IDENTIFIER) {
                error(immed->line, "Cannot use non constant identifiers as immediate value", immed->lex);
                skip_until_newline(stream);
                break;
            }
            else value = immed->type == NUMBER ? immed->value.number : immed->value.address;

            if (insrange(15, value)) {
                instruction_size += emit_be32(OP_ADDI << 25 | rd->value.reg << 20 | value & 0x7FFF);
                break;
            } else {
                instruction_size += emit_be32(OP_LUI << 25 | rd->value.reg << 20 | (value >> 12) & 0xFFFFF);
            }

            if (rd->value.reg != 0) {
                instruction_size += emit_be32(OP_ADDI << 25 | rd->value.reg << 20 | rd->value.reg << 15 | value & 0xFFF);
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
        immed = expect_and_get_immediate(stream, "Expected NUMBER, ADDRESS or IDENTIFIER, instead got");

        if(rd && immed) {
            unsigned long value;

            if (immed->type == IDENTIFIER) {
                error(immed->line, "Cannot use non constant identifiers as immediate value", immed->lex);
                skip_until_newline(stream);
                break;
            }
            else value = immed->type == NUMBER ? immed->value.number : immed->value.address;

            if ((immed->type == NUMBER && ((signed)value < -524288 ) || (signed)value > 524287) 
            || (immed->type == ADDRESS && value > 0xFFFFF)) 
                warning(immed->line, "Immediate is only 20 bits, it will be truncated.", immed->lex);

            instruction_size += emit_be32((op->value.opcode << 25) | rd->value.reg << 20 | value & 0xFFFFF);
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
                if (immed->value.number < -16384 || immed->value.number > 16383) 
                    warning(immed->line, "Immediate is only 15 bits, it will be truncated.", immed->lex);
                instruction_size += emit_be32(op->value.opcode << 25 | rs1->value.reg << 20 | rs2->value.reg << 15 | ((immed->value.number) & 0x7FFF));
            }
        } else if (match(stream, IDENTIFIER)) {

            immed = expect_and_get(stream, IDENTIFIER, "Expected IDEN, instead got");
            expect(stream, COMMA, "Expected comma, instead got ");
            rs2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
           
            add_patch(patches, immed->line,  immed->value.identifier, curr_section, sections[curr_section], METHOD_RH20, 0); /*is this value right?*/
            instruction_size += emit_be32(OP_AUIPC << 25 | rs2->value.reg << 20 | 0);
            add_patch(patches, immed->line,  immed->value.identifier, curr_section, sections[curr_section] + 4, METHOD_RS12, 0); /*is this value right?*/
            instruction_size += emit_be32(op->value.opcode << 25 | rs1->value.reg << 20 | rs2->value.reg << 15 | ((0) & 0x7FFF));
        } else {
            error(op->line, "Expected either format 's %rs1, #immed(%rs2)' or 'l %rs1, ident, %rs2' at", stream[tok_current++].lex);
        }
        break;
    }
    /* format3CFS */
    case OP_COPY:
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
        int direction;

        rd = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        rs2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");

        direction = (op->value.opcode & 0xf00) >> 8;
        
        if(rd && rs1 && rs2)
            instruction_size += emit_be32((op->value.opcode  & 0x7F) << 25 
            | rd->value.opcode << 20 | rs1->value.opcode << 15 | rs2->value.opcode << 10 
            | direction & 0x3);
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
            instruction_size += emit_be32(op->value.opcode << 25 | rs1->value.reg << 20 | immed->value.number & 0xFF);
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
    /* formatMMU */
    case OP_MMUTOG:
    case OP_MMUGPT: { 
        struct Token *r1;
        r1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        instruction_size += emit_be32(op->value.opcode << 25 | r1->value.reg << 20);
        break;
    }
    case OP_MMUMAP: {
        struct Token *r1, *r2, *r3, *r4, *w, *x;
        r1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r3 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r4 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        w = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got");
        expect(stream, COMMA, "Expected comma, instead got ");
        x = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got");

        if (r1 && r2 && r3 && r4 && w && x)
            instruction_size += emit_be32(op->value.opcode << 25 | r1->value.reg << 20 | r2->value.reg << 15 | r3->value.reg << 10 | r4->value.reg << 5 
            | (w->value.number & 0x1) << 1 | (x->value.number & 0x1));
        break;
    }
    case OP_MMUUNMAP:    
    case OP_UMODETOG: {   
        struct Token *r1, *r2;
        r1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        
        if (r1 && r2)
            instruction_size += emit_be32(op->value.opcode << 25 | r1->value.reg << 20 | r2->value.reg << 15);
        break;
    }
    case OP_MMUSTAT: {
        struct Token *r1, *r2, *r3;
        r1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r3 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
       
        
        if (r1 && r2 && r3)
            instruction_size += emit_be32(op->value.opcode << 25 | r1->value.reg << 20 | r2->value.reg << 15 | r3->value.reg << 10);
        break;
    }
    case OP_MMUGETFRAME: {
        struct Token *r1, *r2, *r3;
        r1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r3 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        
        if (r1 && r2)
            instruction_size += emit_be32((op->value.opcode & 0xF) << 25 | r1->value.reg << 20 | r2->value.reg << 15 | r3->value.reg << 10 | 1);
        break;
    }
    case OP_MMUSPT: {
        struct Token *r1, *r2, *immed;
        r1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        immed = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got");

        if (r1 && r2 && immed)
            instruction_size += emit_be32(op->value.opcode << 25 | r1->value.reg << 20 | r2->value.reg << 15 | (immed->value.number & 0xfff));
        break;
    }
    case OP_MMUUPDATE: {
        struct Token *r1, *r2, *r3, *pt, *dirty, *present;
        r1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r3 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        pt = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got");
        expect(stream, COMMA, "Expected comma, instead got ");
        dirty = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got");
        expect(stream, COMMA, "Expected comma, instead got ");
        present = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got");

        if (r1 && r2 && r3 && pt && dirty && present)
            instruction_size += emit_be32(op->value.opcode << 25 
            | r1->value.reg << 20 | r2->value.reg << 15 | r3->value.reg << 10 
            | (pt->value.number & 0xf) << 2 | (dirty->value.number & 0x1) << 1 | (present->value.number & 0x1));
        break;
}
    case OP_MMUSW: {
        struct Token *r1, *r2, *pt, *clone;
        r1 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        r2 = expect_and_get(stream, REGISTER, "Expected Register, instead got ");
        expect(stream, COMMA, "Expected comma, instead got ");
        pt = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got");
        expect(stream, COMMA, "Expected comma, instead got ");
        clone = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got");

        if (r1 && r2 && pt && clone)
            instruction_size += emit_be32(op->value.opcode << 25 | r1->value.reg << 20 | r2->value.reg << 15 | (clone->value.number & 0xf) << 4 | (pt->value.number & 0xf));
        break;
    }
    default:
        error(op->line, "Unknown instruction ", op->lex);
        break;
    }

    return instruction_size;

}

static size_t do_label(struct Token *stream) {
    struct Token *label;
    size_t instruction_size = 0;

    label = &stream[tok_current++]; /*consume label token*/
    
    if (expect(stream, COLON, NULL)) {
        if (!add_defined_symbol(symtable, label->lex, SYM_LABEL, sections[curr_section]))
            error(label->line, "Redefined symbol ", label->lex);
    } else {
        error(label->line, "Unknown instruction: ", label->lex);
        skip_until_newline(stream);
        return 0;
    }

    if (match(stream, MNEMONIC)) {
        instruction_size = do_op(stream);
    } 

    return instruction_size;
}

static size_t do_directive(struct Token *stream) {
    /*TODO: Expand to accept multiple comma separated values. ADD INSTRUCTION SIZES*/
    struct Token *d;
    size_t instruction_size = 0;
    d = &stream[tok_current++]; /*consume label token*/

    /* switch with all ds. some table depending on the types or something */
    switch (d->value.directive)
    {
    case BYTE: {
        struct Token *n;

        if (curr_section == SECTION_BSS) {
            error(stream[tok_current].line, "Cannot emit initialized data on BSS", stream[tok_current].lex);
            skip_until_newline(stream);
            break;
        }

        if (match(stream, NUMBER)) {
            n = expect_and_get(stream, NUMBER, "Expected 8 bit number, instead got ");
            if (n) {
                if (n->value.number > 255 || n->value.number < -128) error(n->line, ".byte directive only accepts 8 bit wide integers", n->lex);
                else instruction_size += emit_be8(n->value.number);
            } 
        } else {
            n = expect_and_get(stream, STRING, "Expected a string, instead got ");
            if (n) {
                char c, *p;
                p = n->value.string;
                while ((c = *p++)) instruction_size += emit_be8(c);
            }
        }
        break;
    }
    case HALF: {
        struct Token *n;

        if (curr_section == SECTION_BSS) {
            error(stream[tok_current].line, "Cannot emit initialized data on BSS", stream[tok_current].lex);
            skip_until_newline(stream);
            break;
        }

        n = expect_and_get(stream, NUMBER, "Expected 16 bit number, instead got ");
        if (n) {
            if (n->value.number > 65535 || n->value.number < -32768) error(n->line, ".half directive only accepts 16 bit wide integers", n->lex);
            else instruction_size += emit_be16(n->value.number);
        } 
        break;
    }
    case WORD: {
        struct Token *arg;

        if (curr_section == SECTION_BSS) {
            error(stream[tok_current].line, "Cannot emit initialized data on BSS", stream[tok_current].lex);
            skip_until_newline(stream);
            break;
        }

        if (match(stream, NUMBER)) {
            arg = expect_and_get(stream, NUMBER, "Expected 32 bit number, instead got ");
            if (arg->value.number > 4294967295 || arg->value.number < -2147483648) error(arg->line, ".word directive accepts 32 bit wide integers", arg->lex);
            else instruction_size += emit_be32(arg->value.number);
        } else if (match(stream, ADDRESS)) {
            arg = expect_and_get(stream, ADDRESS, "Expected an address, instead got ");
            if (arg) instruction_size += emit_be32(arg->value.number);
        } else if (match(stream, IDENTIFIER)) {
            arg = expect_and_get(stream, IDENTIFIER, "Expected a label, instead got ");
            if (arg) { 
                add_patch(patches, arg->line, arg->value.identifier, curr_section, sections[curr_section], METHOD_W32, 0);
                instruction_size += emit_be32(0);
            }
        } else {
            error(stream[tok_current].line, "Syntax error, .word directive accepts a label, a number or an address, got", stream[tok_current].lex);
            tok_current++;
        }

        break;
    }
    case TEXT: {
        curr_section = SECTION_TEXT;
        break;
    }
    case DATA: {
        curr_section = SECTION_DATA;
        break;
    }
    case BSS: {
        curr_section = SECTION_BSS;
        break;
    }
    case ALIGN: {
        struct Token *arg;
        size_t padding;
        int n;

        arg = expect_and_get(stream, NUMBER, "Expected NUMBER bit number, instead got ");
        n = arg->value.number;

        /* check if its power of 2*/
        if ((n <= 0) || !(n & (n - 1)) == 0) {
            error(arg->line, ".align directive accepts only power of 2 (in bytes)", arg->lex);
            break;
        }

        padding = align(n, sections[curr_section]);
        instruction_size = padding;

        while (padding--) {
            emit_be8(0);
        }

        break;
    }
    case FORMAT: {
        struct Token *arg;
        size_t padding;
        int i;

        if (d->line != 1) {
            error(d->line, ".format directive can only appear at line 1!", d->lex);
            skip_until_newline(stream);
            break;
        }

        arg = expect_and_get(stream, STRING, "Expected STRING (\"a.out\" or \"raw\"), instead got ");

        for (i = 0; i<FORMATS_N; i++) {
            if (!strcmp(arg->value.string, formats[i])) {
                object_file_format = i;
                goto endcase;
            }
        }

        error(arg->line, "Object file format not supported", arg->lex);
        endcase: break;
    }
    case ORG: {
        /*TODO: change this so it does not make the file bigger*/
        struct Token *arg;
        unsigned long addr;

        if (object_file_format == AOUT) {
            error(d->line, ".org directive not allowed in a.out format", d->lex);
            skip_until_newline(stream);
            break;
        }

        arg = expect_and_get(stream, ADDRESS, "Expected ADDRESS, instead got ");
        addr = arg->value.address;

        if (curr_section == SECTION_TEXT && (addr % 4 != 0 )) {
            warning(arg->line, "setting origin with .org directive in .text section to an address that is not 32 bit aligned. Expect faults!", arg->lex);
        }

        if (curr_section != SECTION_TEXT) {
            error(d->line, ".org directive only allowed on .text section", arg->lex);
        }
        if (!base_address_set) {
            base_address = addr;
            base_address_set = 1;
            /* Don't advance the pointer yet, this IS our start */
            text_pointer = 0; 
        } else {
            /* Handle subsequent .orgs as padding relative to base */
            text_pointer = addr - base_address;
        }
        break;
    }
    case CONST: {
        struct Token *name, *value;
        unsigned long v;

        name  = expect_and_get(stream, IDENTIFIER, "Expected IDENTIFIER, instead got ");
        if (match(stream, NUMBER)) {
            value = expect_and_get(stream, NUMBER, "Expected NUMBER, instead got ");
            v = value->value.number;
        } else {
            value = expect_and_get(stream, ADDRESS, "Expected ADDRES, instead got ");
            v = value->value.number;
        }

        if (!add_defined_symbol(symtable, name->lex, SYM_CONST, v))
            error(name->line, "Redefined symbol ", name->lex);
        break;
    }
    default:
        error(d->line, "Unknown directive ", d->lex);
        break;
    }

    return instruction_size;
}


static size_t instruction(struct Token *stream) {
    size_t instruction_size = 0;

    switch (stream[tok_current].type)
    {
    case NEWLINE:
        /* empty line*/
        tok_current++; /*consume token*/
        return instruction_size;
    case IDENTIFIER:
        /* a label */
        instruction_size = do_label(stream);
        break;
    case MNEMONIC:
        instruction_size = do_op(stream);
        break;
    case DIRECTIVE:
        instruction_size = do_directive(stream);
        break;
    default:
        error(stream[tok_current].line, "Syntax error, statements must be started by a label, a mnemonic or a directive", stream[tok_current].lex);
        print_token(stream[tok_current]);
        tok_current++; /*consume token*/
        break;
    }

    expect(stream, NEWLINE, "Expected NEWLINE, instead got ");

    return instruction_size;

}

static void parse_and_gen(struct Token *stream) {
    unsigned int padding = 0;

    while (stream[tok_current].type != ENDOFILE) {
        sections[curr_section] += instruction(stream);
    }

    padding = align(4, sections[SECTION_TEXT]);
    sections[SECTION_TEXT] += padding;

    padding = align(4, sections[SECTION_DATA]);
    sections[SECTION_DATA] += padding;
    
}

unsigned char *assemble(const char *asm_source, size_t source_len, size_t *object_size) {
    struct Lexer lexer;
    size_t i = 0, file_size;

    lexer.source = asm_source;
    lexer.source_len = source_len;
    lexer.start = lexer.current = 0;
    lexer.lineno = 1;
    lexer.token_stream = token_stream;
    lexer.current_token = 0;

    lex(&lexer);

#ifdef SAS_DEBUG
    for (i = 0; i < lexer.current_token; i++) {
        print_token(token_stream[i]);
    }
#endif

    /* this shoud not be necessary on a sane system, bss should be 0,
        but it does not hurt
    */
    memset(text_stream, 0, SAS_MAX_TEXT_SIZE); 
    memset(data_stream, 0, SAS_MAX_DATA_SIZE); 
    memset(assembled, 0, SAS_MAX_OBJ_SIZE);
 

    parse_and_gen(token_stream);

    printf("TEXT: %d, DATA: %d, BSS: %d\n", sections[SECTION_TEXT], sections[SECTION_DATA], sections[SECTION_BSS]);

#ifdef SAS_DEBUG
    printf("SYMBOLS:\n");
    print_symtable(symtable);
#endif

    
    file_size    = sections[SECTION_TEXT]
                 + sections[SECTION_DATA]
                 + sections[SECTION_BSS];
    *object_size = file_size + align(4, file_size);

    fill_patches(patches, symtable);
    printf("Passed fill patches\n");

    memcpy(assembled, text_stream, sections[SECTION_TEXT]);
    memcpy(assembled + sections[SECTION_TEXT], data_stream, sections[SECTION_DATA]);
    /* I don't have to copy the bss right?*/
    printf("Patching.\n");

    patch(patches, assembled);
    printf("Patched.\n");

    return assembled;
}