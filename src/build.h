/* build.h - runs a .Job file (ASM / LINK / RMAKER lines). */
#ifndef MAC68K_BUILD_H
#define MAC68K_BUILD_H
#include "asm.h"

/* Files named in the job are looked up in the job's directory and dirs.
 * Returns 0 on success. Prints "DONE: <app.bin>" for the first APPL produced. */
int build_job(const char *jobpath, const char *outdir, const char *const *dirs, int ndirs, const AsmOptions *opt);

#endif
