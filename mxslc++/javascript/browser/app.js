// ============================================================
//  MXSL <-> MTLX Converter — WASM (in-browser) app entry
// ============================================================
//  Defines the WASM engine and bootstraps the shared UI core in common.js.

import { setEngine, start, logMessage } from './common.js';
import Mxslc from './lib/JsMxslc.js';

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
        logMessage('WebAssembly module loaded', 'success');
    },

    // True once the WASM module is available for conversions.
    isReady() { return !!mx; },

    // Return the keywords, type names and function names, from the compiler.
    getKeywords() {
        const reflection = mx.reflect();
        return {
            keywords: mx.getKeywords(),
            types: reflection.typeNames,
            functions: reflection.functionNames,
        };
    },

    // Compile MXSL source to MTLX XML. Maps the shared plain options object
    // onto a WASM CompileOptions instance. Returns the MTLX string.
    compile(source, options) {
        const opts = new mx.CompileOptions();
        opts.version = options.version;
        opts.reduceGraph = options.reduce_graph;
        opts.errorOnMissingGlobals = options.error_on_missing_globals;
        opts.errorOnUnusedGlobals = options.error_on_unused_globals;
        opts.defaultFunctionBehaviour = mx.FunctionBehaviour[options.default_function_behaviour ?? 'NodeDef'];
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
