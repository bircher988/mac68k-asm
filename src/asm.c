/* asm.c - 68000 assembler for the classic Macintosh.
 *
 * Source dialect and code generation follow the MDS assembler (verified byte
 * for byte against real MDS builds):
 *   - labels are addressed PC-relative, numeric constants as abs.W when they fit
 *   - forward branches without a size are .W; forward BSR/BRA become JSR/JMP d16(PC);
 *     backward branches and explicit .S are short when they fit (.S silently widens)
 *   - CMP/ADD/SUB/AND/OR/EOR #imm,<ea> are encoded as CMPI/ADDI/... (An: CMPA/ADDA/SUBA)
 *   - DS.x reserves in the A5 globals area, not in the code; the first DS label is at
 *     -(256+total)(A5); references become d16(A5), label(Dn) becomes d8(A5,Dn)
 *   - DC.W/DC.L are aligned to even addresses and a label standing alone right before
 *     them moves along; instructions at odd addresses get a pad byte but labels stay
 *   - symbols are case-insensitive; modules have their own namespaces; XDEF exports
 *   - @local labels are scoped between two global labels
 *   - .TRAP name $Axxx, .MACRO name / %1..%9 / .ENDM, IF 'a' = 'b' / IF 'a' <> 'b' / ENDIF
 */
#define _GNU_SOURCE
#include "asm.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MODULES 64
#define MAX_ITER    64
#define MAX_OPS     8
#define MAX_WORDS   12

/* ---------------------------------------------------------------- lines ---- */

typedef struct {
    char *text;            /* the source line, comment included */
    const char *file;      /* file name for diagnostics */
    int lineno;
    int module;
    int in_macro_def;      /* part of a .MACRO definition: skipped in the passes */
} Line;

typedef struct { Line *v; int n, cap; } Lines;

static void lines_add(Lines *l, const char *text, const char *file, int lineno, int module) {
    if (l->n == l->cap) { l->cap = l->cap ? l->cap * 2 : 1024; l->v = xrealloc(l->v, l->cap * sizeof *l->v); }
    Line *ln = &l->v[l->n++];
    ln->text = xstrdup(text); ln->file = file; ln->lineno = lineno; ln->module = module; ln->in_macro_def = 0;
}

/* -------------------------------------------------------------- symbols ---- */

enum { K_UNDEF = 0, K_CONST, K_LABEL, K_DS, K_REG };
enum { SYM_LABEL = 1, SYM_CONST, SYM_DS, SYM_REG, SYM_MACRO, SYM_TRAP };

typedef struct Macro { StrList body; } Macro;

typedef struct Sym {
    char *name;            /* lowercase; local labels are stored as "scope@name" */
    int kind;              /* SYM_* */
    int module;            /* -1 = global (XDEF), else private to that module */
    long val;              /* label: code offset; DS: A5 offset; const: value; trap: word */
    long prev;             /* value in the previous pass (for convergence) */
    int reg;               /* SYM_REG: 0-7 = D0-D7, 8-15 = A0-A7 */
    int vkind;             /* SYM_CONST: K_CONST, or K_LABEL/K_DS when the EQU names a label */
    int defined;           /* defined in some pass (or a macro/trap) */
    int def_module;        /* module that defines the symbol */
    int def_count;         /* definitions seen in the current pass (redefinition check) */
    Macro *macro;
    int rom;               /* SYM_TRAP: ROM that introduced the trap (64 or 128), 0 = unknown */
    struct Sym *next;
} Sym;

#define HASH_SIZE 4096

static unsigned hash_str(const char *s) {
    unsigned h = 5381;
    for (; *s; s++) h = h * 33 + (unsigned char)*s;
    return h & (HASH_SIZE - 1);
}

/* ------------------------------------------------------------ assembler ---- */

typedef struct {
    int mode, reg;         /* mode 0-7 as in the instruction word; reg 0-7 */
    long disp; int disp_kind;
    int idx, idx_long;     /* index register 0-15, .L flag */
    long imm; int imm_kind;
    unsigned mask;         /* register list */
    int plain_label;       /* a bare code label (for the write-to-code-segment message) */
    int size_hint;         /* explicit .W/.L on an absolute address */
} EA;

/* modes beyond the 68000 fields */
enum { M_DN = 0, M_AN = 1, M_IND = 2, M_INC = 3, M_DEC = 4, M_D16 = 5, M_IDX = 6, M_EXT = 7,
       M_ABSW = 8, M_ABSL = 9, M_PCD16 = 10, M_PCIDX = 11, M_IMM = 12,
       M_SR = 20, M_CCR = 21, M_USP = 22, M_REGLIST = 23, M_NONE = 30 };

typedef struct {
    Lines lines;
    int nmodules;
    Sym *tab[HASH_SIZE];
    StrList xdef[MAX_MODULES];
    const AsmOptions *opt;
    StrList included;      /* files already included */

    /* pass state */
    int pass, emit, errors;
    Buf code;
    unsigned pc;
    int module;
    char scope[256];       /* current global label for @locals */
    unsigned ds_off, ds_total, ds_total_prev;
    unsigned ds_size[MAX_MODULES], ds_size_prev[MAX_MODULES];   /* DS bytes per module (rounded to even) */
    StrList lit[MAX_MODULES];          /* string literals used as operands ('text'), per module, deduplicated */
    Sym *pending[64]; int npending;   /* labels standing alone at pc, movable by DC.W alignment */
    int if_stack[64]; int if_depth;   /* 1 = active, 0 = skipping */
    int macro_depth;
    FILE *lst;
    const Line *cur;       /* line being processed (diagnostics) */
    int cur_is_macro_line;
    int cur_index;         /* index of the current source line */
    unsigned char *branch_long;   /* per source line: a short branch here did not fit once -> stays long */
} Asm;

static void error_at(Asm *A, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    if (A->cur) fprintf(stderr, "%s line %d: ", path_basename(A->cur->file), A->cur->lineno);
    vfprintf(stderr, fmt, ap); fputc('\n', stderr);
    va_end(ap);
    A->errors++;
}

static void warn_at(Asm *A, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    fprintf(stderr, "asm: warning: ");
    if (A->cur) fprintf(stderr, "%s line %d: ", path_basename(A->cur->file), A->cur->lineno);
    vfprintf(stderr, fmt, ap); fputc('\n', stderr);
    va_end(ap);
}

static Sym *sym_find_exact(Asm *A, const char *lname, int module) {
    for (Sym *s = A->tab[hash_str(lname)]; s; s = s->next)
        if (s->module == module && !strcmp(s->name, lname)) return s;
    return NULL;
}

/* Lookup as the modules see it: own private symbols first, then globals, then a
 * private symbol of exactly one other module (the linker's single namespace). */
static Sym *sym_lookup(Asm *A, const char *lname, int module) {
    Sym *s = sym_find_exact(A, lname, module);
    if (s) return s;
    s = sym_find_exact(A, lname, -1);
    if (s) return s;
    Sym *found = NULL; int n = 0;
    for (Sym *t = A->tab[hash_str(lname)]; t; t = t->next)
        if (!strcmp(t->name, lname) && t->module != module) { found = t; n++; }
    return n == 1 ? found : NULL;
}

static Sym *sym_create(Asm *A, const char *lname, int module, int kind) {
    Sym *s = xcalloc(1, sizeof *s);
    s->name = xstrdup(lname); s->module = module; s->kind = kind;
    unsigned h = hash_str(lname);
    s->next = A->tab[h]; A->tab[h] = s;
    return s;
}

static int name_is_xdef(Asm *A, const char *lname, int module) {
    StrList *l = &A->xdef[module];
    for (int i = 0; i < l->n; i++) if (!strcmp(l->v[i], lname)) return 1;
    return 0;
}

/* The symbol a definition in `module` refers to (global if XDEF'd there). */
static Sym *sym_for_def(Asm *A, const char *lname, int module, int kind) {
    int m = name_is_xdef(A, lname, module) ? -1 : module;
    Sym *s = sym_find_exact(A, lname, m);
    if (!s) s = sym_create(A, lname, m, kind);
    return s;
}

static char *lower_dup(const char *s) { return str_lower(xstrdup(s)); }

/* Local label "@x" -> "scope@x" (lowercase). Returns malloc'd. */
static char *scoped_name(Asm *A, const char *name) {
    if (name[0] == '@') return str_lower(xsprintf("%s%s", A->scope, name));
    return lower_dup(name);
}

/* ------------------------------------------------------------ registers ---- */

/* returns 0-7 Dn, 8-15 An, 16 PC, 17 SR, 18 CCR, 19 USP, -1 none */
static int reg_by_name(Asm *A, const char *tok) {
    size_t n = strlen(tok);
    if (n == 2 && (tok[0] == 'd' || tok[0] == 'D') && tok[1] >= '0' && tok[1] <= '7') return tok[1] - '0';
    if (n == 2 && (tok[0] == 'a' || tok[0] == 'A') && tok[1] >= '0' && tok[1] <= '7') return 8 + tok[1] - '0';
    if (str_ieq(tok, "sp")) return 15;
    if (str_ieq(tok, "pc")) return 16;
    if (str_ieq(tok, "sr")) return 17;
    if (str_ieq(tok, "ccr")) return 18;
    if (str_ieq(tok, "usp")) return 19;
    if (A) {
        char *l = lower_dup(tok);
        Sym *s = sym_lookup(A, l, A->module);
        free(l);
        if (s && s->kind == SYM_REG) return s->reg;
    }
    return -1;
}

/* ---------------------------------------------------------- expressions ---- */

typedef struct { long val; int kind; int undefined; int external; } Val;   /* external: defined in another module */

typedef struct { Asm *A; const char *s; int err; } Expr;

static int is_ident_start(int c) { return isalpha(c) || c == '_' || c == '@' || c == '.'; }
static int is_ident_char(int c) { return isalnum(c) || c == '_' || c == '@' || c == '.'; }

static void skip_ws(Expr *e) { while (*e->s == ' ' || *e->s == '\t') e->s++; }

static Val val_const(long v) { Val r = { v, K_CONST, 0, 0 }; return r; }

static Val combine(Val a, Val b, int op) {
    Val r = val_const(0);
    r.undefined = a.undefined || b.undefined;
    r.external = a.external || b.external;
    switch (op) {
    case '+': r.val = a.val + b.val;
        r.kind = (a.kind == K_CONST) ? b.kind : (b.kind == K_CONST ? a.kind : K_CONST); break;
    case '-': r.val = a.val - b.val;
        r.kind = (b.kind == K_CONST) ? a.kind : (a.kind == b.kind ? K_CONST : a.kind); break;
    case '*': r.val = a.val * b.val; break;
    case '/': r.val = b.val ? a.val / b.val : 0; break;
    case '%': r.val = b.val ? a.val % b.val : 0; break;
    case '<': r.val = a.val << b.val; break;
    case '>': r.val = a.val >> b.val; break;
    case '&': r.val = a.val & b.val; break;
    case '|': r.val = a.val | b.val; break;
    case '^': r.val = a.val ^ b.val; break;
    }
    return r;
}

