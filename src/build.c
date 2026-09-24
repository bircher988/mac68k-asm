/* build.c - runs a .Job file: ASM lines are skipped (the modules are assembled
 * during LINK), LINK runs the linker, RMAKER the resource compiler. Afterwards
 * the first APPL MacBinary in the output directory is reported as DONE. */
#include "build.h"
#include "link.h"
#include "rescomp.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Split a job line into columns separated by tabs or runs of two or more
 * spaces (a single space may be part of a file name). */
static char **split_columns(const char *line, int *count) {
    int n = 0, cap = 8;
    char **v = xmalloc(cap * sizeof *v);
    const char *q = line;
    for (;;) {
        while (*q == '\t' || *q == ' ') q++;
        if (!*q) break;
        const char *e = q;
        while (*e && *e != '\t' && !(*e == ' ' && (e[1] == ' ' || e[1] == '\t' || !e[1]))) e++;
        if (n + 2 >= cap) { cap *= 2; v = xrealloc(v, cap * sizeof *v); }
        v[n++] = xstrndup(q, e - q);
        q = e;
    }
    v[n] = NULL; *count = n;
    return v;
}

static int cmp_names(const void *a, const void *b) { return strcoll(*(char *const *)a, *(char *const *)b); }

/* Type of a MacBinary file (header offset 65), or 0 if unreadable. */
static int macbinary_type(const char *path, char type[5]) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    unsigned char h[128];
    size_t n = fread(h, 1, sizeof h, f);
    fclose(f);
    if (n < 128) return 0;
    memcpy(type, h + 65, 4); type[4] = 0;
    return 1;
}

int build_job(const char *jobpath, const char *outdir, const char *const *dirs, int ndirs, const AsmOptions *opt) {
    char *text = read_text_file(jobpath);
    if (!text) { fprintf(stderr, "build: cannot read %s\n", jobpath); return 1; }
    char *jdir = path_dirname(jobpath);
    const char **sdirs = xmalloc((ndirs + 1) * sizeof *sdirs);
    int nsdirs = 0;
    sdirs[nsdirs++] = jdir;
    for (int i = 0; i < ndirs; i++) sdirs[nsdirs++] = dirs[i];

    char *s = text;
    while (s && *s) {
        char *nl = strchr(s, '\n');
        char *line = nl ? xstrndup(s, nl - s) : xstrdup(s);
        s = nl ? nl + 1 : NULL;
        int ncol; char **col = split_columns(line, &ncol);
        if (ncol == 0) { free(col); free(line); continue; }
        char *cmd = col[0];
        /* tolerate a single space between the command and the file name */
        char *sp = strchr(cmd, ' ');
        if (sp && ncol == 1) { *sp = 0; col[1] = xstrdup(sp + 1); col[2] = NULL; ncol = 2; }
        str_upper(cmd);
        const char *file = ncol > 1 ? col[1] : "";
        int rc = 0;
        if (!strcmp(cmd, "ASM")) {
            printf("== ASM %s (assembled during LINK)\n", file);
        } else if (!strcmp(cmd, "LINK") || !strcmp(cmd, "RMAKER")) {
            printf("== %s %s\n", cmd, file);
            fflush(stdout);
            char *src = find_file_ci(file, sdirs, nsdirs, NULL, 0);
            if (!src) { fprintf(stderr, "build: %s not found in %s\n", file, jdir); rc = 1; }
            else if (!strcmp(cmd, "LINK")) rc = link_file(src, outdir, dirs, ndirs, opt);
            else rc = rescomp_file(src, outdir, dirs, ndirs);
            free(src);
        } else {
            printf("unknown job command: %s\n", cmd);
        }
        fflush(stdout);
        for (int i = 0; i < ncol; i++) free(col[i]);
        free(col); free(line);
        if (rc) { free(text); free(jdir); free(sdirs); return rc; }
    }
    free(text);

    /* the finished application: the first APPL file in the output folder */
    StrList bins = {0};
    DIR *d = opendir(outdir && *outdir ? outdir : ".");
    if (d) {
        struct dirent *e;
        while ((e = readdir(d)) != NULL) {
            size_t n = strlen(e->d_name);
            if (n > 4 && !strcmp(e->d_name + n - 4, ".bin")) strlist_add(&bins, e->d_name);
        }
        closedir(d);
    }
    if (bins.n) qsort(bins.v, bins.n, sizeof *bins.v, cmp_names);
    char *app = NULL;
    for (int i = 0; i < bins.n && !app; i++) {
        char *p = path_join(outdir, bins.v[i]);
        char type[5];
        if (macbinary_type(p, type) && !strcmp(type, "APPL")) app = p; else free(p);
    }
    if (app) printf("DONE: %s\n", app);
    else printf("note: no APPL file produced (only .code/.rsrc).\n");
    fflush(stdout);
    free(app); strlist_free(&bins); free(jdir); free(sdirs);
    return 0;
}
