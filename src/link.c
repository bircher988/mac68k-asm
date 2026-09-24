/* link.c - linker: .Link file -> CODE 0/CODE 1 (+ /Include resources) as a resource fork.
 *
 * The modules are assembled in link order into one code image; the result is
 * byte-identical to what the MDS linker produces for single-segment
 * programs. */
#include "link.h"
#include "resfork.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLOBALS_SIZE 0x100     /* reserved globals area below A5 (as MDS: "Size of Global Data Area is 256") */

/* buf_put that tolerates an empty source with a NULL pointer */
static void put_bytes(Buf *b, const void *p, size_t n) { if (n) buf_put(b, p, n); }

typedef struct {
    char *output;
    char type[5], creator[5];
    StrList modules, includes;
    int segments;
    char *start;
} LinkSpec;

/* Extract the quoted strings 'xxx' of a /Type argument. */
static int quoted(const char *arg, char *t1, char *t2) {
    int n = 0;
    const char *q = arg;
    while (n < 2 && (q = strchr(q, '\'')) != NULL) {
        const char *e = strchr(q + 1, '\'');
        if (!e) break;
        char *dst = n == 0 ? t1 : t2;
        size_t len = e - q - 1;
        memset(dst, 0, 5);
        memcpy(dst, q + 1, len < 4 ? len : 4);
        n++;
        q = e + 1;
    }
    return n;
}

static int parse_link(const char *path, LinkSpec *spec) {
    char *text = read_text_file(path);
    if (!text) { fprintf(stderr, "linker: cannot read %s\n", path); return -1; }
    memset(spec, 0, sizeof *spec);
    strcpy(spec->type, "TEMP"); strcpy(spec->creator, "????");
    char *s = text;
    while (s && *s) {
        char *nl = strchr(s, '\n');
        char *line = nl ? xstrndup(s, nl - s) : xstrdup(s);
        s = nl ? nl + 1 : NULL;
        char *sc = strchr(line, ';');
        if (sc) *sc = 0;
        str_trim(line);
        if (!*line || line[0] == '$') { free(line); continue; }
        if (line[0] == '/') {
            char *cmd = line + 1;
            char *arg = strchr(cmd, ' ');
            if (arg) { *arg++ = 0; str_trim(arg); } else arg = cmd + strlen(cmd);
            str_lower(cmd);
            if (!strcmp(cmd, "output")) { free(spec->output); spec->output = xstrdup(arg); }
            else if (!strcmp(cmd, "type")) {
                char t1[5], t2[5];
                int n = quoted(arg, t1, t2);
                if (n < 1) { fprintf(stderr, "linker: /Type expects 'TYPE' 'CREATOR': %s\n", arg); free(line); free(text); return -1; }
                strcpy(spec->type, t1);
                strcpy(spec->creator, n > 1 ? t2 : "????");
            }
            else if (!strcmp(cmd, "include")) strlist_add(&spec->includes, arg);
            else if (!strcmp(cmd, "segment")) spec->segments++;
            else if (!strcmp(cmd, "start")) { free(spec->start); spec->start = xstrdup(arg); }
            else if (!strcmp(cmd, "end")) { free(line); break; }
            else { fprintf(stderr, "linker: directive /%s not supported\n", cmd); free(line); free(text); return -1; }
        } else {
            strlist_add(&spec->modules, line);
        }
        free(line);
    }
    free(text);
    return 0;
}

static char *dirs_repr(const char *const *dirs, int ndirs) {
    Buf b = {0};
    buf_put8(&b, '[');
    for (int i = 0; i < ndirs; i++) {
        if (i) buf_put(&b, ", ", 2);
        buf_put8(&b, '\'');
        put_bytes(&b, dirs[i], strlen(dirs[i]));
        buf_put8(&b, '\'');
    }
    buf_put8(&b, ']');
    buf_put8(&b, 0);
    return (char *)b.d;
}