static Val expr_or(Expr *e);

static Val expr_primary(Expr *e) {
    skip_ws(e);
    const char *s = e->s;
    if (*s == '(') {
        e->s++;
        Val v = expr_or(e);
        skip_ws(e);
        if (*e->s == ')') e->s++; else e->err = 1;
        return v;
    }
    if (*s == '-') { e->s++; Val v = expr_primary(e); v.val = -v.val; return v; }
    if (*s == '+') { e->s++; return expr_primary(e); }
    if (*s == '~') { e->s++; Val v = expr_primary(e); v.val = ~v.val; v.kind = K_CONST; return v; }
    if (*s == '!') { e->s++; Val v = expr_primary(e); v.val = !v.val; v.kind = K_CONST; return v; }
    if (*s == '$') {
        s++; long v = 0; int n = 0;
        while (isxdigit((unsigned char)*s)) { v = v * 16 + (isdigit((unsigned char)*s) ? *s - '0' : (tolower((unsigned char)*s) - 'a' + 10)); s++; n++; }
        if (!n) e->err = 1;
        e->s = s; return val_const((long)(int32_t)v);
    }
    if (*s == '%' && (s[1] == '0' || s[1] == '1')) {
        s++; long v = 0;
        while (*s == '0' || *s == '1') v = v * 2 + (*s++ - '0');
        e->s = s; return val_const(v);
    }
    if (isdigit((unsigned char)*s)) {
        long v = 0;
        while (isdigit((unsigned char)*s)) v = v * 10 + (*s++ - '0');
        e->s = s; return val_const((long)(int32_t)v);
    }
    if (*s == '\'' || *s == '"') {
        char q = *s++; long v = 0; int n = 0;
        while (*s && *s != q) { v = (v << 8) | (unsigned char)*s++; n++; }
        if (*s == q) s++;
        e->s = s; return val_const(n ? v : 0);
    }
    if (*s == '*' ) { e->s++; Val v = val_const(e->A->pc); v.kind = K_LABEL; return v; }
    if (is_ident_start((unsigned char)*s)) {
        const char *b = s;
        while (is_ident_char((unsigned char)*s)) s++;
        char *name = xstrndup(b, s - b);
        e->s = s;
        Asm *A = e->A;
        char *ln = scoped_name(A, name);
        Sym *sym = sym_lookup(A, ln, A->module);
        free(ln); free(name);
        Val v = val_const(0);
        if (!sym) { v.undefined = 1; return v; }
        switch (sym->kind) {
        case SYM_LABEL: v.kind = K_LABEL; break;
        case SYM_DS:    v.kind = K_DS; break;
        case SYM_CONST: v.kind = sym->vkind ? sym->vkind : K_CONST; break;
        case SYM_REG:   v.kind = K_REG; v.val = sym->reg; break;
        case SYM_TRAP:  v.kind = K_CONST; break;
        default:        v.undefined = 1; break;
        }
        v.val = sym->kind == SYM_REG ? sym->reg : sym->val;
        if (!sym->defined) v.undefined = 1;
        else if (sym->kind == SYM_LABEL && sym->def_module != A->module) v.external = 1;
        return v;
    }
    e->err = 1;
    return val_const(0);
}

static Val expr_mul(Expr *e) {
    Val v = expr_primary(e);
    for (;;) {
        skip_ws(e);
        if (*e->s == '*' || *e->s == '/' || *e->s == '%') { int op = *e->s++; v = combine(v, expr_primary(e), op); }
        else return v;
    }
}

static Val expr_add(Expr *e) {
    Val v = expr_mul(e);
    for (;;) {
        skip_ws(e);
        if (*e->s == '+' || *e->s == '-') { int op = *e->s++; v = combine(v, expr_mul(e), op); }
        else return v;
    }
}

static Val expr_shift(Expr *e) {
    Val v = expr_add(e);
    for (;;) {
        skip_ws(e);
        if (e->s[0] == '<' && e->s[1] == '<') { e->s += 2; v = combine(v, expr_add(e), '<'); }
        else if (e->s[0] == '>' && e->s[1] == '>') { e->s += 2; v = combine(v, expr_add(e), '>'); }
        else return v;
    }
}

static Val expr_and(Expr *e) {
    Val v = expr_shift(e);
    for (;;) { skip_ws(e); if (*e->s == '&') { e->s++; v = combine(v, expr_shift(e), '&'); } else return v; }
}

static Val expr_xor(Expr *e) {
    Val v = expr_and(e);
    for (;;) { skip_ws(e); if (*e->s == '^') { e->s++; v = combine(v, expr_and(e), '^'); } else return v; }
}

static Val expr_or(Expr *e) {
    Val v = expr_xor(e);
    for (;;) { skip_ws(e); if (*e->s == '|') { e->s++; v = combine(v, expr_xor(e), '|'); } else return v; }
}

/* Evaluate a whole string. Returns 0 on syntax error. */
static int eval_quiet(Asm *A, const char *s, Val *out) {
    Expr e = { A, s, 0 };
    *out = expr_or(&e);
    skip_ws(&e);
    return !(e.err || *e.s);
}

static int eval(Asm *A, const char *s, Val *out) {
    if (!eval_quiet(A, s, out)) return 0;
    if (out->undefined && A->emit) error_at(A, "undefined symbol in '%s'", s);
    return 1;
}

/* -------------------------------------------------------- line splitting ---- */

/* Split a line into label, mnemonic and operand text. Comments (';' outside quotes,
 * '*' in column 0) are removed; blanks inside the operands are removed outside quotes.
 * Returns 0 for an empty line. All three outputs are malloc'd (may be empty strings). */
static int split_line(const char *text, char **label, char **op, char **args, char **raw) {
    char *line = xstrdup(text);
    /* strip the comment */
    if (line[0] == '*') line[0] = 0;
    {
        char q = 0;
        for (char *p = line; *p; p++) {
            if (q) { if (*p == q) q = 0; }
            else if (*p == '\'' || *p == '"') q = *p;
            else if (*p == ';') { *p = 0; break; }
        }
    }
    *label = xstrdup(""); *op = xstrdup(""); *args = xstrdup(""); *raw = xstrdup("");
    if (is_blank_line(line)) { free(line); return 0; }
    char *p = line;
    int col0 = !(line[0] == ' ' || line[0] == '\t');
    /* tokens: first, second, rest */
    while (*p == ' ' || *p == '\t') p++;
    char *t1 = p; while (*p && *p != ' ' && *p != '\t') p++;
    char *t1e = p;
    while (*p == ' ' || *p == '\t') p++;
    char *t2 = p; while (*p && *p != ' ' && *p != '\t') p++;
    char *t2e = p;
    while (*p == ' ' || *p == '\t') p++;
    char *rest = p;
    char tok1[512], tok2[512];
    snprintf(tok1, sizeof tok1, "%.*s", (int)(t1e - t1), t1);
    snprintf(tok2, sizeof tok2, "%.*s", (int)(t2e - t2), t2);
    size_t l1 = strlen(tok1);
    int t1_colon = l1 > 1 && tok1[l1-1] == ':';
    if (t1_colon) tok1[l1-1] = 0;
    static const char *dirs_col0[] = { "include", "xdef", "xref", "end", ".trap", ".macro", ".endm", "macro", "endm",
                                       "if", "else", "endif", "endc", "even" };
    static const char *label_kw[] = { "equ", "equr", "set", "dc", "dc.b", "dc.w", "dc.l", "ds", "ds.b", "ds.w", "ds.l",
                                      "dcb", "dcb.b", "dcb.w", "dcb.l", "macro", ".macro" };
    int is_label = 0;
    if (t1_colon) is_label = 1;
    else if (col0) {
        is_label = 1;
        for (size_t i = 0; i < sizeof dirs_col0 / sizeof *dirs_col0; i++) if (str_ieq(tok1, dirs_col0[i])) is_label = 0;
    } else {
        for (size_t i = 0; i < sizeof label_kw / sizeof *label_kw; i++) if (str_ieq(tok2, label_kw[i])) is_label = 1;
    }
    char *opsrc;
    if (is_label) {
        free(*label); *label = xstrdup(tok1);
        free(*op); *op = xstrdup(tok2);
        opsrc = rest;
    } else {
        free(*op); *op = xstrdup(tok1);
        opsrc = t2;   /* second token and everything after it */
    }
    free(*raw); *raw = str_trim(xstrdup(opsrc));
    /* operands: remove blanks outside quotes */
    {
        char *o = xmalloc(strlen(opsrc) + 1); size_t n = 0; char q = 0;
        for (const char *s = opsrc; *s; s++) {
            if (q) { o[n++] = *s; if (*s == q) q = 0; }
            else if (*s == '\'' || *s == '"') { q = *s; o[n++] = *s; }
            else if (*s == ' ' || *s == '\t') continue;
            else o[n++] = *s;
        }
        o[n] = 0;
        free(*args); *args = o;
    }
    free(line);
    return 1;
}

/* Split operands at commas outside parentheses and quotes. Returns count. */
static int split_operands(const char *args, char **ops, int max) {
    int n = 0; int depth = 0; char q = 0;
    const char *start = args;
    if (!*args) return 0;
    for (const char *s = args;; s++) {
        if (q) { if (*s == q) q = 0; }
        else if (*s == '\'' || *s == '"') q = *s;
        else if (*s == '(') depth++;
        else if (*s == ')') depth--;
        if ((*s == ',' && depth == 0 && !q) || !*s) {
            if (n < max) ops[n++] = xstrndup(start, s - start);
            start = s + 1;
            if (!*s) break;
        }
    }
    return n;
}

/* ------------------------------------------------------------- operands ---- */

/* "D0-D2/A0" -> mask (bit 0-7 = D0-D7, 8-15 = A0-A7). Returns 1 if the whole
 * string is a register list with at least one '-' or '/'. */
static int parse_reglist(Asm *A, const char *s, unsigned *mask) {
    unsigned m = 0; int had_sep = 0;
    const char *p = s;
    while (*p) {
        const char *b = p;
        while (*p && *p != '-' && *p != '/') p++;
        char *tok = xstrndup(b, p - b);
        int r1 = reg_by_name(A, tok); free(tok);
        if (r1 < 0 || r1 > 15) return 0;
        if (*p == '-') {
            p++; b = p;
            while (*p && *p != '/') p++;
            tok = xstrndup(b, p - b);
            int r2 = reg_by_name(A, tok); free(tok);
            if (r2 < 0 || r2 > 15 || r2 < r1) return 0;
            for (int r = r1; r <= r2; r++) m |= 1u << r;
            had_sep = 1;
        } else m |= 1u << r1;
        if (*p == '/') { p++; had_sep = 1; if (!*p) return 0; }
    }
    if (!had_sep) return 0;
    *mask = m;
    return 1;
}

