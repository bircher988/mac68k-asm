/* resfork.h - classic Mac resource forks and MacBinary files. */
#ifndef MAC68K_RESFORK_H
#define MAC68K_RESFORK_H
#include "util.h"

typedef struct {
    char type[5];            /* four characters, NUL-terminated */
    int id;
    char *name;              /* NULL if unnamed */
    unsigned attr;
    unsigned char *data;
    size_t len;
} Res;

typedef struct { Res *v; int n, cap; } ResList;

/* Append a resource (data is copied, name may be NULL). */
void reslist_add(ResList *l, const char *type, int id, const unsigned char *data, size_t len, unsigned attr, const char *name);
void reslist_free(ResList *l);

/* Resource fork layout as Apple's RMaker/linker write it: 256-byte header, data,
 * map. Types in order of first appearance, IDs in list order. */
void write_fork(const ResList *l, Buf *out);
/* Parse a raw fork; resources are appended in file order. Returns 0 on success. */
int  read_fork(const unsigned char *d, size_t n, ResList *out);

/* MacBinary (as hcopy -m reads it). Time stamps are "now". */
void write_macbinary(const char *name, const char *type, const char *creator,
                     const unsigned char *data, size_t dlen,
                     const unsigned char *rsrc, size_t rlen, unsigned flags, Buf *out);
typedef struct {
    char name[64], type[5], creator[5];
    const unsigned char *data; size_t dlen;
    const unsigned char *rsrc; size_t rlen;
} MacBinary;
int  read_macbinary(const unsigned char *b, size_t n, MacBinary *out);  /* 0 = ok */

/* Load resources from a .bin (MacBinary) or from a raw resource fork. 0 = ok. */
int  load_resfile(const char *path, ResList *out);

#endif
