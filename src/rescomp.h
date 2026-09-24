/* rescomp.h - resource compiler: .R text -> resource list / files. */
#ifndef MAC68K_RESCOMP_H
#define MAC68K_RESCOMP_H
#include "resfork.h"

/* Compile an .R file. INCLUDEd files are searched in outdir, the .R file's
 * directory and dirs. Writes outdir/<Name>.rsrc and outdir/<Name>.bin and prints
 * a one-line summary. Returns 0 on success. */
int rescomp_file(const char *rpath, const char *outdir, const char *const *dirs, int ndirs);

#endif