/* index register spec "Dn", "An.W", "D0.L" -> 0-15, sets *is_long. -1 if not one. */
static int parse_index(Asm *A, const char *s, int *is_long) {
    char buf[64];
    snprintf(buf, sizeof buf, "%s", s);
    *is_long = 0;
    size_t n = strlen(buf);
    if (n > 2 && buf[n-2] == '.') {
        if (buf[n-1] == 'l' || buf[n-1] == 'L') *is_long = 1;
        else if (buf[n-1] == 'w' || buf[n-1] == 'W') *is_long = 0;
        else return -1;
        buf[n-2] = 0;
    }
    int r = reg_by_name(A, buf);
    return (r >= 0 && r <= 15) ? r : -1;
}

static void ea_from_val(Asm *A, EA *ea, Val v, int size_hint) {
    ea->disp = v.val; ea->disp_kind = v.kind;
    if (v.kind == K_LABEL) { ea->mode = M_PCD16; ea->plain_label = 1; }
    else if (v.kind == K_DS) { ea->mode = M_D16; ea->reg = 5; }
    else if (size_hint == 4) ea->mode = M_ABSL;
    else if (size_hint == 2) ea->mode = M_ABSW;
    else ea->mode = (v.val >= -32768 && v.val <= 32767) ? M_ABSW : M_ABSL;
}

/* Parse one operand. Returns 1 on success (errors are reported). */
static int parse_ea(Asm *A, const char *src, EA *ea) {
    memset(ea, 0, sizeof *ea);
    ea->mode = M_NONE;
    char *s = xstrdup(src);
    size_t n = strlen(s);
    int ok = 1;
    Val v;

    if (!n) { error_at(A, "missing operand"); free(s); return 0; }

    if (s[0] == '#') {
        if (!eval(A, s + 1, &v)) { error_at(A, "bad expression '%s'", s + 1); ok = 0; }
        else if (v.kind == K_LABEL && A->emit) {
            error_at(A, "illegal or missing operand(s): '%s' names a code label, whose address is only known at run time. Use LEA %s,An", src, s + 1);
            ok = 0;
        } else if (v.kind == K_DS && A->emit) {
            error_at(A, "illegal or missing operand(s): '%s' names a DS variable. Use LEA %s,An", src, s + 1);
            ok = 0;
        }
        ea->mode = M_IMM; ea->imm = v.val; ea->imm_kind = v.kind;
        free(s); return ok;
    }
    /* 'text' as a memory operand: a Pascal string placed at the end of the module (MDS) */
    if ((s[0] == '\'' || s[0] == '"') && n >= 2 && s[n-1] == s[0]) {
        Buf t = {0};
        for (size_t i = 1; i + 1 < n; i++) {
            if (s[i] == s[0] && s[i+1] == s[0]) { buf_put8(&t, (unsigned char)s[0]); i++; }
            else buf_put8(&t, (unsigned char)s[i]);
        }
        if (t.n > 255) { error_at(A, "string literal longer than 255 characters"); buf_free(&t); free(s); return 0; }
        buf_put8(&t, 0);
        int m = A->module < 0 ? 0 : A->module;
        int k = -1;
        for (int i = 0; i < A->lit[m].n; i++) if (!strcmp(A->lit[m].v[i], (char *)t.d)) { k = i; break; }
        if (k < 0) { k = A->lit[m].n; strlist_add(&A->lit[m], (char *)t.d); }
        buf_free(&t);
        char *ln = xsprintf("$lit_%d", k);
        Sym *sym = sym_find_exact(A, ln, m);
        free(ln);
        ea->mode = M_PCD16; ea->disp_kind = K_LABEL; ea->disp = sym && sym->defined ? sym->val : 0;
        free(s); return 1;
    }
    /* -(An) */
    if (s[0] == '-' && s[1] == '(' && s[n-1] == ')') {
        char *inner = xstrndup(s + 2, n - 3);
        int r = reg_by_name(A, inner); free(inner);
        if (r >= 8 && r <= 15) { ea->mode = M_DEC; ea->reg = r - 8; free(s); return 1; }
    }
    /* (An)+ */
    if (s[0] == '(' && n > 3 && s[n-1] == '+' && s[n-2] == ')') {
        char *inner = xstrndup(s + 1, n - 3);
        int r = reg_by_name(A, inner); free(inner);
        if (r >= 8 && r <= 15) { ea->mode = M_INC; ea->reg = r - 8; free(s); return 1; }
        error_at(A, "bad operand '%s'", s); free(s); return 0;
    }
    /* register list */
    if (strchr(s, '/') || (strchr(s + 1, '-') && s[0] != '(' && s[0] != '-')) {
        unsigned m;
        if (parse_reglist(A, s, &m)) { ea->mode = M_REGLIST; ea->mask = m; free(s); return 1; }
    }
    /* a register */
    {
        int r = reg_by_name(A, s);
        if (r >= 0) {
            if (r < 8) { ea->mode = M_DN; ea->reg = r; }
            else if (r < 16) { ea->mode = M_AN; ea->reg = r - 8; }
            else if (r == 17) ea->mode = M_SR;
            else if (r == 18) ea->mode = M_CCR;
            else if (r == 19) ea->mode = M_USP;
            else { error_at(A, "PC is not a valid operand here"); ok = 0; }
            free(s); return ok;
        }
    }
    /* something(...) : find the '(' matching the final ')' */
    if (s[n-1] == ')') {
        int depth = 0; long i;
        for (i = (long)n - 1; i >= 0; i--) {
            if (s[i] == ')') depth++;
            else if (s[i] == '(') { depth--; if (depth == 0) break; }
        }
        if (i >= 0) {
            char *before = xstrndup(s, i);
            char *inner = xstrndup(s + i + 1, n - i - 2);
            char *parts[4]; int np = split_operands(inner, parts, 4);
            int base = -1, idx = -1, idx_long = 0; const char *dispsrc = before; int form_ok = 0;
            if (np == 1) {
                base = reg_by_name(A, parts[0]);
                form_ok = base >= 0;
            } else if (np == 2) {
                int r0 = reg_by_name(A, parts[0]);
                if (r0 >= 8 && r0 <= 16) {           /* (An,Xn) or (PC,Xn) */
                    base = r0; idx = parse_index(A, parts[1], &idx_long); form_ok = idx >= 0;
                } else if (!*before) {                /* (d,An) or (d,PC) */
                    dispsrc = parts[0]; base = reg_by_name(A, parts[1]); form_ok = base >= 8 && base <= 16;
                }
            } else if (np == 3 && !*before) {         /* (d,An,Xn) */
                dispsrc = parts[0]; base = reg_by_name(A, parts[1]);
                idx = parse_index(A, parts[2], &idx_long); form_ok = base >= 8 && base <= 16 && idx >= 0;
            }
            if (form_ok) {
                if (*dispsrc) {
                    if (!eval(A, dispsrc, &v)) { error_at(A, "bad expression '%s'", dispsrc); ok = 0; }
                } else v = val_const(0);
                ea->disp = v.val; ea->disp_kind = v.kind;
                if (base >= 8 && base <= 15) {           /* An based */
                    ea->reg = base - 8;
                    if (idx >= 0) { ea->mode = M_IDX; ea->idx = idx; ea->idx_long = idx_long; }
                    else if (!*dispsrc && np == 1) ea->mode = M_IND;
                    else ea->mode = M_D16;
                } else if (base == 16) {                 /* PC based */
                    if (idx >= 0) { ea->mode = M_PCIDX; ea->idx = idx; ea->idx_long = idx_long; }
                    else ea->mode = M_PCD16;
                } else if (base >= 0 && base < 8 && np == 1) {
                    /* label(Dn): PC-indexed for code labels, A5-indexed for DS variables */
                    idx = parse_index(A, parts[0], &idx_long);
                    ea->idx = idx; ea->idx_long = idx_long;
                    if (v.kind == K_DS) { ea->mode = M_IDX; ea->reg = 5; }
                    else ea->mode = M_PCIDX;
                } else ok = 0;
                for (int k = 0; k < np; k++) free(parts[k]);
                free(before); free(inner); free(s);
                return ok;
            }
            for (int k = 0; k < np; k++) free(parts[k]);
            free(before); free(inner);
        }
    }
    /* expression, optionally with .W / .L */
    {
        int hint = 0;
        if (n > 2 && s[n-2] == '.' && (s[n-1] == 'w' || s[n-1] == 'W')) { hint = 2; s[n-2] = 0; }
        else if (n > 2 && s[n-2] == '.' && (s[n-1] == 'l' || s[n-1] == 'L')) { hint = 4; s[n-2] = 0; }
        if (!eval(A, s, &v)) { error_at(A, "bad operand '%s'", src); free(s); return 0; }
        if (v.kind == K_REG) { error_at(A, "register alias not allowed here: '%s'", src); free(s); return 0; }
        ea_from_val(A, ea, v, hint);
        free(s);
        return 1;
    }
}

/* ---- addressing mode categories ---- */
#define B(m) (1u << (m))
#define CAT_ALL      (B(M_DN)|B(M_AN)|B(M_IND)|B(M_INC)|B(M_DEC)|B(M_D16)|B(M_IDX)|B(M_ABSW)|B(M_ABSL)|B(M_PCD16)|B(M_PCIDX)|B(M_IMM))
#define CAT_DATA     (CAT_ALL & ~B(M_AN))
#define CAT_MEM      (CAT_ALL & ~(B(M_DN)|B(M_AN)))
#define CAT_CTRL     (B(M_IND)|B(M_D16)|B(M_IDX)|B(M_ABSW)|B(M_ABSL)|B(M_PCD16)|B(M_PCIDX))
#define CAT_ALT      (CAT_ALL & ~(B(M_PCD16)|B(M_PCIDX)|B(M_IMM)))
#define CAT_DATA_ALT (CAT_ALT & ~B(M_AN))
#define CAT_MEM_ALT  (CAT_ALT & ~(B(M_DN)|B(M_AN)))
#define CAT_CTRL_ALT (CAT_CTRL & ~(B(M_PCD16)|B(M_PCIDX)))

static int ea_check(Asm *A, const EA *ea, unsigned cat, const char *mn) {
    if (ea->mode <= M_IMM && (cat & B(ea->mode))) return 1;
    if (ea->plain_label && (ea->mode == M_PCD16 || ea->mode == M_PCIDX) && !(cat & B(M_PCD16)))
        error_at(A, "a label in the code segment cannot be written (%s): the 68000 reads PC-relative only. "
                    "Load the address first (LEA label,An) or declare the variable with DS", mn);
    else
        error_at(A, "illegal or missing operand(s) for %s", mn);
    return 0;
}

