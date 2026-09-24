#!/bin/bash
# build.sh: convenience wrapper around `mac68k-asm build` for a repository layout with one
# folder per project: <repo>/projects/<project>/ and shared sources in <repo>/shared/.
#
#   build.sh <project> <job> [--disk]
#
# Expects to live in <repo>/tools/native (or set REPO). Output goes to
# <repo>/out-native/<project>/ (override with OUT=...). --disk additionally writes a
# copy of the development disk image (DEV_DISK, default <repo>/images/dev2.dsk) with
# the application in folder DISK_FOLDER (default aProject) - needs hfsutils.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="${REPO:-$(cd "$HERE/../.." && pwd)}"
MAC68K="${MAC68K:-$HERE/mac68k-asm}"
[ -x "$MAC68K" ] || MAC68K=$(command -v mac68k-asm || true)
[ -n "$MAC68K" ] && [ -x "$MAC68K" ] || { echo "mac68k-asm not found - run make in $HERE first."; exit 1; }
PROJECT="${1:?project name required, e.g. myapp}"
JOB="${2:?job name required, e.g. MyApp}"
DISK="${3:-}"
PROJECT_DIR="$REPO/projects/$PROJECT"
OUT="${OUT:-$REPO/out-native/$PROJECT}"
DEV_DISK="${DEV_DISK:-$REPO/images/dev2.dsk}"
DISK_FOLDER="${DISK_FOLDER:-aProject}"
[ -d "$PROJECT_DIR" ] || { echo "project '$PROJECT' not found."; exit 1; }
JOBFILE=$(find "$PROJECT_DIR" -maxdepth 1 -iname "$JOB.Job" | head -1)
[ -n "$JOBFILE" ] || { echo "$JOB.Job not found in $PROJECT_DIR"; exit 1; }
mkdir -p "$OUT"

"$MAC68K" build "$JOBFILE" -o "$OUT" -I "$PROJECT_DIR" -I "$REPO/shared"

if [ "$DISK" = "--disk" ]; then
  APP=""
  for b in "$OUT"/*.bin; do
    t=$(dd if="$b" bs=1 skip=65 count=4 2>/dev/null)
    [ "$t" = "APPL" ] && APP="$b"
  done
  [ -n "$APP" ] || { echo "no APPL produced, no disk written."; exit 1; }
  DSK="$OUT/build.dsk"; cp "$DEV_DISK" "$DSK"
  name=$(basename "$APP" .bin)
  hmount "$DSK" >/dev/null; hcd ":$DISK_FOLDER"; hmkdir ":$PROJECT" 2>/dev/null || true; hcd ":$PROJECT"
  hdel ":$name" 2>/dev/null || true; hcopy -m "$APP" ":$name"; humount
  echo "image with app: $DSK  (:$DISK_FOLDER:$PROJECT:$name)"
fi