int link_file(const char *linkpath, const char *outdir, const char *const *dirs, int ndirs, const AsmOptions *opt) {
    LinkSpec spec;
    if (parse_link(linkpath, &spec)) return 1;
    if (!spec.output) {
        spec.output = path_strip_ext(path_basename(linkpath));
    }
    if (spec.segments || spec.start) {
        fprintf(stderr, "linker: /Segment and /Start are not implemented yet (multi-segment programs)\n");
        return 1;
    }
    if (make_dirs(outdir)) { fprintf(stderr, "linker: cannot create %s\n", outdir); return 1; }

    /* search path: outdir, the .Link file's directory, then dirs */
    char *ldir = path_dirname(linkpath);
    int nsdirs = 0;
    const char **sdirs = xmalloc((ndirs + 2) * sizeof *sdirs);
    sdirs[nsdirs++] = outdir;
    sdirs[nsdirs++] = ldir;
    for (int i = 0; i < ndirs; i++) sdirs[nsdirs++] = dirs[i];

    /* locate the modules */
    static const char *const asm_exts[] = { ".Asm", ".asm", ".ASM" };
    const char **srcs = xmalloc((spec.modules.n + 1) * sizeof *srcs);
    for (int i = 0; i < spec.modules.n; i++) {
        char *f = find_file_ci(spec.modules.v[i], sdirs, nsdirs, asm_exts, 3);
        if (!f) {
            char *r = dirs_repr(sdirs, nsdirs);
            fprintf(stderr, "linker: module %s.Asm not found in %s\n", spec.modules.v[i], r);
            free(r);
            return 1;
        }
        srcs[i] = f;
    }

    /* assembler options: include search path is the module search path plus
     * the caller's include directories (which end with the built-in inc dir) */
    AsmOptions ao = {0};
    if (opt) ao = *opt;
    int ninc = nsdirs + (opt ? opt->nincdirs : 0);
    const char **inc = xmalloc((ninc + 1) * sizeof *inc);
    int k = 0;
    for (int i = 0; i < nsdirs; i++) inc[k++] = sdirs[i];
    for (int i = 0; opt && i < opt->nincdirs; i++) inc[k++] = opt->incdirs[i];
    ao.incdirs = inc; ao.nincdirs = ninc;
    char *base = path_join(outdir, spec.output);
    char *lst;
    if (opt && opt->listing) lst = strchr(opt->listing, '/') ? xstrdup(opt->listing) : path_join(outdir, opt->listing);
    else lst = xsprintf("%s.lst", base);
    ao.listing = lst;

    char *errpath = xsprintf("%s.ERR", base);
    AsmResult ar = {0};
    int rc = asm_assemble(srcs, spec.modules.n, &ao, &ar);
    if (rc || ar.errors) {
        char *note = xsprintf("linker: %d assembler error(s) in %s; see the diagnostics printed by the assembler\n",
                              ar.errors ? ar.errors : 1, spec.output);
        write_binary_file(errpath, (const unsigned char *)note, strlen(note));
        fprintf(stderr, "linker: assembler errors, see %s\n", errpath);
        free(note);
        return 1;
    }
    remove(errpath);

    /* CODE 1: jump table offset 0, one entry, then the code (padded to even) */
    char *rawpath = xsprintf("%s.raw", base);
    if (write_binary_file(rawpath, ar.code, ar.len)) { fprintf(stderr, "linker: cannot write %s\n", rawpath); return 1; }
    Buf code1 = {0};
    buf_put16be(&code1, 0); buf_put16be(&code1, 1);
    put_bytes(&code1, ar.code, ar.len);
    if (ar.len & 1) buf_put8(&code1, 0);
    size_t codelen = code1.n - 4;
    unsigned globals_size = GLOBALS_SIZE + ar.ds_total;

    /* CODE 0: sizes, then the jump table: MOVE.W #1,-(SP) ; _LoadSeg */
    Buf code0 = {0};
    buf_put32be(&code0, 32 + 8);
    buf_put32be(&code0, globals_size);
    buf_put32be(&code0, 8);
    buf_put32be(&code0, 32);
    buf_put16be(&code0, 0); buf_put16be(&code0, 0x3f3c); buf_put16be(&code0, 1); buf_put16be(&code0, 0xa9f0);

    ResList res = {0};
    reslist_add(&res, "CODE", 0, code0.d, code0.n, 0x20, NULL);
    reslist_add(&res, "CODE", 1, code1.d, code1.n, 0x24, NULL);

    /* /Include resources: appended in reverse type order (linked list, as the MDS linker does) */
    static const char *const inc_exts[] = { "", ".bin" };
    for (int i = 0; i < spec.includes.n; i++) {
        const char *name = spec.includes.v[i];
        char *f = find_file_ci(name, sdirs, nsdirs, inc_exts, 2);
        if (!f) {
            char *r = dirs_repr(sdirs, nsdirs);
            fprintf(stderr, "linker: /Include %s not found (expected %s or %s.bin in %s)\n", name, name, name, r);
            free(r);
            return 1;
        }
        ResList extra = {0};
        if (load_resfile(f, &extra)) { fprintf(stderr, "linker: cannot read resource file %s\n", f); return 1; }
        /* types in order of first appearance */
        int norder = 0;
        const char **order = xmalloc((extra.n + 1) * sizeof *order);
        for (int j = 0; j < extra.n; j++) {
            int t;
            for (t = 0; t < norder; t++) if (!memcmp(order[t], extra.v[j].type, 4)) break;
            if (t == norder) order[norder++] = extra.v[j].type;
        }
        for (int t = norder - 1; t >= 0; t--)
            for (int j = 0; j < extra.n; j++)
                if (!memcmp(extra.v[j].type, order[t], 4))
                    reslist_add(&res, extra.v[j].type, extra.v[j].id, extra.v[j].data, extra.v[j].len, extra.v[j].attr, extra.v[j].name);
        free(order);
        reslist_free(&extra);
        free(f);
    }

    Buf fork = {0};
    write_fork(&res, &fork);
    char *rsrcpath = xsprintf("%s.rsrc", base);
    char *binpath = xsprintf("%s.bin", base);
    char *mappath = xsprintf("%s.MAP", base);
    if (write_binary_file(rsrcpath, fork.d, fork.n)) { fprintf(stderr, "linker: cannot write %s\n", rsrcpath); return 1; }
    Buf mb = {0};
    write_macbinary(spec.output, spec.type, spec.creator, NULL, 0, fork.d, fork.n, 0, &mb);
    if (write_binary_file(binpath, mb.d, mb.n)) { fprintf(stderr, "linker: cannot write %s\n", binpath); return 1; }
    char *map = xsprintf("\nSegment Sizes\n\n\tSize of Code Segment #1 is %zu ($%zX) bytes\n\tSize of Global Data Area is %u ($%X) bytes\n",
                         codelen, codelen, globals_size, globals_size);
    if (write_binary_file(mappath, (const unsigned char *)map, strlen(map))) { fprintf(stderr, "linker: cannot write %s\n", mappath); return 1; }
    printf("linker: %s (%s/%s) code %zu bytes, %d resources -> %s\n", spec.output, spec.type, spec.creator, codelen, res.n, binpath);
    fflush(stdout);

    buf_free(&mb); buf_free(&fork); buf_free(&code0); buf_free(&code1); reslist_free(&res);
    asm_result_free(&ar);
    free(map); free(rsrcpath); free(binpath); free(mappath); free(rawpath); free(errpath); free(lst); free(base);
    for (int i = 0; i < spec.modules.n; i++) free((char *)srcs[i]);
    free(srcs); free(inc); free(sdirs); free(ldir);
    strlist_free(&spec.modules); strlist_free(&spec.includes); free(spec.output); free(spec.start);
    return 0;
}