static unsigned ea_field(const EA *ea) {
    switch (ea->mode) {
    case M_ABSW:  return (7u << 3) | 0;
    case M_ABSL:  return (7u << 3) | 1;
    case M_PCD16: return (7u << 3) | 2;
    case M_PCIDX: return (7u << 3) | 3;
    case M_IMM:   return (7u << 3) | 4;
    default:      return ((unsigned)ea->mode << 3) | (unsigned)ea->reg;
    }
}

static int fits(long v, long lo, long hi) { return v >= lo && v <= hi; }

/* Append the extension word(s) of an operand. addr = address of the instruction,
 * *nw = words so far (the extension goes at addr + 2*(*nw)). */
static int ea_ext(Asm *A, const EA *ea, unsigned addr, unsigned *w, int *nw, int opsize) {
    unsigned here = addr + 2u * (unsigned)*nw;
    long d;
    switch (ea->mode) {
    case M_D16:
        if (A->emit && !fits(ea->disp, -32768, 32767)) error_at(A, "displacement out of range (%ld)", ea->disp);
        w[(*nw)++] = (unsigned)ea->disp & 0xffff; break;
    case M_IDX:
        if (A->emit && !fits(ea->disp, -128, 127)) error_at(A, "index displacement out of range (%ld)", ea->disp);
        w[(*nw)++] = ((unsigned)ea->idx << 12) | ((unsigned)ea->idx_long << 11) | ((unsigned)ea->disp & 0xff); break;
    case M_ABSW: w[(*nw)++] = (unsigned)ea->disp & 0xffff; break;
    case M_ABSL: w[(*nw)++] = ((unsigned)ea->disp >> 16) & 0xffff; w[(*nw)++] = (unsigned)ea->disp & 0xffff; break;
    case M_PCD16:
        d = ea->disp_kind == K_LABEL ? ea->disp - (long)here : ea->disp;   /* a plain number in d(PC) is used as is */
        if (A->emit && !fits(d, -32768, 32767)) error_at(A, "PC-relative displacement out of range (%ld)", d);
        w[(*nw)++] = (unsigned)d & 0xffff; break;
    case M_PCIDX:
        d = ea->disp_kind == K_LABEL ? ea->disp - (long)here : ea->disp;
        if (A->emit && !fits(d, -128, 127)) error_at(A, "PC-relative index displacement out of range (%ld)", d);
        w[(*nw)++] = ((unsigned)ea->idx << 12) | ((unsigned)ea->idx_long << 11) | ((unsigned)d & 0xff); break;
    case M_IMM:
        if (opsize == 4) { w[(*nw)++] = ((unsigned)ea->imm >> 16) & 0xffff; w[(*nw)++] = (unsigned)ea->imm & 0xffff; }
        else if (opsize == 1) {
            if (A->emit && !fits(ea->imm, -128, 255)) warn_at(A, "immediate value %ld does not fit in a byte", ea->imm);
            w[(*nw)++] = (unsigned)ea->imm & 0xff;
        } else {
            if (A->emit && !fits(ea->imm, -32768, 65535)) warn_at(A, "immediate value %ld does not fit in a word", ea->imm);
            w[(*nw)++] = (unsigned)ea->imm & 0xffff;
        }
        break;
    default: break;
    }
    return 1;
}

/* --------------------------------------------------------- instructions ---- */

static int cond_code(const char *cc) {
    static const struct { const char *n; int c; } t[] = {
        {"t",0},{"f",1},{"hi",2},{"ls",3},{"cc",4},{"hs",4},{"cs",5},{"lo",5},{"ne",6},{"eq",7},
        {"vc",8},{"vs",9},{"pl",10},{"mi",11},{"ge",12},{"lt",13},{"gt",14},{"le",15},{NULL,0}};
    for (int i = 0; t[i].n; i++) if (!strcmp(cc, t[i].n)) return t[i].c;
    return -1;
}

static int sizebits(int sz) { return sz == 1 ? 0 : sz == 2 ? 1 : 2; }

static int size_from_suffix(int suf, int dflt) {
    switch (suf) { case 'b': return 1; case 'w': return 2; case 'l': return 4; case 's': return 1; default: return dflt; }
}

#define EMIT(x) (w[(*nw)++] = (unsigned)(x) & 0xffff)

static int parse_two(Asm *A, char **ops, int nops, EA *a, EA *b, const char *mn) {
    if (nops != 2) { error_at(A, "%s needs two operands", mn); return 0; }
    return parse_ea(A, ops[0], a) && parse_ea(A, ops[1], b);
}

static int parse_one(Asm *A, char **ops, int nops, EA *a, const char *mn) {
    if (nops != 1) { error_at(A, "%s needs one operand", mn); return 0; }
    return parse_ea(A, ops[0], a);
}

/* Immediate-to-<ea> instructions: ORI ANDI SUBI ADDI EORI CMPI (also to CCR/SR). */
static int enc_imm_ea(Asm *A, unsigned op, int sz, EA *src, EA *dst, unsigned addr, unsigned *w, int *nw, const char *mn) {
    if (dst->mode == M_CCR || dst->mode == M_SR) {
        if (op == 0x0400 || op == 0x0600 || op == 0x0C00) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        EMIT(op | (dst->mode == M_SR ? 0x7C : 0x3C));
        EMIT(dst->mode == M_SR ? src->imm : (src->imm & 0xff));
        return 1;
    }
    if (!ea_check(A, dst, CAT_DATA_ALT, mn)) return 0;
    EMIT(op | (sizebits(sz) << 6) | ea_field(dst));
    if (!ea_ext(A, src, addr, w, nw, sz)) return 0;
    return ea_ext(A, dst, addr, w, nw, sz);
}

static int enc_branch(Asm *A, int cc, int suf, char **ops, int nops, unsigned addr, unsigned *w, int *nw, const char *mn) {
    Val v;
    if (nops != 1) { error_at(A, "%s needs one operand", mn); return 0; }
    if (!eval(A, ops[0], &v)) { error_at(A, "bad branch target '%s'", ops[0]); return 0; }
    long disp = v.val - (long)(addr + 2);
    /* a target not yet known in this pass is a label defined further down: forward.
     * A label from another module is unknown to a one-pass assembler as well (MDS). */
    int unknown = v.undefined || v.external;
    int forward = unknown || v.val > (long)addr;
    int use_short;
    if (suf == 'l') { error_at(A, "%s.L is not available on the 68000", mn); return 0; }
    int fit = (v.undefined || (fits(disp, -128, 127) && disp != 0)) && !A->branch_long[A->cur_index];
    if (v.external && (suf == 's' || suf == 'b')) fit = fits(disp, -128, 127) && disp != 0;
    /* Bcc.S to the next instruction: a NOP (MDS). It is as long as a short branch, so it
     * must not mark the branch long: in a pass where earlier code has just grown, a forward
     * Bcc.S over one instruction sees a stale displacement of 0 here, and a permanent .W
     * would push the next such branch into the same state - one per pass, until the
     * assembly no longer converges. */
    int nop = (suf == 's' || suf == 'b') && disp == 0 && !unknown && cc != 1 && !A->branch_long[A->cur_index];
    if (suf == 's' || suf == 'b') use_short = fit;
    else if (forward) use_short = 0;           /* forward: .W, BSR/BRA as JSR/JMP (MDS assembles in one pass) */
    else use_short = fit;                      /* backward: short when it fits, even with an explicit .W (MDS) */
    if (!use_short && !nop && !v.undefined) A->branch_long[A->cur_index] = 1;   /* never shrink again: guarantees convergence */
    if (!use_short && !nop && (suf == 's' || suf == 'b') && A->emit) warn_at(A, "%s.S does not reach its target, widened to .W", mn);
    if (use_short) { EMIT(0x6000 | (cc << 8) | ((unsigned)disp & 0xff)); return 1; }
    if (nop) { EMIT(0x4E71); return 1; }
    if (forward && (cc == 0 || cc == 1)) {     /* BRA/BSR to an unknown label -> JMP/JSR d16(PC) (MDS) */
        EMIT(cc == 0 ? 0x4EFA : 0x4EBA);
    } else {
        EMIT(0x6000 | (cc << 8));
    }
    if (A->emit && !fits(disp, -32768, 32767)) error_at(A, "branch out of range");
    EMIT(disp);
    return 1;
}

static int enc_shift(Asm *A, unsigned type, int dir, int sz, char **ops, int nops, unsigned addr, unsigned *w, int *nw, const char *mn) {
    EA a, b;
    if (nops == 1) {
        if (!parse_ea(A, ops[0], &a)) return 0;
        if (a.mode == M_DN) { EMIT(0xE000 | (1u << 9) | (dir << 8) | (sizebits(sz) << 6) | (type << 3) | a.reg); return 1; }
        if (sz != 2) { error_at(A, "memory shifts are word-sized (%s)", mn); return 0; }
        if (!ea_check(A, &a, CAT_MEM_ALT, mn)) return 0;
        EMIT(0xE0C0 | (type << 9) | (dir << 8) | ea_field(&a));
        return ea_ext(A, &a, addr, w, nw, 2);
    }
    if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
    if (b.mode != M_DN) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
    if (a.mode == M_IMM) {
        if (A->emit && !fits(a.imm, 1, 8)) { error_at(A, "shift count must be 1..8"); return 0; }
        EMIT(0xE000 | (((unsigned)a.imm & 7) << 9) | (dir << 8) | (sizebits(sz) << 6) | (type << 3) | b.reg);
        return 1;
    }
    if (a.mode == M_DN) { EMIT(0xE000 | ((unsigned)a.reg << 9) | (dir << 8) | (sizebits(sz) << 6) | (1u << 5) | (type << 3) | b.reg); return 1; }
    error_at(A, "illegal or missing operand(s) for %s", mn);
    return 0;
}

