/* util.c - small helpers shared by all tools. */
#define _GNU_SOURCE
#include "util.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

const char *prog_name = "mac68k-asm";

void *xmalloc(size_t n) { void *p = malloc(n ? n : 1); if (!p) die("out of memory"); return p; }
void *xcalloc(size_t n, size_t sz) { void *p = calloc(n ? n : 1, sz ? sz : 1); if (!p) die("out of memory"); return p; }
void *xrealloc(void *p, size_t n) { p = realloc(p, n ? n : 1); if (!p) die("out of memory"); return p; }
char *xstrdup(const char *s) { size_t n = strlen(s) + 1; char *d = xmalloc(n); memcpy(d, s, n); return d; }
char *xstrndup(const char *s, size_t n) { char *d = xmalloc(n + 1); memcpy(d, s, n); d[n] = 0; return d; }

char *xsprintf(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    char *s = NULL;
    if (vasprintf(&s, fmt, ap) < 0) die("out of memory");
    va_end(ap);
    return s;
}

void die(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    fprintf(stderr, "%s: ", prog_name); vfprintf(stderr, fmt, ap); fputc('\n', stderr);
    va_end(ap);
    exit(1);
}

void warnf(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    fprintf(stderr, "%s: warning: ", prog_name); vfprintf(stderr, fmt, ap); fputc('\n', stderr);
    va_end(ap);
}

int str_ieq(const char *a, const char *b) { return strcasecmp(a, b) == 0; }
int str_ieq_n(const char *a, const char *b, size_t n) { return strncasecmp(a, b, n) == 0; }
int str_istarts(const char *s, const char *prefix) { return strncasecmp(s, prefix, strlen(prefix)) == 0; }
char *str_upper(char *s) { for (char *p = s; *p; p++) *p = (char)toupper((unsigned char)*p); return s; }
char *str_lower(char *s) { for (char *p = s; *p; p++) *p = (char)tolower((unsigned char)*p); return s; }

char *str_trim(char *s) {
    char *b = s;
    while (*b == ' ' || *b == '\t') b++;
    size_t n = strlen(b);
    while (n && (b[n-1] == ' ' || b[n-1] == '\t' || b[n-1] == '\n' || b[n-1] == '\r')) n--;
    memmove(s, b, n); s[n] = 0;
    return s;
}

int is_blank_line(const char *s) {
    for (; *s; s++) if (*s != ' ' && *s != '\t' && *s != '\r' && *s != '\n') return 0;
    return 1;
}

void strlist_add(StrList *l, const char *s) {
    if (l->n == l->cap) { l->cap = l->cap ? l->cap * 2 : 8; l->v = xrealloc(l->v, l->cap * sizeof *l->v); }
    l->v[l->n++] = xstrdup(s);
}
void strlist_free(StrList *l) { for (int i = 0; i < l->n; i++) free(l->v[i]); free(l->v); l->v = NULL; l->n = l->cap = 0; }

void buf_reserve(Buf *b, size_t extra) {
    if (b->n + extra > b->cap) {
        size_t c = b->cap ? b->cap : 256;
        while (c < b->n + extra) c *= 2;
        b->d = xrealloc(b->d, c); b->cap = c;
    }
}
void buf_put8(Buf *b, unsigned v) { buf_reserve(b, 1); b->d[b->n++] = (unsigned char)v; }
void buf_put16be(Buf *b, unsigned v) { buf_put8(b, v >> 8); buf_put8(b, v); }
void buf_put32be(Buf *b, uint32_t v) { buf_put16be(b, v >> 16); buf_put16be(b, v & 0xffff); }
void buf_put(Buf *b, const void *p, size_t n) { if (!n) return; buf_reserve(b, n); memcpy(b->d + b->n, p, n); b->n += n; }
void buf_pad(Buf *b, size_t align) { while (b->n % align) buf_put8(b, 0); }
void buf_free(Buf *b) { free(b->d); b->d = NULL; b->n = b->cap = 0; }
unsigned get16be(const unsigned char *p) { return (p[0] << 8) | p[1]; }
uint32_t get32be(const unsigned char *p) { return ((uint32_t)get16be(p) << 16) | get16be(p + 2); }
void put16be(unsigned char *p, unsigned v) { p[0] = (unsigned char)(v >> 8); p[1] = (unsigned char)v; }
void put32be(unsigned char *p, uint32_t v) { put16be(p, v >> 16); put16be(p + 2, v & 0xffff); }

