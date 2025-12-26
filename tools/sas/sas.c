#include "sas.h"
#include <stdlib.h>
/* TODO: transform into a single header library that does not require malloc
         and can be architecture agnostic
*/

static int errors = 0;
static char asm_source[SAS_MAX_SOURCE_SIZE];

static char *read_asm_source(const char *file, size_t *asm_size) {
    FILE *inf;
    size_t sz, sz_read;

    inf = fopen(file, "rb");
    if (inf == NULL) {
        puts("Could not open file for reading: ");
        puts(file);
        fatal("opening input file");
    }

    fseek(inf, 0L, SEEK_END); /* check errors */
    *asm_size = ftell(inf);
    rewind(inf); /* check errors */

    if (*asm_size >= SAS_MAX_SOURCE_SIZE) {
        fatal("source does not fit in buffer");
    }

    sz = *asm_size;
    sz_read = fread(asm_source, 1, sz, inf);
    if (sz_read != sz) {
        printf("%lu != %lu\n", sz_read, sz);
        perror("On reading");
        fatal("Error reading input file");
    }

    fclose(inf);

    return asm_source;
}

static void write_object_file(const char *file, unsigned char *object_code, size_t object_size) {
    FILE *outf;
    size_t sz;

    outf = fopen(file, "wb");
    if (outf == NULL) {
        puts("Could not open file for writing: ");
        puts(file);
        fatal("writing object code");
    }

    sz = fwrite(object_code, 1, object_size, outf);
    printf("written: %lu, osize: %lu\n", sz, object_size);
    if (sz != object_size) {
        puts("Error writing object code file: ");
        puts(file);
        fatal("writing object code");
    }

    fclose(outf);
}

/* Main should  be the only part of this assembler that is not portable*/
int main(int argc, char *argv[]) {
    char *asm_filename, *obj_filename, *asm_buff;
    unsigned char *object_code;
    size_t asm_size, obj_size;
    
    if (argc != 3) {
        puts("Usage: sas <file.s> <out>");
        return 1;
    }

    asm_filename = argv[1];
    obj_filename = argv[2];

    asm_buff = read_asm_source(asm_filename, &asm_size);

    object_code = assemble(asm_buff, asm_size, &obj_size);

    if (object_code == NULL) {
        puts("Error assembling object code");
        return 1;
    }

    if (errors != 0) {
        printf("Aborting: %d errors found.\n", errors);
        return 1;
    }

    write_object_file(obj_filename, object_code, obj_size);   

    printf("OK assembled: %lu bytes.\n", obj_size);
    return 0;
}


void warning (unsigned int line, const char *msg, const char *offending) {
    static int warnings = 0;
    if (offending != NULL) 
        printf("Warning (line %u): %s (%s)\n", line, msg, offending);
    else
        printf("Warning (line %u): %s\n", line, msg);

    warnings++;
}

void error (unsigned int line, const char *msg, const char *offending) {
    if (offending != NULL) 
        printf("Error (line %u): %s (%s)\n", line, msg, offending);
    else
        printf("Error (line %u): %s\n", line, msg);

    errors++;
}

void fatal(const char *msg) {
    printf("FATAL: %s\n", msg);
    exit(1);
}


