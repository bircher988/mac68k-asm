/* rescomp.c - resource compiler: .R text -> resource fork (+ MacBinary).
 *
 * The input format is that of RMaker, the MDS resource compiler: the first non-blank line names the
 * output file, an optional second line gives type+creator (old form), then
 * "INCLUDE file" and "Type XXXX [= FMT]" sections with ",id [(attr)]" resources.
 * Lines starting with '*' are comments. */
#include "rescomp.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char **lines; int nlines;
    int pos;                     /* index of the next line to read */
    const char *const *dirs; int ndirs;
    int cur_id;                  /* MENU needs the ID inside its data block */
    const char *path;
} RParser;

static int is_ws(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f'; }

/* buf_put that tolerates an empty source with a NULL pointer */
static void put_bytes(Buf *b, const void *p, size_t n) { if (n) buf_put(b, p, n); }

static int blank(const char *s) { for (; *s; s++) if (!is_ws((unsigned char)*s)) return 0; return 1; }

/* strip leading and trailing whitespace, in place */
static char *strip(char *s) {
    char *b = s;
    while (is_ws((unsigned char)*b)) b++;
    size_t n = strlen(b);
    while (n && is_ws((unsigned char)b[n-1])) n--;
    memmove(s, b, n); s[n] = 0;
    return s;
}

#ifdef __GNUC__
static void fail(RParser *p, const char *fmt, ...) __attribute__((noreturn, format(printf, 2, 3)));
#endif
static void fail(RParser *p, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    fprintf(stderr, "rescomp: error near line %d: ", p->pos);
    vfprintf(stderr, fmt, ap); fputc('\n', stderr);
    va_end(ap);
    exit(1);
}

/* ---- line access: peek skips comment lines, next returns "" at EOF ---- */
static const char *peek(RParser *p) {
    while (p->pos < p->nlines && p->lines[p->pos][0] == '*') p->pos++;
    return p->pos < p->nlines ? p->lines[p->pos] : NULL;
}
static char *next_line(RParser *p) {
    const char *l = peek(p); p->pos++;
    return xstrdup(l ? l : "");
}
static char *next_nonblank(RParser *p) {
    for (;;) {
        const char *l = peek(p);
        if (!l) return NULL;
        p->pos++;
        if (!blank(l)) return xstrdup(l);
    }
}

/* ---- value parsing ---- */
static int hexval(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* \hh = hex character code (RMaker convention, e.g. \14 = Apple symbol); in place */
static char *unescape(char *s) {
    char *o = s;
    for (char *r = s; *r; ) {
        if (r[0] == '\\' && hexval((unsigned char)r[1]) >= 0 && hexval((unsigned char)r[2]) >= 0) {
            *o++ = (char)(hexval((unsigned char)r[1]) * 16 + hexval((unsigned char)r[2]));
            r += 3;
        } else *o++ = *r++;
    }
    *o = 0;
    return s;
}

/* Python int(): optional sign, decimal digits, surrounding whitespace allowed */
static long parse_int(RParser *p, const char *s) {
    const char *b = s;
    while (is_ws((unsigned char)*b)) b++;
    const char *q = b;
    if (*q == '+' || *q == '-') q++;
    if (!isdigit((unsigned char)*q)) fail(p, "invalid integer: '%s'", s);
    while (isdigit((unsigned char)*q)) q++;
    while (is_ws((unsigned char)*q)) q++;
    if (*q) fail(p, "invalid integer: '%s'", s);
    return strtol(b, NULL, 10);
}

static long parse_int_base(RParser *p, const char *s, int base) {
    char *end;
    while (is_ws((unsigned char)*s)) s++;
    long v = strtol(s, &end, base);
    if (end == s) fail(p, "invalid number: '%s'", s);
    while (is_ws((unsigned char)*end)) end++;
    if (*end) fail(p, "invalid number: '%s'", s);
    return v;
}

static void put_i16(RParser *p, Buf *b, long v) {
    if (v < -32768 || v > 32767) fail(p, "value out of range for a word: %ld", v);
    buf_put16be(b, (unsigned)v & 0xffff);
}
static void put_u16(RParser *p, Buf *b, long v) {
    if (v < 0 || v > 65535) fail(p, "value out of range for an unsigned word: %ld", v);
    buf_put16be(b, (unsigned)v);
}
static void put_i32(RParser *p, Buf *b, long v) {
    if (v < -2147483648L || v > 2147483647L) fail(p, "value out of range for a long: %ld", v);
    buf_put32be(b, (uint32_t)v);
}

/* split on whitespace runs; returns a malloc'd NULL-terminated vector */
static char **split_ws(const char *s, int *count) {
    int n = 0, cap = 8;
    char **v = xmalloc(cap * sizeof *v);
    for (const char *q = s; ; ) {
        while (is_ws((unsigned char)*q)) q++;
        if (!*q) break;
        const char *e = q;
        while (*e && !is_ws((unsigned char)*e)) e++;
        if (n + 1 >= cap) { cap *= 2; v = xrealloc(v, cap * sizeof *v); }
        v[n++] = xstrndup(q, e - q);
        q = e;
    }
    v[n] = NULL; *count = n;
    return v;
}
static void free_words(char **v) { for (char **w = v; *w; w++) free(*w); free(v); }

/* Pascal string: length byte + text */
static void pstr_checked(RParser *p, Buf *b, const char *s) {
    size_t n = strlen(s);
    if (n > 255) fail(p, "string longer than 255 bytes");
    buf_put8(b, (unsigned)n);
    put_bytes(b, s, n);
}

static void rect(RParser *p, Buf *b, const char *line) {
    int n; char **w = split_ws(line, &n);
    if (n != 4) fail(p, "rectangle expected (top left bottom right): '%s'", line);
    for (int i = 0; i < 4; i++) put_i16(p, b, parse_int(p, w[i]));
    free_words(w);
}

static int has_word_ci(const char *line, const char *word) {
    int n, found = 0; char **w = split_ws(line, &n);
    for (int i = 0; i < n; i++) if (str_ieq(w[i], word)) { found = 1; break; }
    free_words(w);
    return found;
}
static void flags_word(Buf *b, const char *line, const char *yes) {
    buf_put16be(b, has_word_ci(line, yes) ? 0xffff : 0);
}

/* ---- resource types ---- */
static void p_wind(RParser *p, Buf *b) {
    char *title = unescape(next_line(p));
    char *r = next_line(p), *fl = next_line(p), *proc = next_line(p), *refcon = next_line(p);
    rect(p, b, r);
    put_i16(p, b, parse_int(p, proc));
    flags_word(b, fl, "Visible");
    flags_word(b, fl, "GoAway");
    put_i32(p, b, parse_int(p, refcon));
    pstr_checked(p, b, title);
    free(title); free(r); free(fl); free(proc); free(refcon);
}

static void p_dlog(RParser *p, Buf *b) {
    char *title = unescape(next_line(p));
    char *r = next_line(p), *fl = next_line(p), *proc = next_line(p), *refcon = next_line(p), *items = next_line(p);
    rect(p, b, r);
    put_i16(p, b, parse_int(p, proc));
    flags_word(b, fl, "Visible");
    flags_word(b, fl, "GoAway");
    put_i32(p, b, parse_int(p, refcon));
    put_i16(p, b, parse_int(p, items));
    pstr_checked(p, b, title);
    free(title); free(r); free(fl); free(proc); free(refcon); free(items);
}

static void p_alrt(RParser *p, Buf *b) {
    char *r = next_nonblank(p);
    if (!r) fail(p, "ALRT: rectangle expected");
    char *items = next_line(p), *st = strip(next_line(p));
    rect(p, b, r);
    put_i16(p, b, parse_int(p, items));
    int binary = strlen(st) == 16;
    for (const char *q = st; binary && *q; q++) if (*q != '0' && *q != '1') binary = 0;
    put_u16(p, b, parse_int_base(p, st, binary ? 2 : 16));
    free(r); free(items); free(st);
}

static const struct { const char *name; int kind; } ITEM_TYPES[] = {
    { "useritem", 0 }, { "button", 4 }, { "checkbox", 5 }, { "radiobutton", 6 }, { "control", 7 },
    { "statictext", 8 }, { "edittext", 16 }, { "icon", 32 }, { "picture", 64 },
};

static void p_ditl(RParser *p, Buf *b) {
    char *nl = next_line(p);
    long n = parse_int(p, nl); free(nl);
    put_i16(p, b, n - 1);
    for (long i = 0; i < n; i++) {
        char *tl = next_nonblank(p);
        if (!tl) fail(p, "DITL: item expected");
        int nw; char **w = split_ws(tl, &nw);
        if (nw == 0) fail(p, "DITL: item type expected");
        int kind = -1;
        for (size_t k = 0; k < sizeof ITEM_TYPES / sizeof *ITEM_TYPES; k++)
            if (str_ieq(w[0], ITEM_TYPES[k].name)) { kind = ITEM_TYPES[k].kind; break; }
        if (kind < 0) { fprintf(stderr, "rescomp: unknown DITL item type %s\n", w[0]); exit(1); }
        for (int k = 1; k < nw; k++) if (str_ieq(w[k], "disabled")) kind |= 128;
        free_words(w); free(tl);
        Buf item = {0};
        buf_put32be(&item, 0);
        char *r = next_line(p);
        rect(p, &item, r); free(r);
        Buf body = {0};
        int base = kind & 127;
        if (base == 0) {
            /* useritem: no body line */
        } else if (base == 7 || base == 32 || base == 64) {
            char *v = next_line(p);
            put_i16(p, &body, parse_int(p, v)); free(v);
        } else {
            char *s = unescape(next_line(p));
            put_bytes(&body, s, strlen(s)); free(s);
        }
        if (body.n > 255) fail(p, "DITL item text longer than 255 bytes");
        buf_put8(&item, (unsigned)kind);
        buf_put8(&item, (unsigned)body.n);
        put_bytes(&item, body.d, body.n);
        if (item.n & 1) buf_put8(&item, 0);
        put_bytes(b, item.d, item.n);
        buf_free(&item); buf_free(&body);
    }
}

/* remove s[a..b) in place */
static void cut(char *s, size_t a, size_t b) { memmove(s + a, s + b, strlen(s + b) + 1); }

static void menu_data(RParser *p, Buf *b, const char *title, char **items, int nitems) {
    uint32_t enable = 0xffffffff;
    Buf body = {0};
    for (int i = 1; i <= nitems; i++) {
        char *it = items[i - 1];
        if (it[0] == '(') { cut(it, 0, 1); if (i < 32) enable &= ~(1u << i); }
        long icon = 0, cmd = 0, mark = 0, style = 0;
        char *m;
        /* ^n icon number */
        for (m = it; (m = strchr(m, '^')) != NULL; m++) {
            if (isdigit((unsigned char)m[1])) {
                char *e = m + 1;
                while (isdigit((unsigned char)*e)) e++;
                icon = strtol(m + 1, NULL, 10);
                cut(it, m - it, e - it);
                break;
            }
        }
        /* !c marking character */
        if ((m = strchr(it, '!')) != NULL && m[1]) { mark = (unsigned char)m[1]; cut(it, m - it, m - it + 2); }
        /* /c command key */
        if ((m = strchr(it, '/')) != NULL && m[1]) { cmd = (unsigned char)m[1]; cut(it, m - it, m - it + 2); }
        /* <BIUOS style */
        for (m = it; (m = strchr(m, '<')) != NULL; m++) {
            if (strchr("BIUOS", m[1]) && m[1]) {
                char *e = m + 1;
                while (*e && strchr("BIUOS", *e)) {
                    switch (*e) { case 'B': style |= 1; break; case 'I': style |= 2; break; case 'U': style |= 4; break;
                                  case 'O': style |= 8; break; case 'S': style |= 16; break; }
                    e++;
                }
                cut(it, m - it, e - it);
                break;
            }
        }
        if (icon > 255) fail(p, "menu icon number out of range: %ld", icon);
        pstr_checked(p, &body, unescape(it));
        buf_put8(&body, (unsigned)icon); buf_put8(&body, (unsigned)cmd);
        buf_put8(&body, (unsigned)mark); buf_put8(&body, (unsigned)style);
    }
    put_i16(p, b, p->cur_id);
    buf_put16be(b, 0); buf_put16be(b, 0); buf_put16be(b, 0); buf_put16be(b, 0);
    buf_put32be(b, enable);
    pstr_checked(p, b, title);
    put_bytes(b, body.d, body.n);
    buf_put8(b, 0);
    buf_free(&body);
}

static void p_menu(RParser *p, Buf *b) {
    char *title = unescape(next_line(p));
    StrList items = {0};
    for (;;) {
        const char *l = peek(p);
        if (!l || blank(l)) { p->pos++; break; }
        p->pos++;
        char *s = strip(xstrdup(l));
        strlist_add(&items, s); free(s);
    }
    menu_data(p, b, title, items.v, items.n);
    strlist_free(&items); free(title);
}

static void put_type4(Buf *b, const char *s) {
    unsigned char t[4]; memset(t, ' ', 4);
    size_t n = strlen(s); if (n > 4) n = 4;
    memcpy(t, s, n);
    buf_put(b, t, 4);
}

/* "-?\d+\s+-?\d+" with optional surrounding whitespace */
static int is_int_pair(const char *l) {
    int n, ok = 1; char **w = split_ws(l, &n);
    if (n != 2) ok = 0;
    for (int i = 0; ok && i < n; i++) {
        const char *q = w[i]; if (*q == '-') q++;
        if (!isdigit((unsigned char)*q)) ok = 0;
        while (isdigit((unsigned char)*q)) q++;
        if (*q) ok = 0;
    }
    free_words(w);
    return ok;
}

static void p_bndl(RParser *p, Buf *b) {
    char *l = next_line(p);
    int n; char **w = split_ws(l, &n);
    if (n != 2) fail(p, "BNDL: signature and ID expected: '%s'", l);
    put_type4(b, w[0]);
    put_i16(p, b, parse_int(p, w[1]));
    free_words(w); free(l);
    Buf types = {0}; int ntypes = 0;
    for (;;) {
        const char *pk = peek(p);
        if (!pk || blank(pk)) { p->pos++; break; }
        char *t = strip(next_line(p));
        put_type4(&types, t); free(t);
        Buf pairs = {0}; int npairs = 0;
        for (;;) {
            pk = peek(p);
            if (!pk || blank(pk) || !is_int_pair(pk)) break;
            char *pl = next_line(p);
            int pn; char **pw = split_ws(pl, &pn);
            put_i16(p, &pairs, parse_int(p, pw[0]));
            put_i16(p, &pairs, parse_int(p, pw[1]));
            free_words(pw); free(pl);
            npairs++;
        }
        put_i16(p, &types, npairs - 1);
        put_bytes(&types, pairs.d, pairs.n);
        buf_free(&pairs);
        ntypes++;
    }
    put_i16(p, b, ntypes - 1);
    put_bytes(b, types.d, types.n);
    buf_free(&types);
}

static void p_gnrl(RParser *p, Buf *b) {
    int mode = 0;
    for (;;) {
        const char *pk = peek(p);
        if (!pk || blank(pk)) { p->pos++; break; }
        char *l = strip(next_line(p));
        if (l[0] == '.' && isalpha((unsigned char)l[1]) && !l[2]) { mode = toupper((unsigned char)l[1]); free(l); continue; }
        int n; char **w;
        switch (mode) {
        case 'H': {
            int hi = -1;
            for (const char *q = l; *q; q++) {
                if (is_ws((unsigned char)*q)) continue;
                int v = hexval((unsigned char)*q);
                if (v < 0) fail(p, "invalid hex data: '%s'", l);
                if (hi < 0) hi = v; else { buf_put8(b, (unsigned)(hi * 16 + v)); hi = -1; }
            }
            if (hi >= 0) fail(p, "odd number of hex digits: '%s'", l);
            break;
        }
        case 'I':
            w = split_ws(l, &n);
            for (int i = 0; i < n; i++) put_i16(p, b, parse_int(p, w[i]));
            free_words(w); break;
        case 'L':
            w = split_ws(l, &n);
            for (int i = 0; i < n; i++) put_i32(p, b, parse_int(p, w[i]));
            free_words(w); break;
        case 'P': pstr_checked(p, b, unescape(l)); break;
        case 'S': unescape(l); put_bytes(b, l, strlen(l)); break;
        case 'B':
            w = split_ws(l, &n);
            for (int i = 0; i < n; i++) {
                long v = parse_int(p, w[i]);
                if (v < 0 || v > 255) fail(p, "byte value out of range: %ld", v);
                buf_put8(b, (unsigned)v);
            }
            free_words(w); break;
        default:
            if (mode) fprintf(stderr, "rescomp: unknown GNRL format .%c\n", mode);
            else fprintf(stderr, "rescomp: unknown GNRL format .None\n");
            exit(1);
        }
        free(l);
    }
}

typedef void (*ResFn)(RParser *, Buf *);
static ResFn lookup_fmt(const char *fmt) {
    if (str_ieq(fmt, "WIND")) return p_wind;
    if (str_ieq(fmt, "DLOG")) return p_dlog;
    if (str_ieq(fmt, "ALRT")) return p_alrt;
    if (str_ieq(fmt, "DITL")) return p_ditl;
    if (str_ieq(fmt, "MENU")) return p_menu;
    if (str_ieq(fmt, "BNDL")) return p_bndl;
    if (str_ieq(fmt, "GNRL")) return p_gnrl;
    if (str_ieq(fmt, "STR")) return p_gnrl;
    return NULL;
}

/* INCLUDE search: each directory in turn, trying name, name.bin, name.rsrc, name.rsrc.bin */
static char *find_include(const char *name, const char *const *dirs, int ndirs) {
    static const char *const exts[] = { "", ".bin", ".rsrc", ".rsrc.bin" };
    for (int i = 0; i < ndirs; i++)
        for (int k = 0; k < 4; k++) {
            char *f = find_file_ci(name, dirs + i, 1, exts + k, 1);
            if (f) return f;
        }
    return NULL;
}

/* "^\s*word\b" (case-insensitive): returns the text after the word, or NULL */
static const char *match_keyword(const char *l, const char *word) {
    while (is_ws((unsigned char)*l)) l++;
    size_t n = strlen(word);
    if (!str_ieq_n(l, word, n)) return NULL;
    if (isalnum((unsigned char)l[n]) || l[n] == '_') return NULL;
    return l + n;
}

/* "<name> , <id> [(attr)]": returns 1 and fills the fields when the line matches */
static int match_resource_line(const char *l, char **name, long *rid, long *attr) {
    const char *comma = strrchr(l, ',');
    if (!comma) return 0;
    const char *q = comma + 1;
    while (is_ws((unsigned char)*q)) q++;
    const char *ib = q;
    if (*q == '-') q++;
    if (!isdigit((unsigned char)*q)) return 0;
    while (isdigit((unsigned char)*q)) q++;
    const char *ie = q;
    while (is_ws((unsigned char)*q)) q++;
    long a = 0;
    if (*q == '(') {
        q++;
        if (!isdigit((unsigned char)*q)) return 0;
        char *end;
        a = strtol(q, &end, 10);
        q = end;
        if (*q != ')') return 0;
        q++;
        while (is_ws((unsigned char)*q)) q++;
    }
    if (*q) return 0;
    char *nm = xstrndup(l, comma - l);
    strip(nm);
    *name = nm;
    *rid = strtol(ib, NULL, 10);             /* stops at ie */
    *attr = a;
    (void)ie;
    return 1;
}

int rescomp_file(const char *rpath, const char *outdir, const char *const *dirs, int ndirs) {
    char *text = read_text_file(rpath);
    if (!text) { fprintf(stderr, "rescomp: cannot read %s\n", rpath); return 1; }
    RParser p = {0};
    p.path = rpath;
    /* split into lines; a trailing newline yields a final empty line, as in Python */
    {
        int cap = 64; p.lines = xmalloc(cap * sizeof *p.lines);
        char *s = text;
        for (;;) {
            char *nl = strchr(s, '\n');
            if (p.nlines == cap) { cap *= 2; p.lines = xrealloc(p.lines, cap * sizeof *p.lines); }
            p.lines[p.nlines++] = nl ? xstrndup(s, nl - s) : xstrdup(s);
            if (!nl) break;
            s = nl + 1;
        }
    }
    /* search path for INCLUDE: outdir, the .R file's directory, then dirs */
    char *rdir = path_dirname(rpath);
    const char **sdirs = xmalloc((ndirs + 2) * sizeof *sdirs);
    int nsdirs = 0;
    sdirs[nsdirs++] = outdir;
    sdirs[nsdirs++] = rdir;
    for (int i = 0; i < ndirs; i++) sdirs[nsdirs++] = dirs[i];
    p.dirs = sdirs; p.ndirs = nsdirs;

    char *outname = next_nonblank(&p);
    if (!outname) { fprintf(stderr, "rescomp: %s: output name expected\n", rpath); return 1; }
    strip(outname);
    char ftype[5] = "rsrc", creator[5] = "RSED";
    char *l = next_nonblank(&p);
    if (l && !match_keyword(l, "type") && !match_keyword(l, "include")) {
        strip(l);
        size_t n = strlen(l);
        memset(ftype, 0, sizeof ftype); memset(creator, 0, sizeof creator);
        memcpy(ftype, l, n < 4 ? n : 4);
        if (n > 4) memcpy(creator, l + 4, n - 4 < 4 ? n - 4 : 4);
    } else {
        p.pos--;
    }
    free(l);

    ResList res = {0};
    char cur[5] = ""; int have_cur = 0;
    char *fmt = NULL;
    for (;;) {
        l = next_nonblank(&p);
        if (!l) break;
        const char *rest;
        if ((rest = match_keyword(l, "include")) != NULL && is_ws((unsigned char)*rest)) {
            while (is_ws((unsigned char)*rest)) rest++;
            const char *e = rest; while (*e && !is_ws((unsigned char)*e)) e++;
            if (e > rest) {
                char *name = xstrndup(rest, e - rest);
                char *f = find_include(name, p.dirs, p.ndirs);
                if (!f) { fprintf(stderr, "rescomp: INCLUDE %s not found\n", name); return 1; }
                if (load_resfile(f, &res)) { fprintf(stderr, "rescomp: cannot read resource file %s\n", f); return 1; }
                free(name); free(f); free(l);
                continue;
            }
        }
        if ((rest = match_keyword(l, "type")) != NULL && is_ws((unsigned char)*rest)) {
            while (is_ws((unsigned char)*rest)) rest++;
            const char *e = rest; while (*e && !is_ws((unsigned char)*e) && *e != '=') e++;
            size_t tn = e - rest;
            if (tn >= 1 && tn <= 4) {
                const char *q = e;
                while (is_ws((unsigned char)*q)) q++;
                char *alias = NULL; int ok = 1;
                if (*q == '=') {
                    q++;
                    while (is_ws((unsigned char)*q)) q++;
                    const char *ae = q; while (*ae && !is_ws((unsigned char)*ae)) ae++;
                    if (ae == q) ok = 0; else alias = xstrndup(q, ae - q);
                    q = ae;
                    while (is_ws((unsigned char)*q)) q++;
                }
                if (ok && !*q) {
                    memset(cur, ' ', 4); cur[4] = 0; memcpy(cur, rest, tn);
                    have_cur = 1;
                    free(fmt);
                    fmt = alias ? alias : xstrndup(rest, tn);
                    str_upper(fmt);
                    free(l);
                    continue;
                }
                free(alias);
            }
        }
        char *name; long rid, attr;
        if (have_cur && match_resource_line(l, &name, &rid, &attr)) {
            p.cur_id = (int)rid;
            ResFn fn = lookup_fmt(fmt);
            if (!fn) { fprintf(stderr, "rescomp: resource type %s not supported\n", fmt); return 1; }
            Buf data = {0};
            fn(&p, &data);
            reslist_add(&res, cur, (int)rid, data.d, data.n, (unsigned)attr, *name ? name : NULL);
            buf_free(&data); free(name); free(l);
            continue;
        }
        fprintf(stderr, "rescomp: cannot parse line %d: '%s'\n", p.pos, l);
        return 1;
    }

    Buf fork = {0};
    write_fork(&res, &fork);
    if (make_dirs(outdir)) { fprintf(stderr, "rescomp: cannot create %s\n", outdir); return 1; }
    char *base = path_join(outdir, outname);
    size_t on = strlen(outname);
    int has_ext = on >= 5 && str_ieq(outname + on - 5, ".rsrc");
    char *rsrcpath = has_ext ? xstrdup(base) : xsprintf("%s.rsrc", base);
    char *binpath = xsprintf("%s.bin", base);
    if (write_binary_file(rsrcpath, fork.d, fork.n)) { fprintf(stderr, "rescomp: cannot write %s\n", rsrcpath); return 1; }
    Buf mb = {0};
    write_macbinary(outname, ftype, creator, NULL, 0, fork.d, fork.n, 0, &mb);
    if (write_binary_file(binpath, mb.d, mb.n)) { fprintf(stderr, "rescomp: cannot write %s\n", binpath); return 1; }
    printf("rescomp: %s (%s/%s) %d resources, fork %zu bytes -> %s\n", outname, ftype, creator, res.n, fork.n, binpath);
    fflush(stdout);
    buf_free(&mb); buf_free(&fork); reslist_free(&res);
    free(base); free(rsrcpath); free(binpath); free(outname); free(rdir); free(sdirs); free(fmt); free(text);
    for (int i = 0; i < p.nlines; i++) free(p.lines[i]);
    free(p.lines);
    return 0;
}
