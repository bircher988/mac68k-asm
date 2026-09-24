/* util.h - small helpers shared by all tools: memory, strings, files, byte buffers. */
#ifndef MAC68K_UTIL_H
#define MAC68K_UTIL_H
#include <stddef.h>
#include <stdint.h>

/* ---- memory ---- */
void *xmalloc(size_t n);
void *xcalloc(size_t n, size_t sz);
void *xrealloc(void *p, size_t n);
char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t n);
char *xsprintf(const char *fmt, ...);

/* ---- diagnostics: print "prog: ..." to stderr; die() exits with status 1 ---- */
extern const char *prog_name;
void die(const char *fmt, ...);
void warnf(const char *fmt, ...);

/* ---- strings ---- */
int  str_ieq(const char *a, const char *b);            /* case-insensitive equality */
int  str_ieq_n(const char *a, const char *b, size_t n);
int  str_istarts(const char *s, const char *prefix);
char *str_upper(char *s);                              /* in place, returns s */
char *str_lower(char *s);
char *str_trim(char *s);                               /* in place: strip leading/trailing blanks */
int  is_blank_line(const char *s);

/* ---- growable string list ---- */
typedef struct { char **v; int n, cap; } StrList;
void strlist_add(StrList *l, const char *s);           /* copies s */
void strlist_free(StrList *l);

/* ---- growable byte buffer ---- */
typedef struct { unsigned char *d; size_t n, cap; } Buf;
void buf_reserve(Buf *b, size_t extra);
void buf_put8(Buf *b, unsigned v);
void buf_put16be(Buf *b, unsigned v);
void buf_put32be(Buf *b, uint32_t v);
void buf_put(Buf *b, const void *p, size_t n);
void buf_pad(Buf *b, size_t align);                    /* zero-fill to a multiple of align */
void buf_free(Buf *b);
unsigned get16be(const unsigned char *p);
uint32_t get32be(const unsigned char *p);
void put16be(unsigned char *p, unsigned v);
void put32be(unsigned char *p, uint32_t v);

/* ---- files ----
 * Source files are Latin-1 text with CR, LF or CRLF line ends. read_text_file
 * returns a NUL-terminated copy with LF line ends only, or NULL if unreadable. */
char *read_text_file(const char *path);
int   read_binary_file(const char *path, unsigned char **data, size_t *len);   /* 0 = ok */
int   write_binary_file(const char *path, const unsigned char *data, size_t len);
int   file_exists(const char *path);
int   dir_exists(const char *path);
int   make_dirs(const char *path);                     /* mkdir -p */
char *path_join(const char *dir, const char *name);    /* malloc'd */
char *path_dirname(const char *path);                  /* malloc'd; "." if none */
const char *path_basename(const char *path);
char *path_strip_ext(const char *name);                /* malloc'd copy without the last ".ext" */
char *exe_dir(void);                                   /* directory of the running executable, malloc'd or NULL */

/* Find a file by name in dirs, matching case-insensitively (as HFS does) and
 * trying each extension in exts (use "" for none). Returns a malloc'd path or NULL. */
char *find_file_ci(const char *name, const char *const *dirs, int ndirs, const char *const *exts, int nexts);

#endif
