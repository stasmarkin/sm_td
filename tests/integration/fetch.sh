#!/bin/sh
# Shallow-clone official qmk_firmware at a given tag or commit into checkouts/<ref>/.
# Usage: sh fetch.sh <tag-or-commit>
set -e

REF="${1:?usage: fetch.sh <tag-or-commit>}"
HERE="$(cd "$(dirname "$0")" && pwd)"
BASE="${SMTD_QMK_DIR:-$HERE/checkouts}"
DEST="$BASE/$REF"
REPO="https://github.com/qmk/qmk_firmware"

if [ -d "$DEST" ]; then
    echo "$DEST already exists, skipping clone"
    exit 0
fi

mkdir -p "$BASE"

# Tags can be cloned shallow directly; arbitrary commits cannot, so fall back.
if git clone --depth 1 --branch "$REF" --recurse-submodules --shallow-submodules "$REPO" "$DEST" 2>/dev/null; then
    echo "cloned tag/branch $REF → $DEST"
else
    echo "ref $REF is not a tag/branch, doing full clone + checkout"
    git clone "$REPO" "$DEST"
    git -C "$DEST" checkout "$REF"
    git -C "$DEST" submodule update --init --recursive
fi
