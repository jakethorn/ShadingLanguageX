#!/usr/bin/env bash
#
# Copies the shared browser files from browser_shared/ into the two browser integrations, which keep their own
# copies so each served folder is self-contained (see browser_shared/README.md):
#
#   browser_shared/index.html   -> python/browser/templates/index.html, javascript/browser/index.html
#   browser_shared/common.js    -> python/browser/static/common.js,     javascript/browser/common.js
#   browser_shared/keywords.js  -> python/browser/static/keywords.js,   javascript/browser/keywords.js
#
# The copies are identical, except that javascript/browser/index.html loads "app.js" where the others load
# "static/app.js", because the WASM app serves its files from a different root.
#
# Usage (works from any directory):
#   ./sync_browser.sh            copy the shared files into both integrations
#   ./sync_browser.sh --check    only report copies that differ, and exit 1 if any do
#
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SHARED_DIR="$ROOT_DIR/browser_shared"
FLASK_SCRIPT='src="static/app.js"'
WASM_SCRIPT='src="app.js"'

CHECK=false
case "${1:-}" in
    "") ;;
    --check) CHECK=true ;;
    *) echo "Usage: $0 [--check]" >&2; exit 2 ;;
esac

if ! grep -q "$FLASK_SCRIPT" "$SHARED_DIR/index.html"; then
    echo "Error: browser_shared/index.html doesn't load $FLASK_SCRIPT, so the WASM copy can't be made." >&2
    exit 1
fi

EXPECTED_FILE="$(mktemp)"
trap 'rm -f "$EXPECTED_FILE"' EXIT

DRIFTED=0

# sync_file SHARED_NAME DESTINATION [WASM]
# Writes (or with --check, compares) the expected content of DESTINATION, made from browser_shared/SHARED_NAME.
sync_file() {
    local source="$SHARED_DIR/$1"
    local destination="$ROOT_DIR/$2"
    if [ "${3:-}" = "wasm" ]; then
        sed "s#$FLASK_SCRIPT#$WASM_SCRIPT#" "$source" > "$EXPECTED_FILE"
    else
        cp "$source" "$EXPECTED_FILE"
    fi

    if [ -f "$destination" ] && cmp -s "$EXPECTED_FILE" "$destination"; then
        return
    fi
    if [ "$CHECK" = true ]; then
        echo "Out of sync: $2"
        DRIFTED=1
    else
        mkdir -p "$(dirname "$destination")"
        cp "$EXPECTED_FILE" "$destination"
        echo "Updated: $2"
    fi
}

sync_file index.html  python/browser/templates/index.html
sync_file common.js   python/browser/static/common.js
sync_file keywords.js python/browser/static/keywords.js
sync_file index.html  javascript/browser/index.html wasm
sync_file common.js   javascript/browser/common.js
sync_file keywords.js javascript/browser/keywords.js

if [ "$CHECK" = true ]; then
    if [ "$DRIFTED" -ne 0 ]; then
        echo "Run mxslc++/sync_browser.sh to copy browser_shared/ into the integrations." >&2
        exit 1
    fi
    echo "Browser copies are in sync."
else
    echo "Browser copies are in sync with browser_shared/."
fi
