# mxslc JavaScript (WebAssembly) Bindings

This folder contains the Emscripten (WebAssembly) bindings for the mxslc
compile / decompile pipeline, modeled on the JavaScript build used by
MaterialX itself (see the `javascript/` folder of a MaterialX checkout).

It provides a JavaScript API for converting between ShadingLanguageX (SLX)
source strings or files and MaterialX (MTLX) XML strings or files.

## Building

The build script sources the Emscripten SDK, compiles MaterialX to WebAssembly,
builds the `JsMxslc` module with `-DBUILD_JS=ON`, and runs the tests.

```sh
# Run from the mxslc++ repository root:
javascript/build_javascript.sh [emsdk_location]
```

The emsdk location defaults to `../emsdk` (i.e. `$REPO/../emsdk`). The MaterialX
source tree used for the wasm build defaults to the bundled `MaterialX-src`
folder and can be overridden with the `MATERIALX_SRC` environment variable.

## Output

After a successful build the WebAssembly module is written to:

```
javascript/build/mxslc/javascript/bin/JsMxslc.js
javascript/build/mxslc/javascript/bin/JsMxslc.wasm
javascript/build/mxslc/javascript/bin/JsMxslc.data
```

`JsMxslc.data` contains the preloaded `libraries/` folder that the compiler
needs to locate the standard library and the MaterialX node definitions.

## JavaScript API

The module is an ES6 module that exports a factory function:

```js
import Mxslc from './JsMxslc.js';
const mx = await Mxslc();

// Compile using a CompileOptions instance.
const opts = new mx.CompileOptions();
opts.version = '1.39.5';
opts.reduceGraph = true;
const mtlx2 = mx.compileSlxToMtlx('float z = add(1.0, 2.0);', opts);

// Compile a file that already exists in the WASM filesystem.
opts.outputFile = '/tmp/out.mtlx';
const mtlx3 = mx.compileSlxFileToMtlx('/tmp/in.mxsl', opts);
const outPath = mx.compileSlxFileToMtlxFile('/tmp/in.mxsl', opts);

opts.delete();

// Decompile a MTLX (MaterialX XML) string back to an SLX string.
const slx = mx.decompileMtlxToSlx(mtlx);
```

### CompileOptions

`CompileOptions` exposes the following writable properties:

- `outputFile` (`string | null`) - destination path used by file-output compile calls.
- `version` (string) - the MaterialX library version to load.
- `macros` (`Array<string | [string] | [string, string]>`)
- `searchDirectories` (`string[]`)
- `includes` (`string[]`)
- `libraries` (`string[]`)
- `globals` (`Record<string, unknown>`)
- `reduceGraph` (boolean)
- `validateGraph` (boolean)
- `errorOnMissingGlobals` (boolean)
- `errorOnUnusedGlobals` (boolean)
- `funcName` (`string | null`)
- `funcArgs` (`unknown[]`)

### File-based compile API

File-based compile calls operate on paths inside the Emscripten/WASM
filesystem:

- `mx.compileSlxFileToMtlx(srcPath, opts?)` → MTLX XML string
- `mx.compileSlxFileToMtlxFile(srcPath, opts?)` → output path string

## Testing

Tests are Playwright unit tests that run in Node (no browser required). They
live in the `tests/` folder.

```sh
cd javascript/tests
npm install
npx playwright install chromium
npm run test
```

The `build_javascript.sh` script runs the test suite automatically after the
build completes.

## CMake integration

Building the JS bindings requires configuring the project with `-DBUILD_JS=ON`
and an Emscripten SDK. The root `CMakeLists.txt` handles sourcing the Emscripten
toolchain via `MXLSC_EMSDK_PATH` (or the `EMSDK` environment variable) and only
adds this `javascript/` subdirectory when the JS build is enabled.
