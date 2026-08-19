// ============================================================
//  MXSL <-> MTLX Converter — WASM (in-browser) app entry
// ============================================================
//  Defines the WASM engine and bootstraps the shared UI core in common.js.

import { setEngine, start, logMessage } from './common.js';
import Mxslc from './lib/JsMxslc.js';
import { MXSL_KEYWORDS } from './keywords.js';

// ============================================================
//  Engine adapter (WASM)
// ============================================================

let mx = null; // The mxslc API object, resolved after the WASM module loads.

const engine = {
    // Load the WebAssembly module. The .wasm and .data files are fetched
    // relative to this script (see locateFile), so this page must be served
    // over HTTP.
    async init() {
        mx = await Mxslc({
            locateFile: (file) => new URL('./lib/' + file, import.meta.url).href,
        });
        logMessage('WebAssembly module loaded (' +
            MXSL_KEYWORDS.control.length + ' control keywords, ' +
            MXSL_KEYWORDS.functions.length + ' built-in functions)', 'success');
    },

    // True once the WASM module is available for conversions.
    isReady() { return !!mx; },

    // Return the authoritative keyword sets. `functions` is the MaterialX
    // nodedef-category list from the WASM library and is used first; the
    // static MXSL_KEYWORDS backup is only consulted if this returns no
    // functions. Control/data-type keywords always come from the static list
    // (they are language keywords, not MaterialX nodedefs).
    getKeywords() {
        const defs = mx.getMtlxDefinitionNames();
        const count = defs.size();
        const functions = [];
        for (let i = 0; i < count; i++) functions.push(defs.get(i));
        return Promise.resolve({
            data_types: MXSL_KEYWORDS.dataTypes,
            control: MXSL_KEYWORDS.control,
            builtins: [],
            functions,
        });
    },

    // Compile MXSL source to MTLX XML. Maps the shared plain options object
    // onto a WASM CompileOptions instance. Returns the MTLX string.
    compile(source, options) {
        const opts = new mx.CompileOptions();
        opts.version = options.version;
        opts.reduceGraph = options.reduce_graph;
        opts.errorOnMissingGlobals = options.error_on_missing_globals;
        opts.errorOnUnusedGlobals = options.error_on_unused_globals;
        opts.singleUseAsNodegraph = options.single_use_as_nodegraph;
        const result = mx.compileSlxToMtlx(source, opts);
        opts.delete();
        return result;
    },

    // Decompile MTLX XML to MXSL source. Returns the MXSL string.
    decompile(source) {
        return mx.decompileMtlxToSlx(source);
    },
};

// Inject the engine into the shared core, then start the app.
setEngine(engine);
start();

// Hide version option if the WASM engine is used, since it only supports the
// version embedded in the WASM library (currently 1.39.5). The Flask backend can support
document.getElementById('version_select').style.display = 'none';