/* ADD/SUB/AND/OR/CMP/EOR with register or memory operands (immediates handled by the caller). */
static int enc_arith(Asm *A, const char *mn, unsigned op, int sz, EA *a, EA *b, unsigned addr, unsigned *w, int *nw) {
    int is_cmp = op == 0xB000, is_eor = op == 0xB100;
    int is_addsub = op == 0x9000 || op == 0xD000;
    if (b->mode == M_AN && (is_cmp || is_addsub)) {            /* ADDA/SUBA/CMPA */
        if (sz == 1) { error_at(A, "byte size not allowed with an address register (%s)", mn); return 0; }
        EMIT((is_cmp ? 0xB000 : op) | ((unsigned)b->reg << 9) | ((sz == 4 ? 7u : 3u) << 6) | ea_field(a));
        return ea_ext(A, a, addr, w, nw, sz);
    }
    if (is_eor) {
        if (a->mode != M_DN) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        if (!ea_check(A, b, CAT_DATA_ALT, mn)) return 0;
        EMIT(0xB100 | ((unsigned)a->reg << 9) | (sizebits(sz) << 6) | ea_field(b));
        return ea_ext(A, b, addr, w, nw, sz);
    }
    if (is_cmp && a->mode == M_INC && b->mode == M_INC) {      /* CMPM */
        EMIT(0xB108 | ((unsigned)b->reg << 9) | (sizebits(sz) << 6) | a->reg);
        return 1;
    }
    if (b->mode == M_DN) {                                     /* <ea>,Dn */
        unsigned cat = is_cmp ? CAT_ALL : CAT_ALL;
        if (sz == 1 && a->mode == M_AN) { error_at(A, "byte size not allowed with an address register (%s)", mn); return 0; }
        if (!ea_check(A, a, cat, mn)) return 0;
        EMIT(op | ((unsigned)b->reg << 9) | (sizebits(sz) << 6) | ea_field(a));
        return ea_ext(A, a, addr, w, nw, sz);
    }
    if (a->mode == M_DN && !is_cmp) {                          /* Dn,<ea> */
        if (!ea_check(A, b, CAT_MEM_ALT, mn)) return 0;
        EMIT(op | ((unsigned)a->reg << 9) | ((4u + sizebits(sz)) << 6) | ea_field(b));
        return ea_ext(A, b, addr, w, nw, sz);
    }
    error_at(A, "illegal or missing operand(s) for %s", mn);
    return 0;
}

/* Encode one instruction. mn: lowercase mnemonic without size; suf: 0/'b'/'w'/'l'/'s'.
 * Returns 1 = encoded, 0 = error reported, -1 = not an instruction. */
