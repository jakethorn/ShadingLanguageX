#!/bin/bash
#
# Roundtrips a .mxsl file through mxslc: compiles it to .mtlx (without graph
# reduction, matching the roundtrip tests), then decompiles
# that .mtlx back to .mxsl. Both outputs are written next to the input file:
#
#   foo.mxsl -> foo_roundtrip.mtlx -> foo_roundtrip.mxsl
#
# Usage: roundtrip.sh [debug|release] <input.mxsl>
#   debug|release  Which cmake-build-* directory to take mxslc from (default: debug)

set -e

usage() {
    echo "Usage: $(basename "$0") [debug|release] <input.mxsl>" >&2
    exit 1
}

config=debug
if [[ "$1" == "debug" || "$1" == "release" ]]; then
    config=$1
    shift
fi

[[ $# -eq 1 ]] || usage

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
mxslc="$script_dir/../cmake-build-$config/mxslc"

if [[ ! -x "$mxslc" ]]; then
    echo "Error: mxslc executable not found: $mxslc" >&2
    exit 1
fi

if [[ ! -f "$1" ]]; then
    echo "Error: input file not found: $1" >&2
    exit 1
fi

input=$(realpath "$1")
base="${input%.*}_roundtrip"
mtlx="$base.mtlx"
mxsl="$base.mxsl"

# mxslc exits with 0 even on some failures, so remove stale outputs and check
# that each pass actually produced its file.
rm -f "$mtlx" "$mxsl"

# Match the roundtrip tests, which compile without graph reduction.
"$mxslc" compile "$input" -o "$mtlx" --no-reduce-graph
[[ -f "$mtlx" ]] || { echo "Error: compile pass failed" >&2; exit 1; }

"$mxslc" decompile "$mtlx" -o "$mxsl"
[[ -f "$mxsl" ]] || { echo "Error: decompile pass failed" >&2; exit 1; }
