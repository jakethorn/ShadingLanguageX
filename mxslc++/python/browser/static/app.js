// ============================================================
//  MXSL <-> MTLX Converter — Flask (backend) app entry
// ============================================================
//  Defines the Flask engine and bootstraps the shared UI core in common.js.

import { setEngine, start, logMessage } from './common.js';

// ============================================================
//  Engine adapter (Flask backend)
// ============================================================

const engine = {
    // Load anything needed before conversion is available. For Flask the
    // backend is already serving the page, so this is effectively a no-op.
    init() {
        logMessage('Backend ready', 'success');
    },

    // True once conversions can be performed.
    isReady() { return true; },

    // Return the keywords, type names and function names, from the compiler.
    async getKeywords() {
        const r = await fetch('/api/keywords');
        const data = await r.json();
        if (data.error) throw new Error(data.error);
        return data;
    },

    // Compile MXSL source to MTLX XML. Returns the MTLX string.
    async compile(source, options) {
        const r = await fetch('/api/compile', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ source, options })
        });
        const data = await r.json();
        if (data.error) throw new Error(data.error);
        return data.result;
    },

    // Decompile MTLX XML to MXSL source. Returns the MXSL string.
    async decompile(source) {
        const r = await fetch('/api/decompile', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ source })
        });
        const data = await r.json();
        if (data.error) throw new Error(data.error);
        return data.result;
    },
};

// Inject the engine into the shared core, then start the app.
setEngine(engine);
start();
