# MXSL ⇄ MTLX Converter — Browser (WASM) version

A fully client-side interactive converter between **MXSL** (the `ShadingLanguageX` shading
language) and **MTLX** (MaterialX `XML`). All browser interaction uses the **JsMxslc**
WebAssembly module.

## Layout

```
browser/
├── index.html      # Page UI (two editors + log)
├── app.js          # App logic: loads WASM, wires editors + conversion
├── common.js       # Shared UI core (a copy of browser_shared/common.js)
├── serve.js        # Minimal Node static file server
├── README.md       # This file
└── lib/            # WASM package
    ├── JsMxslc.js      # ES module (export default Mxslc)
    ├── JsMxslc.wasm    # WebAssembly binary
    └── JsMxslc.data    # Preloaded filesystem (libraries/ folder)
```

## Execution

The WASM module fetches `JsMxslc.wasm` and `JsMxslc.data` over HTTP, so the
page must be served (not opened as `file://`).

From this `browser/` folder:

```bash
node serve.js            # -> http://localhost:8080
node serve.js 9090       # custom port
```

or with Python's built-in server:

```bash
python3 -m http.server 8080
```

Then open `http://localhost:8080`.

The entire contents of the folder may be copied and served as a standalone from any static web server.

## The WASM module

Is built by the mxslc JavaScript (WebAssembly) build. The artifacts
are produced by running the `build_javascript.sh` script from the repo root, passing the
path to the Emscripten SDK and the MaterialX source tree paths.

Example:
```bash
# Run from /mxslc++
./javascript/build_javascript.sh ../../emsdk MaterialX-1.39.5
```

The compiled output is saved in `javascript/build/mxslc/javascript/bin/`
(`JsMxslc.js`, `JsMxslc.wasm`, `JsMxslc.data`).

### Refreshing the WASM library

After rebuilding, the `lib/` folder can be updated with the new artifacts:

```bash
cp javascript/build/mxslc/javascript/bin/JsMxslc.js \
   javascript/build/mxslc/javascript/bin/JsMxslc.wasm \
   javascript/build/mxslc/javascript/bin/JsMxslc.data \
   javascript/browser/lib/
```

## API Usage

The module exposes the mxslc API as a Promise-returning factory. `app.js` loads
this in as follows:

```js
import Mxslc from './lib/JsMxslc.js';
const mx = await Mxslc({
    locateFile: (file) => new URL('./lib/' + file, import.meta.url).href,
});
```

Then conversion calls (all synchronous, throw on error):

- `mx.compileSlxToMtlx(slx, opts)` → MTLX XML string, with `opts = new mx.CompileOptions()`
- `mx.decompileMtlxToSlx(mtlx)` → MXSL string
- `mx.getKeywords()` → array of the SLX keywords
- `mx.reflect()` / `mx.reflect(opts)` → `{ globals, typeNames, functionNames }` of an empty program: the names of
  the types (e.g. 'float', 'vec3', 'surfaceshader') and functions (e.g. 'add', 'image', 'standard_surface') every
  program can use
- `mx.reflectFromString(slx)` / `mx.reflectFromString(slx, opts)` → `{ globals, typeNames, functionNames }` of the
  source. `globals` is an array of `{ name, type, defaultValue }`, and `typeNames` and `functionNames` are sorted
  arrays of strings

## CI / Deployment

The `.github/workflows/build-js.yml` workflow builds the WASM module, runs the
unit tests, and (when enabled) deploys the `browser/` folder to GitHub Pages.

**Build only (validation)** — happens automatically on:
- opening or updating a pull request
- any manual dispatch with the Deploy checkbox **off**

**Build + deploy to Pages** — happens when:
- a pull request is **merged into `main`** (automatic), or
- a manual dispatch is run with the **"Deploy to GitHub Pages"** checkbox **on**
  (e.g. to test-deploy the current branch):
  ```bash
  gh workflow run build-js.yml --ref <branch> -f deploy=true
  ```
  UI: **Actions → Build JavaScript (WASM) → Run workflow → tick Deploy**.

> Note: a manual test-deploy publishes that branch's build to Pages and
> overwrites the live site until the next merge to `main`.

## Notes

- Syntax highlighting gets the keywords, types and functions from the compiler, with `getKeywords()` and `reflect()`.
  If the WebAssembly module fails to load, nothing is highlighted as a keyword, type or function, but the page can't
  convert anything then either.
