#!/usr/bin/env bash
#
# Builds the mxslc JavaScript (WebAssembly) bindings on macOS / Linux and runs
# the JavaScript unit tests.
#
# This mirrors the approach used by MaterialX's own JavaScript build
# (see bernard_MaterialX_3/javascript/build_javascript.sh): it sources the
# Emscripten SDK, compiles MaterialX to WebAssembly, builds the mxslc JS module
# with -DBUILD_JS=ON, and finally runs the Playwright test suite.
#
# Usage (run from the repository root or from this folder):
#   javascript/build_javascript.sh [emsdk_location] [materialx_source]
#
# The emsdk location defaults to '../emsdk' (relative to this script).
# The MaterialX source is required (it is never auto-detected).
#
set -euo pipefail

# Resolve the repository root (parent of this folder).
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

EMSDK_LOCATION="${1:-$ROOT_DIR/../emsdk}"
EMSDK_LOCATION="$(cd "$EMSDK_LOCATION" && pwd)"

# MaterialX source tree used to build a WebAssembly install. This is required
# and is never auto-detected or hard-coded: pass it as the second positional
# argument (or via the MATERIALX_SRC environment variable), pointing to a
# complete MaterialX source checkout (the directory containing CMakeLists.txt).
MATERIALX_SRC="${2:-${MATERIALX_SRC:-}}"
if [ -z "${MATERIALX_SRC:-}" ]; then
    echo "Error: MaterialX source is required but not set." >&2
    echo "  Pass it as the second argument, e.g.:" >&2
    echo "    javascript/build_javascript.sh [emsdk_location] /path/to/MaterialX" >&2
    exit 1
fi

BUILD_DIR="$ROOT_DIR/javascript/build"
MTLX_INSTALL="$BUILD_DIR/mtlx_install"

echo "--------------------- Setup Emscripten ---------------------"
source "$EMSDK_LOCATION/emsdk_env.sh" > /dev/null 2>&1 || true
if ! command -v emcc > /dev/null 2>&1; then
    echo "Error: emcc not found after sourcing $EMSDK_LOCATION/emsdk_env.sh" >&2
    exit 1
fi
echo "Using $(emcc --version | head -1)"

echo "--------------------- Setup Node.js ---------------------"
# Prefer the Node.js shipped with the active emsdk (>= 18 is required for the
# Playwright tooling).
if [ -n "${EMSDK_NODE:-}" ] && [ -x "$EMSDK_NODE" ]; then
    export PATH="$(dirname "$EMSDK_NODE"):$PATH"
fi
NODE_MAJOR="$(node -e 'process.stdout.write(String(process.versions.node.split(".")[0]))' 2>/dev/null || echo 0)"
if [ "$NODE_MAJOR" -lt 18 ]; then
    echo "Error: Node.js >= 18 is required, found '$(node --version)'." >&2
    exit 1
fi
echo "Using Node $(node --version)"

echo "--------------------- Build MaterialX (WebAssembly) ---------------------"
if [ ! -d "$MATERIALX_SRC" ]; then
    echo "Error: MaterialX source tree not found at $MATERIALX_SRC. Set MATERIALX_SRC." >&2
    exit 1
fi
emcmake cmake -S "$MATERIALX_SRC" -B "$BUILD_DIR/materialx" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$MTLX_INSTALL" \
    -DMATERIALX_BUILD_TESTS=OFF \
    -DMATERIALX_BUILD_VIEWER=OFF \
    -DMATERIALX_BUILD_GRAPH_EDITOR=OFF \
    -DMATERIALX_BUILD_PYTHON=OFF \
    -DMATERIALX_BUILD_JS=OFF \
    -DMATERIALX_BUILD_GEN_GLSL=OFF \
    -DMATERIALX_BUILD_GEN_MDL=OFF \
    -DMATERIALX_BUILD_GEN_OSL=OFF \
    -DMATERIALX_BUILD_GEN_SLANG=OFF \
    -DMATERIALX_BUILD_RENDER=OFF \
    -G Ninja
cmake --build "$BUILD_DIR/materialx" --target install --config Release --parallel

echo "--------------------- Build mxslc JavaScript ---------------------"
cmake -S "$ROOT_DIR" -B "$BUILD_DIR/mxslc" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_JS=ON \
    -DMXLSC_EMSDK_PATH="$EMSDK_LOCATION" \
    -DMTLX_ROOT="$MTLX_INSTALL" \
    -G Ninja
cmake --build "$BUILD_DIR/mxslc" --target JsMxslc --config Release --parallel

echo "--------------------- Install JavaScript Dependencies ---------------------"
cd "$ROOT_DIR/javascript"
npm install || echo "Warning: npm install reported issues"

echo "--------------------- Run JavaScript Tests ---------------------"
cd "$ROOT_DIR/javascript/tests"
npx playwright install chromium || true
npm run test

echo "--------------------- Browser Setup ---------------------"
# Refresh the self-contained browser app with the freshly built WASM artifacts.
BROWSER_DIR="$ROOT_DIR/javascript/browser"
mkdir -p "$BROWSER_DIR/lib"
cp "$BUILD_DIR/mxslc/javascript/bin/JsMxslc.js" \
   "$BUILD_DIR/mxslc/javascript/bin/JsMxslc.wasm" \
   "$BUILD_DIR/mxslc/javascript/bin/JsMxslc.data" \
   "$BROWSER_DIR/lib/"
echo "Copied WASM artifacts into $BROWSER_DIR/lib/"

echo "--------------------- Run Browser ---------------------"
# The server is NOT started automatically. Serve the app manually when ready
# (WASM fetches .wasm/.data over HTTP, so it cannot be opened via file://).
echo "To run the browser app, from: $BROWSER_DIR"
echo "    node serve.js            # -> http://localhost:8080"
echo "  or:"
echo "    python3 -m http.server 8080"

echo "--------------------- Done ---------------------"
echo "The WebAssembly module was written to:"
echo "  $BUILD_DIR/mxslc/javascript/bin/JsMxslc.js"
echo "  $BUILD_DIR/mxslc/javascript/bin/JsMxslc.wasm"
echo "  $BUILD_DIR/mxslc/javascript/bin/JsMxslc.data"
echo "Browser app: $BROWSER_DIR/index.html (run 'node serve.js' from $BROWSER_DIR)"
