# MXSL ⇄ MTLX Converter — Browser (WASM) version

A fully client-side converter between **MXSL** (the ShadingLanguageX shading
language) and **MTLX** (MaterialX XML). Unlike the Flask + Python reference
app (`mxslc_web`), everything runs in the browser via the **JsMxslc**
WebAssembly module — no server-side compile/decompile and no Python backend.

## Layout

```
browser/
├── index.html      # Page UI (two editors + log)
├── app.js          # App logic: loads WASM, wires editors + conversion
├── keywords.js     # Embedded MXSL keyword lists for syntax highlighting
├── serve.js        # Minimal Node static file server
├── README.md       # This file
└── lib/            # Copied WASM build artifacts
    ├── JsMxslc.js      # ES module (export default Mxslc)
    ├── JsMxslc.wasm
    └── JsMxslc.data    # Preloaded filesystem (libraries/ folder)
```

## Running it

The WASM module fetches `JsMxslc.wasm` and `JsMxslc.data` over HTTP, so the
page **must be served** (not opened as `file://`).

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

## What the WASM module is

The build artifacts come from the mxslc JavaScript (WebAssembly) build. They
are produced by:

```bash
# from the repo root (mxslc++)
./javascript/build_javascript.sh ../../emsdk MaterialX-1.39.5
```

The compiled output lands in `javascript/build/mxslc/javascript/bin/`
(`JsMxslc.js`, `JsMxslc.wasm`, `JsMxslc.data`).

### Refreshing the WASM library

After rebuilding, refresh the copies in `lib/`:

```bash
cp javascript/build/mxslc/javascript/bin/JsMxslc.js \
   javascript/build/mxslc/javascript/bin/JsMxslc.wasm \
   javascript/build/mxslc/javascript/bin/JsMxslc.data \
   javascript/browser/lib/
```

## API used

The module exposes the mxslc API as a Promise-returning factory. `app.js` loads
it with:

```js
import Mxslc from './lib/JsMxslc.js';
const mx = await Mxslc({
    locateFile: (file) => new URL('./lib/' + file, import.meta.url).href,
});
```

Then conversion calls (all synchronous, throw on error):

- `mx.compileSlxToMtlx(slx)` → MTLX XML string
- `mx.decompileMtlxToSlx(mtlx)` → MXSL string
- `mx.compileSlxToMtlxWithOptions(slx, opts)` / `new mx.CompileOptions()`

## Notes

- Syntax highlighting uses **embedded** keyword lists in `keywords.js`
  (there's no backend to fetch them). Edit that file if the language changes.
- Conversion runs synchronously on the main thread; the module is ~1.2 MB
  WASM + ~3.9 MB data, so the first load takes a moment.
