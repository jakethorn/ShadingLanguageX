# MXSL ⇄ MTLX Converter — Browser (WASM) version

A fully client-side interactive converter between **MXSL** (the `ShadingLanguageX` shading
language) and **MTLX** (MaterialX `XML`). All browser interaction uses the **JsMxslc**
WebAssembly module.

## Layout

```
browser/
├── index.html      # Page UI (two editors + log)
├── app.js          # App logic: loads WASM, wires editors + conversion
├── keywords.js     # Embedded MXSL keyword lists for syntax highlighting
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

- `mx.compileSlxToMtlx(slx)` → MTLX XML string
- `mx.decompileMtlxToSlx(mtlx)` → MXSL string
- `mx.compileSlxToMtlxWithOptions(slx, opts)` / `new mx.CompileOptions()`
- `mx.getMtlxDefinitionNames()` → array of MaterialX definition categories (e.g. 'add', 'image', 'standard_surface', etc.)

## Notes

- Syntax highlighting uses the SLX keywords list from `keywords.js` and retrieves the MaterialX node/function names using `getMtlxDefinitionNames()`. In the event the library fails to the load
there is a set of fallback MaterialX keywords in `keywords.js`.