static int encode_instr(Asm *A, const char *mn, int suf, char **ops, int nops, unsigned addr, unsigned *w, int *nw) {
    EA a, b;
    int sz = size_from_suffix(suf, 2);
    *nw = 0;
    size_t ml = strlen(mn);

    /* ---- branches, Scc, DBcc ---- */
    if (mn[0] == 'b' && ml >= 3 && ml <= 3) {
        int cc = !strcmp(mn, "bra") ? 0 : !strcmp(mn, "bsr") ? 1 : cond_code(mn + 1);
        if (cc >= 2 || !strcmp(mn, "bra") || !strcmp(mn, "bsr"))
            return enc_branch(A, cc, suf, ops, nops, addr, w, nw, mn);
    }
    if (mn[0] == 'd' && mn[1] == 'b' && ml >= 3) {
        int cc = !strcmp(mn, "dbra") ? 1 : cond_code(mn + 2);
        if (cc >= 0) {
            Val v;
            if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
            if (a.mode != M_DN) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
            if (!eval(A, ops[1], &v)) return 0;
            long disp = v.val - (long)(addr + 2);
            EMIT(0x50C8 | (cc << 8) | a.reg);
            if (A->emit && !fits(disp, -32768, 32767)) error_at(A, "branch out of range");
            EMIT(disp);
            return 1;
        }
    }
    if (mn[0] == 's' && (ml == 2 || ml == 3) && strcmp(mn, "sub") && strcmp(mn, "swap")) {
        int cc = cond_code(mn + 1);
        if (cc >= 0) {
            if (!parse_one(A, ops, nops, &a, mn) || !ea_check(A, &a, CAT_DATA_ALT, mn)) return 0;
            EMIT(0x50C0 | (cc << 8) | ea_field(&a));
            return ea_ext(A, &a, addr, w, nw, 1);
        }
    }

    /* ---- no-operand instructions ---- */
    if (!strcmp(mn, "rts")) { EMIT(0x4E75); return 1; }
    if (!strcmp(mn, "nop")) { EMIT(0x4E71); return 1; }
    if (!strcmp(mn, "rte")) { EMIT(0x4E73); return 1; }
    if (!strcmp(mn, "rtr")) { EMIT(0x4E77); return 1; }
    if (!strcmp(mn, "trapv")) { EMIT(0x4E76); return 1; }
    if (!strcmp(mn, "reset")) { EMIT(0x4E70); return 1; }
    if (!strcmp(mn, "illegal")) { EMIT(0x4AFC); return 1; }

    /* ---- MOVE family ---- */
    if (!strcmp(mn, "move") || !strcmp(mn, "movea")) {
        if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
        if (b.mode == M_SR || b.mode == M_CCR) {
            if (!ea_check(A, &a, CAT_DATA, mn)) return 0;
            EMIT((b.mode == M_SR ? 0x46C0 : 0x44C0) | ea_field(&a));
            return ea_ext(A, &a, addr, w, nw, 2);
        }
        if (a.mode == M_SR) {
            if (!ea_check(A, &b, CAT_DATA_ALT, mn)) return 0;
            EMIT(0x40C0 | ea_field(&b));
            return ea_ext(A, &b, addr, w, nw, 2);
        }
        if (a.mode == M_USP && b.mode == M_AN) { EMIT(0x4E68 | b.reg); return 1; }
        if (b.mode == M_USP && a.mode == M_AN) { EMIT(0x4E60 | a.reg); return 1; }
        if (a.mode > M_IMM || b.mode > M_IMM) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        if (b.mode == M_AN) {
            if (sz == 1) { error_at(A, "MOVEA.B does not exist"); return 0; }
        } else if (!ea_check(A, &b, CAT_DATA_ALT, mn)) return 0;
        if (sz == 1 && a.mode == M_AN) { error_at(A, "MOVE.B from an address register is not allowed"); return 0; }
        unsigned msz = sz == 1 ? 1 : sz == 2 ? 3 : 2;
        { unsigned df = ea_field(&b); EMIT((msz << 12) | ((df & 7) << 9) | ((df >> 3) << 6) | ea_field(&a)); }
        if (!ea_ext(A, &a, addr, w, nw, sz)) return 0;
        return ea_ext(A, &b, addr, w, nw, sz);
    }
    if (!strcmp(mn, "moveq")) {
        if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
        if (a.mode != M_IMM || b.mode != M_DN) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        if (A->emit && !fits(a.imm, -128, 255)) { error_at(A, "MOVEQ value out of range (%ld)", a.imm); return 0; }
        if (A->emit && a.imm > 127) warn_at(A, "MOVEQ #%ld: using a signed operand as unsigned", a.imm);
        EMIT(0x7000 | ((unsigned)b.reg << 9) | ((unsigned)a.imm & 0xff));
        return 1;
    }
    if (!strcmp(mn, "movem")) {
        if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
        if (sz == 1) { error_at(A, "MOVEM.B does not exist"); return 0; }
        unsigned mask; EA *ea; int to_mem;
        if (a.mode == M_REGLIST || a.mode == M_DN || a.mode == M_AN) {
            mask = a.mode == M_REGLIST ? a.mask : 1u << (a.mode == M_AN ? 8 + a.reg : a.reg); ea = &b; to_mem = 1;
        } else if (b.mode == M_REGLIST || b.mode == M_DN || b.mode == M_AN) {
            mask = b.mode == M_REGLIST ? b.mask : 1u << (b.mode == M_AN ? 8 + b.reg : b.reg); ea = &a; to_mem = 0;
        } else { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        if (to_mem) {
            if (!ea_check(A, ea, CAT_CTRL_ALT | B(M_DEC), mn)) return 0;
            if (ea->mode == M_DEC) { unsigned r = 0; for (int i = 0; i < 16; i++) if (mask & (1u << i)) r |= 1u << (15 - i); mask = r; }
            EMIT(0x4880 | ((sz == 4) << 6) | ea_field(ea));
        } else {
            if (!ea_check(A, ea, CAT_CTRL | B(M_INC), mn)) return 0;
            EMIT(0x4C80 | ((sz == 4) << 6) | ea_field(ea));
        }
        EMIT(mask);
        return ea_ext(A, ea, addr, w, nw, sz);
    }
    if (!strcmp(mn, "movep")) {
        if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
        if (a.mode == M_DN && (b.mode == M_D16 || b.mode == M_IND)) {
            EMIT(0x0188 | ((unsigned)a.reg << 9) | ((sz == 4) << 6) | b.reg); EMIT(b.mode == M_IND ? 0 : b.disp); return 1;
        }
        if (b.mode == M_DN && (a.mode == M_D16 || a.mode == M_IND)) {
            EMIT(0x0108 | ((unsigned)b.reg << 9) | ((sz == 4) << 6) | a.reg); EMIT(a.mode == M_IND ? 0 : a.disp); return 1;
        }
        error_at(A, "illegal or missing operand(s) for %s", mn); return 0;
    }
    if (!strcmp(mn, "lea")) {
        if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
        if (b.mode != M_AN || !ea_check(A, &a, CAT_CTRL, mn)) { if (b.mode != M_AN) error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        EMIT(0x41C0 | ((unsigned)b.reg << 9) | ea_field(&a));
        return ea_ext(A, &a, addr, w, nw, 4);
    }
    if (!strcmp(mn, "pea") || !strcmp(mn, "jmp") || !strcmp(mn, "jsr")) {
        if (!parse_one(A, ops, nops, &a, mn) || !ea_check(A, &a, CAT_CTRL, mn)) return 0;
        EMIT((mn[0] == 'p' ? 0x4840 : mn[1] == 'm' ? 0x4EC0 : 0x4E80) | ea_field(&a));
        return ea_ext(A, &a, addr, w, nw, 4);
    }
    if (!strcmp(mn, "exg")) {
        if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
        if (a.mode == M_DN && b.mode == M_DN) { EMIT(0xC140 | ((unsigned)a.reg << 9) | b.reg); return 1; }
        if (a.mode == M_AN && b.mode == M_AN) { EMIT(0xC148 | ((unsigned)a.reg << 9) | b.reg); return 1; }
        if (a.mode == M_DN && b.mode == M_AN) { EMIT(0xC188 | ((unsigned)a.reg << 9) | b.reg); return 1; }
        if (a.mode == M_AN && b.mode == M_DN) { EMIT(0xC188 | ((unsigned)b.reg << 9) | a.reg); return 1; }
        error_at(A, "illegal or missing operand(s) for %s", mn); return 0;
    }
    if (!strcmp(mn, "swap")) {
        if (!parse_one(A, ops, nops, &a, mn) || a.mode != M_DN) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        EMIT(0x4840 | a.reg); return 1;
    }
    if (!strcmp(mn, "ext")) {
        if (!parse_one(A, ops, nops, &a, mn) || a.mode != M_DN) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        EMIT((sz == 4 ? 0x48C0 : 0x4880) | a.reg); return 1;
    }
    if (!strcmp(mn, "link")) {
        if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
        if (a.mode != M_AN || b.mode != M_IMM) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        EMIT(0x4E50 | a.reg); EMIT(b.imm); return 1;
    }
    if (!strcmp(mn, "unlk")) {
        if (!parse_one(A, ops, nops, &a, mn) || a.mode != M_AN) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        EMIT(0x4E58 | a.reg); return 1;
    }
    if (!strcmp(mn, "trap")) {
        if (!parse_one(A, ops, nops, &a, mn) || a.mode != M_IMM) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        EMIT(0x4E40 | ((unsigned)a.imm & 15)); return 1;
    }
    if (!strcmp(mn, "stop")) {
        if (!parse_one(A, ops, nops, &a, mn) || a.mode != M_IMM) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        EMIT(0x4E72); EMIT(a.imm); return 1;
    }
    /* ---- single <ea> instructions ---- */
    {
        static const struct { const char *n; unsigned op; int sized; int dflt; } t[] = {
            {"clr", 0x4200, 1, 2}, {"neg", 0x4400, 1, 2}, {"negx", 0x4000, 1, 2}, {"not", 0x4600, 1, 2}, {"tst", 0x4A00, 1, 2},
            {"nbcd", 0x4800, 0, 1}, {"tas", 0x4AC0, 0, 1}, {NULL, 0, 0, 0} };
        for (int i = 0; t[i].n; i++) if (!strcmp(mn, t[i].n)) {
            if (!parse_one(A, ops, nops, &a, mn) || !ea_check(A, &a, CAT_DATA_ALT, mn)) return 0;
            int s = t[i].sized ? size_from_suffix(suf, t[i].dflt) : t[i].dflt;
            EMIT(t[i].op | (t[i].sized ? (sizebits(s) << 6) : 0) | ea_field(&a));
            return ea_ext(A, &a, addr, w, nw, s);
        }
    }
    /* ---- bit operations ---- */
    {
        static const struct { const char *n; unsigned type; } t[] = { {"btst", 0}, {"bchg", 1}, {"bclr", 2}, {"bset", 3}, {NULL, 0} };
        for (int i = 0; t[i].n; i++) if (!strcmp(mn, t[i].n)) {
            if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
            unsigned cat = (t[i].type == 0) ? (CAT_DATA & ~B(M_IMM)) : CAT_DATA_ALT;
            if (t[i].type == 0 && b.mode == M_IMM && a.mode == M_DN) cat |= B(M_IMM);
            if (!ea_check(A, &b, cat | (t[i].type == 0 ? B(M_IMM) : 0), mn)) return 0;
            if (a.mode == M_IMM) {
                EMIT(0x0800 | (t[i].type << 6) | ea_field(&b)); EMIT(a.imm & 0xff);
            } else if (a.mode == M_DN) {
                EMIT(0x0100 | ((unsigned)a.reg << 9) | (t[i].type << 6) | ea_field(&b));
            } else { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
            return ea_ext(A, &b, addr, w, nw, b.mode == M_DN ? 4 : 1);
        }
    }
    /* ---- shifts and rotates ---- */
    {
        static const struct { const char *n; unsigned type; int dir; } t[] = {
            {"asl", 0, 1}, {"asr", 0, 0}, {"lsl", 1, 1}, {"lsr", 1, 0}, {"roxl", 2, 1}, {"roxr", 2, 0}, {"rol", 3, 1}, {"ror", 3, 0}, {NULL, 0, 0} };
        for (int i = 0; t[i].n; i++) if (!strcmp(mn, t[i].n))
            return enc_shift(A, t[i].type, t[i].dir, sz, ops, nops, addr, w, nw, mn);
    }
    /* ---- multiply / divide / check ---- */
    {
        static const struct { const char *n; unsigned op; } t[] = { {"mulu", 0xC0C0}, {"muls", 0xC1C0}, {"divu", 0x80C0}, {"divs", 0x81C0}, {"chk", 0x4180}, {NULL, 0} };
        for (int i = 0; t[i].n; i++) if (!strcmp(mn, t[i].n)) {
            if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
            if (b.mode != M_DN || !ea_check(A, &a, CAT_DATA, mn)) { if (b.mode != M_DN) error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
            EMIT(t[i].op | ((unsigned)b.reg << 9) | ea_field(&a));
            return ea_ext(A, &a, addr, w, nw, 2);
        }
    }
    /* ---- ADDQ / SUBQ ---- */
    if (!strcmp(mn, "addq") || !strcmp(mn, "subq")) {
        if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
        if (a.mode != M_IMM || !ea_check(A, &b, CAT_ALT, mn)) { if (a.mode != M_IMM) error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
        if (A->emit && !fits(a.imm, 1, 8)) { error_at(A, "%s value must be 1..8", mn); return 0; }
        if (b.mode == M_AN && sz == 1) { error_at(A, "byte size not allowed with an address register (%s)", mn); return 0; }
        EMIT(0x5000 | (((unsigned)a.imm & 7) << 9) | (mn[0] == 's' ? 0x100 : 0) | (sizebits(sz) << 6) | ea_field(&b));
        return ea_ext(A, &b, addr, w, nw, sz);
    }
    /* ---- ADDX / SUBX / ABCD / SBCD ---- */
    if (!strcmp(mn, "addx") || !strcmp(mn, "subx") || !strcmp(mn, "abcd") || !strcmp(mn, "sbcd")) {
        if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
        unsigned op = mn[0] == 'a' ? (mn[1] == 'd' ? 0xD100 : 0xC100) : (mn[1] == 'u' ? 0x9100 : 0x8100);
        int bcd = mn[1] == 'b';
        unsigned szf = bcd ? 0 : (sizebits(sz) << 6);
        if (a.mode == M_DN && b.mode == M_DN) { EMIT(op | ((unsigned)b.reg << 9) | szf | a.reg); return 1; }
        if (a.mode == M_DEC && b.mode == M_DEC) { EMIT(op | ((unsigned)b.reg << 9) | szf | 8 | a.reg); return 1; }
        error_at(A, "illegal or missing operand(s) for %s", mn); return 0;
    }
    /* ---- immediate forms (explicit) ---- */
    {
        static const struct { const char *n; unsigned op; } t[] = { {"ori", 0x0000}, {"andi", 0x0200}, {"subi", 0x0400}, {"addi", 0x0600}, {"eori", 0x0A00}, {"cmpi", 0x0C00}, {NULL, 0} };
        for (int i = 0; t[i].n; i++) if (!strcmp(mn, t[i].n)) {
            if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
            if (a.mode != M_IMM) { error_at(A, "illegal or missing operand(s) for %s", mn); return 0; }
            if (b.mode == M_AN) {   /* MDS accepts ADDI/SUBI/CMPI #x,An and encodes ADDA/SUBA/CMPA */
                if (t[i].op == 0x0600 || t[i].op == 0x0400 || t[i].op == 0x0C00) {
                    warn_at(A, "%s on an address register -> %sA", mn, t[i].op == 0x0600 ? "ADD" : t[i].op == 0x0400 ? "SUB" : "CMP");
                    return enc_arith(A, mn, t[i].op == 0x0600 ? 0xD000 : t[i].op == 0x0400 ? 0x9000 : 0xB000, sz, &a, &b, addr, w, nw);
                }
            }
            return enc_imm_ea(A, t[i].op, sz, &a, &b, addr, w, nw, mn);
        }
    }
    /* ---- ADD SUB AND OR EOR CMP with the MDS immediate rule ---- */
    {
        static const struct { const char *n; unsigned op, iop; } t[] = {
            {"add", 0xD000, 0x0600}, {"sub", 0x9000, 0x0400}, {"and", 0xC000, 0x0200}, {"or", 0x8000, 0x0000},
            {"eor", 0xB100, 0x0A00}, {"cmp", 0xB000, 0x0C00}, {"adda", 0xD000, 0x0600}, {"suba", 0x9000, 0x0400}, {"cmpa", 0xB000, 0x0C00},
            {"cmpm", 0xB000, 0x0C00}, {NULL, 0, 0} };
        for (int i = 0; t[i].n; i++) if (!strcmp(mn, t[i].n)) {
            if (!parse_two(A, ops, nops, &a, &b, mn)) return 0;
            if (a.mode == M_IMM) {
                if ((!strcmp(mn, "add") || !strcmp(mn, "sub")) && fits(a.imm, 1, 8) && a.imm_kind == K_CONST) {
                    /* ADD/SUB #1..8 -> ADDQ/SUBQ (MDS) */
                    if (!ea_check(A, &b, CAT_ALT, mn)) return 0;
                    if (b.mode == M_AN && sz == 1) { error_at(A, "byte size not allowed with an address register (%s)", mn); return 0; }
                    EMIT(0x5000 | (((unsigned)a.imm & 7) << 9) | (t[i].op == 0x9000 ? 0x100 : 0) | (sizebits(sz) << 6) | ea_field(&b));
                    return ea_ext(A, &b, addr, w, nw, sz);
                }
                if (b.mode == M_AN) return enc_arith(A, mn, t[i].op, sz, &a, &b, addr, w, nw);   /* ADDA/SUBA/CMPA #imm,An */
                return enc_imm_ea(A, t[i].iop, sz, &a, &b, addr, w, nw, mn);
            }
            return enc_arith(A, mn, t[i].op, sz, &a, &b, addr, w, nw);
        }
    }
    return -1;
}

/* --------------------------------------------------------------- passes ---- */

static void emit_byte(Asm *A, unsigned v) { if (A->emit) buf_put8(&A->code, v); A->pc++; }
static void emit_word(Asm *A, unsigned v) { emit_byte(A, v >> 8); emit_byte(A, v); }
static void emit_pad(Asm *A) { if (A->pc & 1) emit_byte(A, 0); }

static void clear_pending(Asm *A) { A->npending = 0; }

/* Define a symbol in the current module. kind = SYM_LABEL/SYM_CONST/SYM_DS/SYM_REG. */
static Sym *define_sym(Asm *A, const char *name, int kind, long val) {
    char *ln = scoped_name(A, name);
    Sym *s = sym_for_def(A, ln, A->module, kind);
    free(ln);
    if (s->kind == SYM_MACRO || s->kind == SYM_TRAP) { error_at(A, "'%s' is already a macro or trap name", name); return s; }
    s->def_count++;
    if (s->def_count > 1 && A->emit) {
        if (kind == SYM_CONST || kind == SYM_REG) { if (s->def_count == 2) warn_at(A, "%s defined by EQU more than once, the last definition wins", name); }
        else error_at(A, "symbol '%s' defined more than once", name);
    }
    s->kind = kind; s->val = val; s->defined = 1; s->def_module = A->module;
    if (kind == SYM_LABEL && name[0] != '@') { snprintf(A->scope, sizeof A->scope, "%s", name); str_lower(A->scope); }
    return s;
}

static void add_pending(Asm *A, Sym *s) { if (A->npending < 64) A->pending[A->npending++] = s; }

static void move_pending(Asm *A) { for (int i = 0; i < A->npending; i++) A->pending[i]->val = A->pc; }

/* Bytes of one DC item (string or expression). */
static void dc_item(Asm *A, const char *item, int sz) {
    if (sz == 1 && (item[0] == '\'' || item[0] == '"')) {
        char q = item[0];
        for (const char *p = item + 1; *p; p++) {
            if (*p == q) { if (p[1] == q) { emit_byte(A, (unsigned char)q); p++; continue; } break; }
            emit_byte(A, (unsigned char)*p);
        }
        return;
    }
    Val v;
    if (!eval(A, item, &v)) { error_at(A, "bad expression '%s'", item); return; }
    if (sz == 1) emit_byte(A, v.val);
    else if (sz == 2) emit_word(A, v.val);
    else { emit_word(A, (unsigned)v.val >> 16); emit_word(A, v.val); }
}

/* IF conditions: 'a' = 'b', 'a' <> 'b', expr, expr = expr, expr <> expr, <, >, <=, >=.
 * Undefined symbols count as 0 (an IF on a symbol nobody defined is simply false). */
static int cond_true(Asm *A, const char *args) {
    /* find the comparison operator outside quotes */
    const char *opp = NULL; int oplen = 0; char q = 0;
    for (const char *p = args; *p; p++) {
        if (q) { if (*p == q) q = 0; continue; }
        if (*p == '\'' || *p == '"') { q = *p; continue; }
        if (p[0] == '<' && p[1] == '>') { opp = p; oplen = 2; break; }
        if ((p[0] == '<' || p[0] == '>') && p[1] == '=') { opp = p; oplen = 2; break; }
        if (p[0] == '=' || p[0] == '<' || p[0] == '>') { opp = p; oplen = 1; break; }
    }
    if (!opp) {
        Val v;
        if (!eval_quiet(A, args, &v)) { error_at(A, "bad condition '%s'", args); return 0; }
        return !v.undefined && v.val != 0;
    }
    char *lhs = xstrndup(args, opp - args); char *rhs = xstrdup(opp + oplen);
    char op[3] = {0}; memcpy(op, opp, oplen);
    int result = 0;
    if ((lhs[0] == '\'' || lhs[0] == '"') && (rhs[0] == '\'' || rhs[0] == '"')) {
        size_t ll = strlen(lhs), rl = strlen(rhs);
        if (ll >= 2) { lhs[ll-1] = 0; memmove(lhs, lhs + 1, ll - 1); }
        if (rl >= 2) { rhs[rl-1] = 0; memmove(rhs, rhs + 1, rl - 1); }
        int c = strcmp(lhs, rhs);
        result = !strcmp(op, "=") ? c == 0 : !strcmp(op, "<>") ? c != 0 : !strcmp(op, "<") ? c < 0 : !strcmp(op, ">") ? c > 0 : !strcmp(op, "<=") ? c <= 0 : c >= 0;
    } else {
        Val a, b;
        if (!eval_quiet(A, lhs, &a) || !eval_quiet(A, rhs, &b)) { error_at(A, "bad condition '%s'", args); free(lhs); free(rhs); return 0; }
        long x = a.undefined ? 0 : a.val, y = b.undefined ? 0 : b.val;
        result = !strcmp(op, "=") ? x == y : !strcmp(op, "<>") ? x != y : !strcmp(op, "<") ? x < y : !strcmp(op, ">") ? x > y : !strcmp(op, "<=") ? x <= y : x >= y;
    }
    free(lhs); free(rhs);
    return result;
}

static void process_text(Asm *A, const char *text);

static void expand_macro(Asm *A, Macro *m, const char *args) {
    char *argv[9]; int argc = 0;
    if (*args) argc = split_operands(args, argv, 9);
    if (++A->macro_depth > 32) { error_at(A, "macro nesting too deep"); A->macro_depth--; return; }
    for (int i = 0; i < m->body.n; i++) {
        const char *b = m->body.v[i];
        Buf out = {0};
        for (const char *p = b; *p; p++) {
            if (*p == '%' && p[1] >= '1' && p[1] <= '9') {
                int k = p[1] - '1';
                if (k < argc) buf_put(&out, argv[k], strlen(argv[k]));
                p++;
            } else buf_put8(&out, (unsigned char)*p);
        }
        buf_put8(&out, 0);
        A->cur_is_macro_line = 1;
        process_text(A, (char *)out.d);
        buf_free(&out);
    }
    A->macro_depth--;
    for (int i = 0; i < argc; i++) free(argv[i]);
}

static void process_text(Asm *A, const char *text) {
    char *label, *op, *args, *raw;
    int has = split_line(text, &label, &op, &args, &raw);
    if (!has) { free(label); free(op); free(args); free(raw); return; }
    char *lop = lower_dup(op);

    /* conditionals */
    if (!strcmp(lop, "if")) {
        int active = A->if_depth == 0 || A->if_stack[A->if_depth - 1];
        if (A->if_depth < 64) A->if_stack[A->if_depth++] = active && cond_true(A, args);
        goto done;
    }
    if (!strcmp(lop, "else")) {
        if (A->if_depth) { int parent = A->if_depth == 1 || A->if_stack[A->if_depth - 2]; A->if_stack[A->if_depth - 1] = parent && !A->if_stack[A->if_depth - 1]; }
        goto done;
    }
    if (!strcmp(lop, "endif") || !strcmp(lop, "endc")) { if (A->if_depth) A->if_depth--; goto done; }
    if (A->if_depth && !A->if_stack[A->if_depth - 1]) goto done;

    /* transparent directives */
    if (!strcmp(lop, "xdef") || !strcmp(lop, "xref") || !strcmp(lop, "include") || !strcmp(lop, ".trap") || !strcmp(lop, "offset")) goto done;
    if (!strcmp(lop, "end")) { A->if_depth = 0; goto done; }

    /* mnemonic and size */
    char mn[64]; int suf = 0;
    {
        snprintf(mn, sizeof mn, "%s", lop);
        char *dot = strrchr(mn, '.');
        if (dot && dot != mn && dot[1] && !dot[2]) { suf = dot[1]; *dot = 0; }
    }

    /* EQU / SET */
    if (!strcmp(mn, "equ") || !strcmp(mn, "set") || !strcmp(mn, "equr")) {
        if (!*label) { error_at(A, "%s needs a label", op); goto blocking; }
        int r = reg_by_name(NULL, args);
        if (r >= 0 && r < 16) { Sym *s = define_sym(A, label, SYM_REG, r); s->reg = r; goto blocking; }
        Val v;
        if (!eval(A, args, &v)) { error_at(A, "bad expression '%s'", args); goto blocking; }
        Sym *s = define_sym(A, label, SYM_CONST, v.val);
        s->vkind = v.kind == K_REG ? K_CONST : v.kind;
        goto blocking;
    }
    /* DS */
    if (!strcmp(mn, "ds")) {
        int sz = size_from_suffix(suf, 2);
        Val v;
        if (!eval(A, args, &v)) { error_at(A, "bad DS count '%s'", args); goto done; }
        if (sz > 1 && (A->ds_off & 1)) A->ds_off++;     /* DS.W / DS.L start on an even offset (MDS) */
        if (*label) {
            /* every module has its own block in the A5 area: module 1 right below the 256
             * reserved bytes, module 2 below that, ...; variables ascend inside the block */
            long base = -256;
            for (int m = 0; m <= A->module && m < MAX_MODULES; m++) base -= (long)A->ds_size_prev[m];
            define_sym(A, label, SYM_DS, base + (long)A->ds_off);
        }
        A->ds_off += (unsigned)(v.val * sz);
        goto done;      /* transparent for the alignment rule */
    }
    /* code label on this line */
    if (*label) {
        Sym *s = define_sym(A, label, SYM_LABEL, A->pc);
        add_pending(A, s);
    }
    if (!*op) goto done;    /* label-only line stays transparent */

    /* data */
    if (!strcmp(mn, "dc") || !strcmp(mn, "dcb")) {
        int sz = size_from_suffix(suf, 2);
        if (sz > 1 && (A->pc & 1) && !A->cur_is_macro_line) { emit_byte(A, 0); move_pending(A); }
        char *items[256]; int n = split_operands(args, items, 256);
        if (!strcmp(mn, "dc")) {
            for (int i = 0; i < n; i++) dc_item(A, items[i], sz);
        } else {
            if (n != 2) error_at(A, "DCB needs count,value");
            else {
                Val c; if (!eval(A, items[0], &c)) error_at(A, "bad DCB count '%s'", items[0]);
                else {
                    if (A->emit && c.val > 32767) warn_at(A, "DCB count %ld: the MDS assembler would emit nothing here (16-bit count)", c.val);
                    for (long i = 0; i < c.val; i++) dc_item(A, items[1], sz);
                }
            }
        }
        for (int i = 0; i < n; i++) free(items[i]);
        goto blocking;
    }
    if (!strcmp(mn, "even")) { emit_pad(A); goto blocking; }

    /* trap or macro? */
    {
        char *ln = lower_dup(op);
        Sym *s = sym_lookup(A, ln, A->module);
        free(ln);
        if (s && s->kind == SYM_TRAP) {
            unsigned word = (unsigned)s->val;
            const char *m = args; if (*m == ',') m++;
            if (*m) {
                if (str_ieq(m, "sys") || str_ieq(m, "async") || str_ieq(m, "autopop")) word |= 0x400;
                else if (str_ieq(m, "clear") || str_ieq(m, "immed")) word |= 0x200;
                else error_at(A, "unknown trap modifier '%s'", m);
            }
            if (A->emit && s->rom > 64 && !A->opt->rom128)
                warn_at(A, "%s needs the %dK ROM (Macintosh Plus, 512Ke); MAC68K_ROM=128 accepts it", op, s->rom);
            emit_word(A, word);
            goto blocking;
        }
        if (s && s->kind == SYM_MACRO) { expand_macro(A, s->macro, args); goto blocking; }
    }

    /* instruction */
    {
        char *ops[MAX_OPS]; int n = split_operands(args, ops, MAX_OPS);
        unsigned w[MAX_WORDS]; int nw = 0;
        emit_pad(A);                      /* instructions are word aligned; labels before them stay */
        int r = encode_instr(A, mn, suf, ops, n, A->pc, w, &nw);
        if (r < 0) error_at(A, "unknown instruction or macro '%s'", op);
        else if (r > 0) for (int i = 0; i < nw; i++) emit_word(A, w[i]);
        for (int i = 0; i < n; i++) free(ops[i]);
    }
blocking:
    clear_pending(A);
done:
    free(label); free(op); free(args); free(raw); free(lop);
}

/* The string literals of a module go to its end, each as a word-aligned Pascal string. */
static void emit_literals(Asm *A, int m) {
    if (m < 0 || A->lit[m].n == 0) return;
    emit_pad(A);
    for (int k = 0; k < A->lit[m].n; k++) {
        char *ln = xsprintf("$lit_%d", k);
        Sym *sym = sym_find_exact(A, ln, m);
        if (!sym) sym = sym_create(A, ln, m, SYM_LABEL);
        free(ln);
        sym->kind = SYM_LABEL; sym->val = A->pc; sym->defined = 1; sym->def_module = m;
        const unsigned char *t = (const unsigned char *)A->lit[m].v[k];
        size_t len = strlen((const char *)t);
        emit_byte(A, (unsigned)len);
        for (size_t i = 0; i < len; i++) emit_byte(A, t[i]);
    }
    emit_pad(A);
}

/* One pass over all lines. */
static void run_pass(Asm *A) {
    A->pc = 0; A->ds_off = 0; A->module = -1; A->scope[0] = 0; A->npending = 0; A->if_depth = 0; A->macro_depth = 0;
    for (int h = 0; h < HASH_SIZE; h++)
        for (Sym *s = A->tab[h]; s; s = s->next) { s->prev = s->val; s->def_count = 0; }
    int ended = 0;
    for (int i = 0; i < A->lines.n; i++) {
        const Line *ln = &A->lines.v[i];
        if (ln->module != A->module) {
            emit_literals(A, A->module);
            if (A->module >= 0) { A->ds_size[A->module] = (A->ds_off + 1) & ~1u; A->ds_off = 0; }
            A->module = ln->module; A->scope[0] = 0; ended = 0; A->if_depth = 0;
            if (A->module > 0) emit_pad(A);
        }
        A->cur = ln; A->cur_is_macro_line = 0; A->cur_index = i;
        unsigned start = A->pc, code_start = (unsigned)A->code.n;
        if (!ln->in_macro_def && !ended) {
            char *l, *o, *a, *r;
            int has = split_line(ln->text, &l, &o, &a, &r);
            int is_end = has && !*l && str_ieq(o, "end");
            free(l); free(o); free(a); free(r);
            if (is_end) ended = 1;
            else process_text(A, ln->text);
        }
        if (A->lst) {
            fprintf(A->lst, "%08X ", start);
            size_t nb = A->code.n - code_start;
            for (size_t k = 0; k < 8; k++) {
                if (k < nb) fprintf(A->lst, "%02X", A->code.d[code_start + k]); else fputs("  ", A->lst);
            }
            fprintf(A->lst, " %s%s\n", nb > 8 ? "+" : " ", ln->text);
        }
    }
    emit_literals(A, A->module);
    if (A->module >= 0) { A->ds_size[A->module] = (A->ds_off + 1) & ~1u; A->ds_off = 0; }
    for (int m = 0; m < A->nmodules; m++) A->ds_off += A->ds_size[m];    /* total of all blocks */
    A->cur = NULL;
}

/* ------------------------------------------------------------- loading ---- */

static void load_file(Asm *A, const char *path, int module);

static void load_file(Asm *A, const char *path, int module) {
    char *text = read_text_file(path);
    if (!text) { die("cannot read %s", path); }
    char *dir = path_dirname(path);
    const char *fname = xstrdup(path);
    int lineno = 0;
    for (char *p = text; *p; ) {
        char *e = strchr(p, '\n');
        size_t len = e ? (size_t)(e - p) : strlen(p);
        char *line = xstrndup(p, len);
        lineno++;
        char *l, *o, *a, *r;
        int has = split_line(line, &l, &o, &a, &r);
        if (has && !*l && str_ieq(o, "include")) {
            char *name = str_trim(xstrdup(r));
            size_t n = strlen(name);
            if (n > 1 && (name[0] == '\'' || name[0] == '"') && name[n-1] == name[0]) { memmove(name, name + 1, n - 2); name[n-2] = 0; }
            const char **dirs = xmalloc((A->opt->nincdirs + 1) * sizeof *dirs);
            dirs[0] = dir;
            for (int i = 0; i < A->opt->nincdirs; i++) dirs[i + 1] = A->opt->incdirs[i];
            char *inc = find_file_ci(name, dirs, A->opt->nincdirs + 1, NULL, 0);
            free(dirs);
            if (!inc) die("%s line %d: include file %s not found", path_basename(path), lineno, name);
            int seen = 0;
            for (int i = 0; i < A->included.n; i++) if (!strcmp(A->included.v[i], inc)) seen = 1;
            char *note = xsprintf("; ---- Include %s%s", name, seen ? " (already included)" : "");
            lines_add(&A->lines, note, fname, lineno, module);
            free(note);
            if (!seen) { strlist_add(&A->included, inc); load_file(A, inc, module); }
            free(inc); free(name);
        } else {
            lines_add(&A->lines, line, fname, lineno, module);
        }
        free(l); free(o); free(a); free(r); free(line);
        if (!e) break;
        p = e + 1;
    }
    free(dir); free(text);
}

/* Collect macros, traps, XDEFs and symbol kinds before the passes. */
static void prescan(Asm *A) {
    Macro *cur = NULL;
    /* macros, traps, XDEF */
    for (int i = 0; i < A->lines.n; i++) {
        Line *ln = &A->lines.v[i];
        A->cur = ln; A->module = ln->module;
        char *l, *o, *a, *r;
        int has = split_line(ln->text, &l, &o, &a, &r);
        if (cur) {
            ln->in_macro_def = 1;
            if (has && (str_ieq(o, ".endm") || str_ieq(o, "endm"))) cur = NULL;
            else strlist_add(&cur->body, ln->text);
        } else if (has && (str_ieq(o, ".macro") || str_ieq(o, "macro"))) {
            const char *name = str_ieq(o, ".macro") ? a : l;
            if (!*name) error_at(A, "macro without a name");
            else {
                char *lname = lower_dup(name);
                Sym *s = sym_find_exact(A, lname, -1);
                if (!s) s = sym_create(A, lname, -1, SYM_MACRO);
                s->kind = SYM_MACRO; s->defined = 1;
                s->macro = cur = xcalloc(1, sizeof *cur);
                free(lname);
            }
            ln->in_macro_def = 1;
        } else if (has && str_ieq(o, ".trap")) {
            char *parts[3] = { NULL, NULL, NULL }; int n = 0;
            for (char *t = strtok(r, " \t"); t && n < 3; t = strtok(NULL, " \t")) parts[n++] = t;
            if (n < 2) error_at(A, ".TRAP needs a name and a value");
            else {
                Val v;
                if (!eval(A, parts[1], &v)) error_at(A, "bad trap value '%s'", parts[1]);
                else {
                    char *lname = lower_dup(parts[0]);
                    Sym *s = sym_find_exact(A, lname, -1);
                    if (!s) s = sym_create(A, lname, -1, SYM_TRAP);
                    s->kind = SYM_TRAP; s->val = v.val; s->defined = 1;
                    if (n == 3) {           /* optional ROM field: 64K or 128K */
                        if (str_ieq(parts[2], "64k")) s->rom = 64;
                        else if (str_ieq(parts[2], "128k")) s->rom = 128;
                        else error_at(A, ".TRAP: unknown ROM '%s' (use 64K or 128K)", parts[2]);
                    }
                    free(lname);
                }
            }
        } else if (has && str_ieq(o, "xdef")) {
            char *names[64]; int n = split_operands(a, names, 64);
            for (int k = 0; k < n; k++) { str_lower(names[k]); if (*names[k]) strlist_add(&A->xdef[ln->module], names[k]); free(names[k]); }
        }
        free(l); free(o); free(a); free(r);
    }
    /* symbol kinds */
    A->scope[0] = 0; A->module = -1;
    for (int i = 0; i < A->lines.n; i++) {
        Line *ln = &A->lines.v[i];
        if (ln->in_macro_def) continue;
        if (ln->module != A->module) { A->module = ln->module; A->scope[0] = 0; }
        A->cur = ln;
        char *l, *o, *a, *r;
        int has = split_line(ln->text, &l, &o, &a, &r);
        if (has && *l) {
            char *lop = lower_dup(o);
            char *dot = strrchr(lop, '.'); if (dot && dot != lop && dot[1] && !dot[2]) *dot = 0;
            int kind = SYM_LABEL;
            if (!strcmp(lop, "equ") || !strcmp(lop, "set") || !strcmp(lop, "equr")) {
                int rg = reg_by_name(NULL, a);
                kind = (rg >= 0 && rg < 16) ? SYM_REG : SYM_CONST;
            } else if (!strcmp(lop, "ds")) kind = SYM_DS;
            char *ln2 = scoped_name(A, l);
            Sym *s = sym_for_def(A, ln2, A->module, kind);
            if (kind == SYM_LABEL && l[0] != '@') { snprintf(A->scope, sizeof A->scope, "%s", l); str_lower(A->scope); }
            if (s->kind == SYM_MACRO || s->kind == SYM_TRAP) { /* reported when defined */ }
            else s->kind = kind;
            free(ln2); free(lop);
        }
        free(l); free(o); free(a); free(r);
    }
    A->cur = NULL;
}

/* ------------------------------------------------------------------ API ---- */

int asm_assemble(const char *const *files, int nfiles, const AsmOptions *opt, AsmResult *res) {
    Asm *A = xcalloc(1, sizeof *A);
    A->opt = opt;
    memset(res, 0, sizeof *res);
    if (nfiles > MAX_MODULES) die("too many modules (%d)", nfiles);
    for (int i = 0; i < nfiles; i++) load_file(A, files[i], i);
    A->nmodules = nfiles;
    prescan(A);
    A->branch_long = xcalloc(A->lines.n + 1, 1);
    int iter;
    for (iter = 0; iter < MAX_ITER; iter++) {
        A->emit = 0;
        run_pass(A);
        int changed = A->ds_off != A->ds_total_prev;
        for (int m = 0; m < A->nmodules; m++) { if (A->ds_size[m] != A->ds_size_prev[m]) changed = 1; A->ds_size_prev[m] = A->ds_size[m]; }
        A->ds_total_prev = A->ds_off;
        for (int h = 0; h < HASH_SIZE && !changed; h++)
            for (Sym *s = A->tab[h]; s; s = s->next) if (s->val != s->prev) { changed = 1; break; }
        if (!changed) break;
    }
    if (iter == MAX_ITER) { error_at(A, "assembly does not converge"); }
    A->emit = 1;
    if (opt->listing) { A->lst = fopen(opt->listing, "w"); }
    run_pass(A);
    if (A->lst) {
        fputs("\nSymbols\n", A->lst);
        for (int h = 0; h < HASH_SIZE; h++)
            for (Sym *s = A->tab[h]; s; s = s->next)
                if (s->kind == SYM_LABEL || s->kind == SYM_DS)
                    fprintf(A->lst, "%-32s %s %08lX\n", s->name, s->kind == SYM_DS ? "A5" : "  ", (unsigned long)s->val & 0xffffffffUL);
        fclose(A->lst);
    }
    res->code = A->code.d ? A->code.d : xmalloc(1);
    res->len = A->code.n;
    res->ds_total = A->ds_off;
    res->errors = A->errors;
    return A->errors ? 1 : 0;
}

void asm_result_free(AsmResult *res) { free(res->code); res->code = NULL; res->len = 0; }
