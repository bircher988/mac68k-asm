/* resfork.c - classic Mac resource forks and MacBinary files. */
#include "resfork.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAC_EPOCH 2082844800u   /* seconds from 1904-01-01 to 1970-01-01 */

/* buf_put that tolerates an empty source with a NULL pointer */
static void put_bytes(Buf *b, const void *p, size_t n) { if (n) buf_put(b, p, n); }

void reslist_add(ResList *l, const char *type, int id, const unsigned char *data, size_t len, unsigned attr, const char *name) {
    if (l->n == l->cap) { l->cap = l->cap ? l->cap * 2 : 16; l->v = xrealloc(l->v, l->cap * sizeof *l->v); }
    Res *r = &l->v[l->n++];
    memset(r->type, ' ', 4); r->type[4] = 0;
    size_t tl = strlen(type); if (tl > 4) tl = 4;
    memcpy(r->type, type, tl);
    r->id = id;
    r->name = name ? xstrdup(name) : NULL;
    r->attr = attr & 0xff;
    r->data = xmalloc(len ? len : 1);
    if (len) memcpy(r->data, data, len);
    r->len = len;
}

void reslist_free(ResList *l) {
    for (int i = 0; i < l->n; i++) { free(l->v[i].name); free(l->v[i].data); }
    free(l->v); l->v = NULL; l->n = l->cap = 0;
}

/* Resource fork layout as RMaker and the MDS linker write it: 256-byte header, data,
 * map. Types in order of first appearance, IDs in list order. */
void write_fork(const ResList *l, Buf *out) {
    int n = l->n;
    /* group the resources by type in order of first appearance */
    int ntypes = 0;
    const char **types = xcalloc(n ? n : 1, sizeof *types);
    int *tindex = xcalloc(n ? n : 1, sizeof *tindex);     /* resource -> type index */
    for (int i = 0; i < n; i++) {
        int t;
        for (t = 0; t < ntypes; t++) if (!memcmp(types[t], l->v[i].type, 4)) break;
        if (t == ntypes) types[ntypes++] = l->v[i].type;
        tindex[i] = t;
    }
    /* data area */
    Buf data = {0};
    uint32_t *offsets = xcalloc(n ? n : 1, sizeof *offsets);
    for (int i = 0; i < n; i++) {
        offsets[i] = (uint32_t)data.n;
        buf_put32be(&data, (uint32_t)l->v[i].len);
        put_bytes(&data, l->v[i].data, l->v[i].len);
    }
    /* type list, reference lists and name list */
    Buf tl = {0}, refs = {0}, names = {0};
    buf_put16be(&tl, (unsigned)(ntypes - 1) & 0xffff);
    size_t refbase = 2 + 8 * (size_t)ntypes;
    for (int t = 0; t < ntypes; t++) {
        int cnt = 0;
        for (int i = 0; i < n; i++) if (tindex[i] == t) cnt++;
        buf_put(&tl, types[t], 4);
        buf_put16be(&tl, (unsigned)(cnt - 1) & 0xffff);
        buf_put16be(&tl, (unsigned)(refbase + refs.n) & 0xffff);
        for (int i = 0; i < n; i++) {
            if (tindex[i] != t) continue;
            const Res *r = &l->v[i];
            int noff = -1;
            if (r->name) {
                noff = (int)names.n;
                size_t nl = strlen(r->name); if (nl > 255) nl = 255;
                buf_put8(&names, (unsigned)nl);
                put_bytes(&names, r->name, nl);
            }
            buf_put16be(&refs, (unsigned)r->id & 0xffff);
            buf_put16be(&refs, (unsigned)noff & 0xffff);
            buf_put32be(&refs, ((uint32_t)(r->attr & 0xff) << 24) | (offsets[i] & 0xffffff));
            buf_put32be(&refs, 0);                       /* handle */
        }
    }
    size_t typelist = tl.n + refs.n;
    size_t maplen = 28 + typelist + names.n;
    /* header */
    buf_put32be(out, 256);
    buf_put32be(out, (uint32_t)(256 + data.n));
    buf_put32be(out, (uint32_t)data.n);
    buf_put32be(out, (uint32_t)maplen);
    for (int i = 0; i < 240; i++) buf_put8(out, 0);
    put_bytes(out, data.d, data.n);
    /* map: copy of the header, handle, file ref, attrs, offsets to type/name lists */
    buf_put32be(out, 256);
    buf_put32be(out, (uint32_t)(256 + data.n));
    buf_put32be(out, (uint32_t)data.n);
    buf_put32be(out, (uint32_t)maplen);
    buf_put32be(out, 0);
    buf_put16be(out, 0);
    buf_put16be(out, 0);
    buf_put16be(out, 28);
    buf_put16be(out, (unsigned)(28 + typelist) & 0xffff);
    put_bytes(out, tl.d, tl.n);
    put_bytes(out, refs.d, refs.n);
    put_bytes(out, names.d, names.n);
    buf_free(&data); buf_free(&tl); buf_free(&refs); buf_free(&names);
    free(types); free(tindex); free(offsets);
}

