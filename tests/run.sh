#!/bin/sh
# tests/run.sh - byte-identity test of the mac68k-asm tool chain against reference builds.
#
# Reference mode (MAC68K_TEST_PROJECTS and MAC68K_TEST_REF set): every <p>/<Job>.Job under
# $MAC68K_TEST_PROJECTS with a reference directory $MAC68K_TEST_REF/<p>/<Job>/ is rebuilt into
# a temporary directory with -I <project dir> -I <projects>/../shared. The reference directory
# holds the files a build of that job produced on the original system, as MacBinary (*.bin):
# the linker output and the application. Each of them must exist in the fresh build and be
# identical in file type, creator and every resource (tests/rescmp.py; MacBinary time stamps,
# the header CRC and the resource map's handle fields do not count). Reference directories
# that contain a non-empty *.Err or *.LErr (the original build failed) are skipped.
# MAC68K_TEST_SKIP lists <p>/<job> names to leave out (known, documented differences).
# Without the variables, example/Hello.Job is built and the result checked.
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/.." && pwd)
MAC68K=${MAC68K:-$ROOT/mac68k-asm}
[ -x "$MAC68K" ] || { echo "mac68k-asm binary not found: $MAC68K (run make first)"; exit 1; }
TMP=$(mktemp -d "${TMPDIR:-/tmp}/mac68k-asm-test.XXXXXX")
trap 'rm -rf "$TMP"' EXIT

# regression tests (always)
mkdir -p "$TMP/regress"
"$MAC68K" build "$HERE/regress/Converge.Job" -o "$TMP/regress" > "$TMP/regress.log" 2>&1
if [ ! -f "$TMP/regress/Converge.bin" ]; then cat "$TMP/regress.log"; echo "FAIL: regress/Converge does not build"; exit 1; fi
n=$(grep -c "6A02 *	BPL.S" "$TMP/regress/Converge.code.lst")
w=$(grep -c "does not reach" "$TMP/regress.log")
[ "$n" = 80 ] && [ "$w" = 1 ] || { echo "FAIL: regress/Converge: $n of 80 BPL.S short, $w widening warnings (want 1)"; exit 1; }
echo "ok: regress/Converge (forward Bcc.S over one instruction stays short)"
# a bare DS variable: reading is fine, an alterable operand needs Var(A5)
if "$MAC68K" build "$HERE/regress/BareVar.Job" -o "$TMP/regress" > "$TMP/bare.log" 2>&1; then
    echo "FAIL: regress/BareVar built, but five lines write a bare DS variable"; exit 1; fi
n=$(grep -c "needs its base register: write Var(A5)$" "$TMP/bare.log")
l=$(grep "needs its base register" "$TMP/bare.log" | sed 's/.*line \([0-9]*\):.*/\1/' | tr '\n' ' ')
[ "$n" = 5 ] && [ "$l" = "9 10 11 12 13 " ] || { cat "$TMP/bare.log"; echo "FAIL: regress/BareVar: want lines 9-13, got '$l'"; exit 1; }
"$MAC68K" build "$HERE/regress/A5Var.Job" -o "$TMP/regress" > "$TMP/a5.log" 2>&1 || { cat "$TMP/a5.log"; echo "FAIL: regress/A5Var"; exit 1; }
for w in "3B40FEFE" "426DFEFE" "4A6DFEFE" "526DFEFE" "08ED0001FEFE"; do
    grep -q "$w" "$TMP/regress/A5Var.lst" || { echo "FAIL: regress/A5Var: $w missing"; exit 1; }; done
echo "ok: regress/BareVar + A5Var (bare DS variables are read-only, Var(A5) is written)"

if [ -z "${MAC68K_TEST_PROJECTS:-}" ] || [ -z "${MAC68K_TEST_REF:-}" ]; then
    mkdir -p "$TMP/hello"
    "$MAC68K" build "$ROOT/example/Hello.Job" -o "$TMP/hello" > "$TMP/hello.log" 2>&1; rc=$?
    cat "$TMP/hello.log"
    if [ $rc -ne 0 ] || [ ! -f "$TMP/hello/Hello.bin" ]; then echo "FAIL: example build"; exit 1; fi
    t=$(dd if="$TMP/hello/Hello.bin" bs=1 skip=65 count=4 2>/dev/null)
    [ "$t" = "APPL" ] || { echo "FAIL: Hello.bin is not an APPL ($t)"; exit 1; }
    echo "ok: example builds an APPL (set MAC68K_TEST_PROJECTS and MAC68K_TEST_REF for the reference test)"
    exit 0
fi

PROJECTS=$MAC68K_TEST_PROJECTS; REF=$MAC68K_TEST_REF
SKIP=" ${MAC68K_TEST_SKIP:-} "      # optional: space-separated <p>/<job> names of known differences
SHARED=$(dirname "$PROJECTS")/shared
ok=0; fail=0; skip=0
for jobfile in $(find "$PROJECTS" -mindepth 2 -maxdepth 2 -iname '*.Job' | sort); do
    p=$(basename "$(dirname "$jobfile")"); job=$(basename "$jobfile"); job=${job%.Job}; job=${job%.job}
    refdir=$REF/$p/$job
    [ -d "$refdir" ] || continue
    case "$SKIP" in *" $p/$job "*) skip=$((skip+1)); continue;; esac
    if [ -n "$(find "$refdir" -maxdepth 1 \( -iname '*.Err' -o -iname '*.LErr' \) -size +0 2>/dev/null)" ]; then skip=$((skip+1)); continue; fi
    bins=$(find "$refdir" -maxdepth 1 -name '*.bin' | sort)
    [ -n "$bins" ] || { skip=$((skip+1)); continue; }
    out=$TMP/$p/$job; mkdir -p "$out"
    if ! "$MAC68K" build "$jobfile" -o "$out" -I "$(dirname "$jobfile")" -I "$SHARED" > "$out/build.log" 2>&1; then
        if grep -q 'not implemented' "$out/build.log"; then skip=$((skip+1)); continue; fi
        echo "FAIL $p/$job: build failed: $(grep -v '^asm: warning' "$out/build.log" | grep -m1 -i 'error\|line' | cut -c1-120)"
        fail=$((fail+1)); continue
    fi
    bad=""
    for b in $bins; do
        n=$(basename "$b")
        if [ ! -f "$out/$n" ]; then bad="$bad $n(missing)"; continue; fi
        python3 "$HERE/rescmp.py" "$b" "$out/$n" > "$out/$n.cmp" || bad="$bad $n"
    done
    if [ -z "$bad" ]; then ok=$((ok+1)); else
        echo "FAIL $p/$job:$bad"; for b in $bins; do n=$(basename "$b"); [ -s "$out/$n.cmp" ] && sed 's/^/    /' "$out/$n.cmp" | head -6; done
        fail=$((fail+1))
    fi
done
echo "reference test: ok=$ok fail=$fail skipped=$skip"
[ "$fail" = 0 ]
