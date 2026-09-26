/* main.c - mac68k-asm command line: asm, link, res, build, version, help. */
#include "asm.h"
#include "build.h"
#include "link.h"
#include "rescomp.h"
#include "resfork.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAC68K_VERSION "1.1"

static void usage(FILE *f) {
    fputs("usage: mac68k-asm asm   <Module.Asm>... [-I dir]... [-o out.raw] [-l listing]\n"
          "       mac68k-asm link  <File.Link> [-o outdir] [-I dir]...\n"
          "       mac68k-asm res   <File.R>    [-o outdir] [-I dir]...\n"
          "       mac68k-asm build <File.Job>  [-o outdir] [-I dir]...\n"
          "       mac68k-asm version\n"
          "       mac68k-asm help\n"
          "\n"
          "  asm    assemble modules in link order into one relocatable code image\n"
          "  link   assemble and link a .Link file -> <Output>.rsrc/.bin/.MAP/.lst\n"
          "  res    compile an .R resource file -> <Name>.rsrc/.bin\n"
          "  build  run a .Job file (ASM / LINK / RMAKER lines), report the APPL produced\n"
          "\n"
          "  -I dir   search directory for modules, includes and resource files\n"
          "  -o       output file (asm) or output directory (link, res, build; default .)\n"
          "  -l file  listing file (asm)\n"
          "\n"
          "environment: MAC68K_INC (built-in include directory), MAC68K_ROM=128 (target the 128K ROM: no warnings for its traps)\n", f);
}

static int bad_usage(void) { usage(stderr); return 2; }

/* The built-in include directory: $MAC68K_INC, <exe>/../share/mac68k-asm/inc or <exe>/inc. */
static char *builtin_inc(void) {
    const char *env = getenv("MAC68K_INC");
    if (env && *env) return xstrdup(env);
    char *ed = exe_dir();
    if (!ed) return NULL;
    char *p = xsprintf("%s/../share/mac68k-asm/inc", ed);
    if (dir_exists(p)) { free(ed); return p; }
    free(p);
    p = xsprintf("%s/inc", ed);
    if (dir_exists(p)) { free(ed); return p; }
    free(p); free(ed);
    return NULL;
}

int main(int argc, char **argv) {
    prog_name = "mac68k-asm";
    if (argc < 2) return bad_usage();
    const char *cmd = argv[1];
    if (!strcmp(cmd, "version") || !strcmp(cmd, "--version") || !strcmp(cmd, "-V")) {
        printf("mac68k-asm %s\n", MAC68K_VERSION);
        return 0;
    }
    if (!strcmp(cmd, "help") || !strcmp(cmd, "--help") || !strcmp(cmd, "-h")) { usage(stdout); return 0; }
    int is_asm = !strcmp(cmd, "asm"), is_link = !strcmp(cmd, "link"), is_res = !strcmp(cmd, "res"), is_build = !strcmp(cmd, "build");
    if (!is_asm && !is_link && !is_res && !is_build) {
        fprintf(stderr, "mac68k-asm: unknown command '%s'\n", cmd);
        return bad_usage();
    }

    StrList files = {0}, dirs = {0};
    const char *out = NULL, *listing = NULL;
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-I")) { if (++i >= argc) return bad_usage(); strlist_add(&dirs, argv[i]); }
        else if (!strncmp(argv[i], "-I", 2) && argv[i][2]) strlist_add(&dirs, argv[i] + 2);
        else if (!strcmp(argv[i], "-o")) { if (++i >= argc) return bad_usage(); out = argv[i]; }
        else if (!strcmp(argv[i], "-l")) { if (++i >= argc || !is_asm) return bad_usage(); listing = argv[i]; }
        else if (argv[i][0] == '-' && argv[i][1]) { fprintf(stderr, "mac68k-asm: unknown option '%s'\n", argv[i]); return bad_usage(); }
        else strlist_add(&files, argv[i]);
    }
    if (files.n == 0 || (!is_asm && files.n != 1)) return bad_usage();

    /* assembler include path: all -I dirs, then the built-in inc dir */
    StrList inc = {0};
    for (int i = 0; i < dirs.n; i++) strlist_add(&inc, dirs.v[i]);
    char *bi = builtin_inc();
    if (bi) { strlist_add(&inc, bi); free(bi); }
    AsmOptions opt = {0};
    opt.incdirs = (const char *const *)inc.v; opt.nincdirs = inc.n;
    const char *rom = getenv("MAC68K_ROM");          /* 64 (default) or 128 */
    const char *rom_old = getenv("MAC68K_ROM128");
    opt.rom128 = (rom && atoi(rom) >= 128) || (rom_old && *rom_old && strcmp(rom_old, "0") != 0);
    opt.listing = listing;

    if (is_asm) {
        AsmResult res = {0};
        int rc = asm_assemble((const char *const *)files.v, files.n, &opt, &res);
        if (rc || res.errors) { fprintf(stderr, "asm: %d error(s)\n", res.errors ? res.errors : 1); return 1; }
        char *outpath = out ? xstrdup(out) : xsprintf("%s.raw", path_strip_ext(path_basename(files.v[0])));
        if (write_binary_file(outpath, res.code, res.len)) { fprintf(stderr, "asm: cannot write %s\n", outpath); return 1; }
        printf("asm: code %zu bytes, globals %u bytes -> %s\n", res.len, res.ds_total, outpath);
        asm_result_free(&res);
        return 0;
    }
    const char *outdir = out ? out : ".";
    const char *const *d = (const char *const *)dirs.v;
    if (is_link) return link_file(files.v[0], outdir, d, dirs.n, &opt) ? 1 : 0;
    if (is_res) return rescomp_file(files.v[0], outdir, d, dirs.n) ? 1 : 0;
    return build_job(files.v[0], outdir, d, dirs.n, &opt) ? 1 : 0;
}