/* Parse a raw fork; resources are appended in file order. Returns 0 on success. */
int read_fork(const unsigned char *d, size_t n, ResList *out) {
    if (n < 16) return 0;                              /* an empty fork has no resources */
    uint32_t doff = get32be(d), moff = get32be(d + 4), mlen = get32be(d + 12);
    if (moff > n || mlen > n - moff || mlen < 30) return -1;
    const unsigned char *m = d + moff;
    unsigned tlo = get16be(m + 24), nlo = get16be(m + 26);
    if (tlo + 2 > mlen) return -1;
    unsigned ntypes = (get16be(m + tlo) + 1) & 0xffff;
    for (unsigned i = 0; i < ntypes; i++) {
        size_t te = tlo + 2 + i * 8;
        if (te + 8 > mlen) return -1;
        char type[5]; memcpy(type, m + te, 4); type[4] = 0;
        unsigned cnt = get16be(m + te + 4) + 1, off = get16be(m + te + 6);
        for (unsigned j = 0; j < cnt; j++) {
            size_t re = tlo + off + j * 12;
            if (re + 12 > mlen) return -1;
            int rid = (int)(int16_t)get16be(m + re);
            int noff = (int)(int16_t)get16be(m + re + 2);
            uint32_t ao = get32be(m + re + 4);
            uint32_t dpos = (uint32_t)doff + (ao & 0xffffff);
            if (dpos + 4 > n) return -1;
            uint32_t len = get32be(d + dpos);
            if (len > n - dpos - 4) return -1;
            char *name = NULL;
            if (noff != -1) {
                size_t np = (size_t)nlo + (size_t)noff;
                if (np >= mlen) return -1;
                unsigned L = m[np];
                if (np + 1 + L > mlen) return -1;
                name = xstrndup((const char *)m + np + 1, L);
            }
            reslist_add(out, type, rid, d + dpos + 4, len, ao >> 24, name);
            free(name);
        }
    }
    return 0;
}

static unsigned crc16_xmodem(const unsigned char *b, size_t n) {
    unsigned crc = 0;
    for (size_t i = 0; i < n; i++) {
        crc ^= (unsigned)b[i] << 8;
        for (int k = 0; k < 8; k++)
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) & 0xffff : (crc << 1) & 0xffff;
    }
    return crc;
}

static void put_padded(Buf *out, const unsigned char *p, size_t n) {
    put_bytes(out, p, n);
    for (size_t k = n % 128; k && k < 128; k++) buf_put8(out, 0);
}

static void put_fourcc(unsigned char *dst, const char *s) {
    memset(dst, ' ', 4);
    size_t n = strlen(s); if (n > 4) n = 4;
    memcpy(dst, s, n);
}

/* MacBinary (as hcopy -m reads it). Time stamps are "now". */
void write_macbinary(const char *name, const char *type, const char *creator,
                     const unsigned char *data, size_t dlen,
                     const unsigned char *rsrc, size_t rlen, unsigned flags, Buf *out) {
    unsigned char h[128];
    memset(h, 0, sizeof h);
    size_t nl = strlen(name); if (nl > 63) nl = 63;
    h[1] = (unsigned char)nl; memcpy(h + 2, name, nl);
    put_fourcc(h + 65, type);
    put_fourcc(h + 69, creator);
    h[73] = (unsigned char)((flags >> 8) & 0xff);
    put32be(h + 83, (uint32_t)dlen);
    put32be(h + 87, (uint32_t)rlen);
    uint32_t now = (uint32_t)((uint64_t)time(NULL) + MAC_EPOCH);
    put32be(h + 91, now);
    put32be(h + 95, now);
    h[101] = (unsigned char)(flags & 0xff);
    h[122] = 129; h[123] = 129;
    put16be(h + 124, crc16_xmodem(h, 124));
    buf_put(out, h, 128);
    put_padded(out, data, dlen);
    put_padded(out, rsrc, rlen);
}

int read_macbinary(const unsigned char *b, size_t n, MacBinary *out) {
    if (n < 128) return -1;
    unsigned nl = b[1]; if (nl > 63) nl = 63;
    memcpy(out->name, b + 2, nl); out->name[nl] = 0;
    memcpy(out->type, b + 65, 4); out->type[4] = 0;
    memcpy(out->creator, b + 69, 4); out->creator[4] = 0;
    uint32_t dl = get32be(b + 83), rl = get32be(b + 87);
    size_t doff = 128, roff = doff + ((size_t)dl + 127) / 128 * 128;
    if (dl > n - doff) return -1;
    if (roff > n || rl > n - roff) return -1;
    out->data = b + doff; out->dlen = dl;
    out->rsrc = b + roff; out->rlen = rl;
    return 0;
}

/* Load resources from a .bin (MacBinary) or from a raw resource fork. 0 = ok. */
int load_resfile(const char *path, ResList *out) {
    unsigned char *d; size_t n;
    if (read_binary_file(path, &d, &n)) return -1;
    int r;
    size_t pl = strlen(path);
    if (pl >= 4 && !strcmp(path + pl - 4, ".bin")) {
        MacBinary mb;
        r = read_macbinary(d, n, &mb);
        if (!r) r = read_fork(mb.rsrc, mb.rlen, out);
    } else {
        r = read_fork(d, n, out);
    }
    free(d);
    return r;
}
