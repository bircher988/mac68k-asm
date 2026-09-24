/* link.h - linker: .Link file -> CODE resources (+ /Include resources), .rsrc, .bin, .MAP. */
#ifndef MAC68K_LINK_H
#define MAC68K_LINK_H
#include "asm.h"

/* Modules and /Include files are searched in outdir, the .Link file's directory
 * and dirs; includes for the assembler come from opt->incdirs plus dirs.
 * Writes outdir/<Output>.rsrc, .bin, .MAP (and .lst if opt->listing is set to a
 * directory-less name, the linker places it in outdir). Returns 0 on success. */
int link_file(const char *linkpath, const char *outdir, const char *const *dirs, int ndirs, const AsmOptions *opt);

#endif
