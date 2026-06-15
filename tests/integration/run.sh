#!/bin/sh
# Fetch QMK (if needed), symlink the sm_td overlay + sources into its tests/ dir,
# then build & run the QMK-native sm_td suite(s).
# Usage: sh run.sh [<tag-or-commit>] [<suite> ...]
set -e

HERE="$(cd "$(dirname "$0")" && pwd)"
REF="${1:-0.33.5}"
shift 2>/dev/null || true
SUITES="${*:-smtd_qmk_taphold}"
BASE="${SMTD_QMK_DIR:-$HERE/checkouts}"

sh "$HERE/fetch.sh" "$REF"
QMK="$BASE/$REF"
SMTD_SRC="$HERE/../../sm_td"

# The smtd_via suite (Option B) needs sm_td's keycode resolution to reach the real
# VIA/Vial dynamic-keymap chain. Two idempotent, upstream-safe guards on the
# gitignored checkout make that possible without forking QMK; both are inert for
# every other suite:
#   1) wrap the harness's own keymap_key_to_keycode override in
#      #ifndef SMTD_USE_DYNAMIC_KEYMAP, so a suite can opt out of it and supply its
#      own (dynamic-keymap-backed) override instead.
#   2) add an #ifndef guard around the test-harness TOTAL_EEPROM_BYTE_COUNT (default
#      32, far too small for a real dynamic keymap) so a suite can raise it.
patch_checkout() {
    tf="$QMK/tests/test_common/test_fixture.cpp"
    if [ -f "$tf" ] && ! grep -q SMTD_USE_DYNAMIC_KEYMAP "$tf"; then
        perl -0pi -e 's/(extern "C" uint16_t keymap_key_to_keycode\(.*?\n\})/#ifndef SMTD_USE_DYNAMIC_KEYMAP\n$1\n#endif/s' "$tf"
    fi
    ee="$QMK/platforms/eeprom.h"
    if [ -f "$ee" ] && ! grep -q 'ifndef TOTAL_EEPROM_BYTE_COUNT' "$ee"; then
        perl -0pi -e 's/#(\s*)define TOTAL_EEPROM_BYTE_COUNT 32/#$1ifndef TOTAL_EEPROM_BYTE_COUNT\n#$1    define TOTAL_EEPROM_BYTE_COUNT 32\n#$1endif/' "$ee"
    fi
}
patch_checkout

for suite in $SUITES; do
    overlay="$HERE/suites/$suite"
    [ -d "$overlay" ] || { echo "no overlay for suite '$suite'"; exit 1; }
    # The suite dir must be a REAL directory: QMK discovers tests via
    # `find tests -type f -name test.mk`, which does not descend into symlinked
    # directories nor match symlinked files. Copy the overlay in; symlink only
    # the sm_td sources (referenced by make/VPATH, not find).
    dest="$QMK/tests/$suite"
    mkdir -p "$dest"
    cp "$overlay"/test.mk "$overlay"/config.h "$overlay"/*.c "$overlay"/*.cpp "$dest"/
    # sm_td engine sources (used by sm_td.c suites via smtd_src).
    ln -sfn "$SMTD_SRC" "$dest/smtd_src"
    ln -sfn "$SMTD_SRC" "$dest/sm_td"
done

for suite in $SUITES; do
    echo "=== make test:$suite (qmk $REF) ==="
    make -C "$QMK" "test:$suite"
done
