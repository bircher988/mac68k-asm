/* asm.h - the 68000 assembler: modules in link order -> one relocatable code image. */
#ifndef MAC68K_ASM_H
#define MAC68K_ASM_H
#include "util.h"

typedef struct {
    const char *const *incdirs; int nincdirs;  /* searched after the including file's directory */
    int rom128;              /* nonzero: the program targets the 128K ROM (no warnings for its traps) */
    const char *listing;     /* path for the listing file, or NULL */
} AsmOptions;

typedef struct {
    unsigned char *code; size_t len;   /* code of all modules, concatenated */
    unsigned ds_total;                 /* bytes reserved by DS in the A5 globals area */
    int errors;                        /* number of errors reported */
} AsmResult;

/* Assemble the modules in the given order. Diagnostics go to stderr as
 * "file line N: message". Returns 0 on success, nonzero if errors occurred. */
int  asm_assemble(const char *const *files, int nfiles, const AsmOptions *opt, AsmResult *res);
void asm_result_free(AsmResult *res);

#endif
