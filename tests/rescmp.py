#!/usr/bin/env python3
"""rescmp.py A B - compare two MacBinary files by content: file type and creator, then the
resource forks as sets of (type, id, name, attributes, data), and the Finder's hasBundle flag. The MacBinary header's time stamps
and CRC and the resource map's handle fields are ignored. Exit 0 if equal, 1 with a report if not.
Used by tests/run.sh; needs only the Python standard library."""
import struct, sys

def macbinary(b):
    n = b[1]; name = b[2:2 + n].decode('latin-1')
    dl = struct.unpack('>I', b[83:87])[0]; rl = struct.unpack('>I', b[87:91])[0]
    doff = 128; roff = doff + ((dl + 127) // 128) * 128
    return name, b[65:69], b[69:73], b[doff:doff + dl], b[roff:roff + rl]

def resources(d):
    if len(d) < 16: return {}
    do, mo, dl, ml = struct.unpack('>IIII', d[:16])
    m = d[mo:mo + ml]
    tlo, nlo = struct.unpack('>HH', m[24:28])
    n = (struct.unpack('>H', m[tlo:tlo + 2])[0] + 1) & 0xffff
    out = {}
    for i in range(n):
        t, cnt, off = struct.unpack('>4sHH', m[tlo + 2 + i * 8:tlo + 10 + i * 8])
        for j in range(cnt + 1):
            rid, noff, ao = struct.unpack('>hhI', m[tlo + off + j * 12:tlo + off + j * 12 + 8])
            doff = ao & 0xffffff
            ln = struct.unpack('>I', d[do + doff:do + doff + 4])[0]
            name = None
            if noff != -1:
                L = m[nlo + noff]; name = m[nlo + noff + 1:nlo + noff + 1 + L].decode('latin-1')
            out[(t.decode('latin-1'), rid)] = (name, ao >> 24, d[do + doff + 4:do + doff + 4 + ln])
    return out

def ditl_mask(d):
    """Zero the pad byte after each odd-length item text: RMaker leaves memory garbage there."""
    try:
        d = bytearray(d); n = struct.unpack('>h', d[:2])[0] + 1; pos = 2
        for _ in range(n):
            ln = d[pos + 13]; end = pos + 14 + ln
            if ln & 1: d[end] = 0
            pos = end + (ln & 1)
        return bytes(d)
    except (IndexError, struct.error):
        return bytes(d)

def main():
    a, b = sys.argv[1:3]
    ba, bb = open(a, 'rb').read(), open(b, 'rb').read()
    A = macbinary(ba); B = macbinary(bb)
    bad = []
    if A[1] != B[1] or A[2] != B[2]: bad.append(f'type/creator {A[1]}{A[2]} vs {B[1]}{B[2]}')
    if (ba[73] ^ bb[73]) & 0x20: bad.append(f'Finder flag hasBundle {ba[73] >> 5 & 1} vs {bb[73] >> 5 & 1}')
    if A[3] != B[3]: bad.append('data forks differ')
    ra, rb = resources(A[4]), resources(B[4])
    for k in sorted(set(ra) | set(rb)):
        if k not in ra: bad.append(f'{k[0]} {k[1]}: only in {b}'); continue
        if k not in rb: bad.append(f'{k[0]} {k[1]}: only in {a}'); continue
        (na, aa, da), (nb, ab, db) = ra[k], rb[k]
        if na != nb: bad.append(f'{k[0]} {k[1]}: name {na!r} vs {nb!r}')
        if aa != ab: bad.append(f'{k[0]} {k[1]}: attributes {aa:#x} vs {ab:#x}')
        if da != db and k[0] == 'DITL': da, db = ditl_mask(da), ditl_mask(db)
        if da != db:
            off = next((i for i in range(min(len(da), len(db))) if da[i] != db[i]), min(len(da), len(db)))
            bad.append(f'{k[0]} {k[1]}: data differs at offset {off:#x} (sizes {len(da)} vs {len(db)})')
    if bad:
        print(f'{a} vs {b}:'); [print('  ' + x) for x in bad]
        return 1
    return 0

if __name__ == '__main__':
    sys.exit(main())