char *read_text_file(const char *path) {
    unsigned char *d; size_t n;
    if (read_binary_file(path, &d, &n)) return NULL;
    char *out = xmalloc(n + 1); size_t o = 0;
    for (size_t i = 0; i < n; i++) {
        if (d[i] == '\r') { out[o++] = '\n'; if (i + 1 < n && d[i+1] == '\n') i++; }
        else out[o++] = (char)d[i];
    }
    out[o] = 0;
    free(d);
    return out;
}

int read_binary_file(const char *path, unsigned char **data, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    Buf b = {0};
    unsigned char tmp[65536]; size_t r;
    while ((r = fread(tmp, 1, sizeof tmp, f)) > 0) buf_put(&b, tmp, r);
    fclose(f);
    *data = b.d ? b.d : xmalloc(1); *len = b.n;
    return 0;
}

int write_binary_file(const char *path, const unsigned char *data, size_t len) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    int ok = fwrite(data, 1, len, f) == len;
    if (fclose(f) != 0) ok = 0;
    return ok ? 0 : -1;
}

int file_exists(const char *path) { struct stat st; return stat(path, &st) == 0 && S_ISREG(st.st_mode); }
int dir_exists(const char *path) { struct stat st; return stat(path, &st) == 0 && S_ISDIR(st.st_mode); }

int make_dirs(const char *path) {
    char *p = xstrdup(path);
    for (char *s = p + 1; *s; s++) {
        if (*s == '/') { *s = 0; if (mkdir(p, 0777) && errno != EEXIST) { free(p); return -1; } *s = '/'; }
    }
    int r = (mkdir(p, 0777) && errno != EEXIST) ? -1 : 0;
    free(p);
    return r;
}

char *path_join(const char *dir, const char *name) {
    if (!dir || !*dir || !strcmp(dir, ".")) return xstrdup(name);
    size_t n = strlen(dir);
    return xsprintf(dir[n-1] == '/' ? "%s%s" : "%s/%s", dir, name);
}

char *path_dirname(const char *path) {
    const char *s = strrchr(path, '/');
    if (!s) return xstrdup(".");
    if (s == path) return xstrdup("/");
    return xstrndup(path, s - path);
}

const char *path_basename(const char *path) { const char *s = strrchr(path, '/'); return s ? s + 1 : path; }

char *path_strip_ext(const char *name) {
    const char *dot = strrchr(name, '.');
    const char *slash = strrchr(name, '/');
    if (dot && (!slash || dot > slash)) return xstrndup(name, dot - name);
    return xstrdup(name);
}

char *exe_dir(void) {
    char buf[4096];
#ifdef __APPLE__
    uint32_t n = sizeof buf;
    if (_NSGetExecutablePath(buf, &n) != 0) return NULL;
#else
    ssize_t n = readlink("/proc/self/exe", buf, sizeof buf - 1);
    if (n <= 0) return NULL;
    buf[n] = 0;
#endif
    return path_dirname(buf);
}

char *find_file_ci(const char *name, const char *const *dirs, int ndirs, const char *const *exts, int nexts) {
    static const char *none[] = { "" };
    if (!exts || nexts == 0) { exts = none; nexts = 1; }
    for (int i = 0; i < ndirs; i++) {
        DIR *d = opendir(dirs[i] && *dirs[i] ? dirs[i] : ".");
        if (!d) continue;
        struct dirent *e;
        char *found = NULL;
        while (!found && (e = readdir(d))) {
            for (int k = 0; k < nexts; k++) {
                char *cand = xsprintf("%s%s", name, exts[k]);
                if (str_ieq(e->d_name, cand)) { found = path_join(dirs[i], e->d_name); free(cand); break; }
                free(cand);
            }
        }
        closedir(d);
        if (found) return found;
    }
    return NULL;
}
