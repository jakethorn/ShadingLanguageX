// Main application logic for the client-side MXSL <-> MTLX converter.
// All compile/decompile happens in the browser via the JsMxslc WebAssembly
// module.

import { MXSL_KEYWORDS } from './keywords.js';
import Mxslc from './lib/JsMxslc.js';

// ---------------------------------------------------------------------------
// Logging / status helpers
// ---------------------------------------------------------------------------

function logMessage(message, type = 'info') {
    const el = document.getElementById('message-log');
    const timestamp = new Date().toLocaleTimeString();
    const icons = { error: '[\u2716]', success: '[\u2714]', info: '[\u2139]' };
    const icon = icons[type] || icons.info;
    el.value += `[${timestamp}] ${icon} ${message}\n`;
    el.scrollTop = el.scrollHeight;
}

function clearLog() {
    document.getElementById('message-log').value = '';
}

// ---------------------------------------------------------------------------
// CodeMirror MXSL mode (built from the embedded keyword lists)
// ---------------------------------------------------------------------------

// Set of names highlighted as functions. Seeded from the static keyword list
// so the editor has something to highlight before WASM loads, then replaced by
// the authoritative MaterialX nodedef category set once the WASM module is
// ready. The static list is kept only as a fallback when the WASM module
// returns no nodedefs.
let mxslFunctionSet = null;

function registerMxslMode(dataTypes, control, functions, funcStyle) {
    dataTypes = new Set(dataTypes || []);
    control = new Set(control || []);
    mxslFunctionSet = new Set(functions || []);
    funcStyle = funcStyle || 'function';

    CodeMirror.defineMode('mxsl', function () {
        return {
            startState: function () { return { inComment: false }; },
            token: function (stream, state) {
                if (state.inComment) {
                    const m = stream.match(/.*?\*\//);
                    if (m) { state.inComment = false; return 'comment'; }
                    stream.skipToEnd();
                    return 'comment';
                }
                if (stream.eatSpace()) return null;
                if (stream.match(/\/\/.*/)) return 'comment';
                if (stream.match(/\/\*/)) { state.inComment = true; return 'comment'; }
                if (stream.match(/"(?:[^"\\]|\\.)*"/)) return 'string';
                if (stream.match(/'(?:[^'\\]|\\.)*'/)) return 'string';
                if (stream.match(/\b\d+\.?\d*(?:[eE][+-]?\d+)?\b/)) return 'number';
                if (stream.match(/#\w+/)) return 'meta';
                if (stream.match(/[+\-*/%=<>!&|^~]+/)) return 'operator';
                if (stream.match(/[a-zA-Z_$][\w$]*/)) {
                    const word = stream.current();
                    if (dataTypes.has(word)) return 'type';
                    if (control.has(word)) return 'keyword';
                    if (mxslFunctionSet.has(word) && stream.match(/\s*\(/, false)) return funcStyle;
                    if (word === word.toUpperCase() && word.length >= 2) return 'atom';
                    return 'variable';
                }
                if (stream.match(/[{}()\[\];,.]/)) return null;
                stream.next();
                return null;
            }
        };
    });
}

// ---------------------------------------------------------------------------
// Editors
// ---------------------------------------------------------------------------

const sharedOptions = {
    lineNumbers: true,
    matchBrackets: true,
    styleActiveLine: true,
    indentUnit: 4,
    tabSize: 4,
    indentWithTabs: false,
    lineWrapping: true,
    autofocus: false,
};

let editorMtlx = null;
let editorMxsl = null;

function initEditors() {
    editorMtlx = CodeMirror.fromTextArea(
        document.getElementById('editor-mtlx'),
        Object.assign({}, sharedOptions, {
            theme: 'dracula',
            mode: 'xml',
            placeholder: 'Paste MTLX XML here, or load a file...',
        })
    );
    editorMxsl = CodeMirror.fromTextArea(
        document.getElementById('editor-mxsl'),
        Object.assign({}, sharedOptions, {
            theme: 'darcula',
            mode: 'mxsl',
            placeholder: 'Paste MXSL code here, or load a file...',
        })
    );
    editorMtlx.refresh();
    editorMxsl.refresh();
}

// ---------------------------------------------------------------------------
// File loading
// ---------------------------------------------------------------------------

let pendingTarget = null;

function loadFile(target) {
    pendingTarget = target;
    const input = document.getElementById('file-input');
    input.accept = '.mtlx,.mxsl,.xml,.txt';
    input.value = '';
    input.click();
}

function onFileChosen(e) {
    const file = e.target.files[0];
    if (!file || !pendingTarget) return;
    logMessage(`Loading ${file.name}...`, 'info');
    const reader = new FileReader();
    reader.onload = function (ev) {
        const content = ev.target.result;
        if (pendingTarget === 'mtlx') {
            editorMtlx.setValue(content);
            logMessage(`Loaded ${file.name} into MTLX editor`, 'success');
        } else {
            editorMxsl.setValue(content);
            logMessage(`Loaded ${file.name} into MXSL editor`, 'success');
        }
        pendingTarget = null;
    };
    reader.onerror = function () {
        logMessage('Failed to read file', 'error');
        pendingTarget = null;
    };
    reader.readAsText(file);
}

// ---------------------------------------------------------------------------
// Conversion (client-side, via WASM)
// ---------------------------------------------------------------------------

let mx = null; // The mxslc API object, resolved after the WASM module loads.

function ensureReady() {
    if (!mx) {
        throw new Error('WASM module is still loading. Please wait a moment and try again.');
    }
}

// Decompile: MTLX XML -> MXSL source
async function convertMtlxToMxsl() {
    const source = editorMtlx.getValue().trim();
    if (!source) {
        logMessage('MTLX editor is empty', 'error');
        return;
    }
    logMessage('Decompiling MTLX -> MXSL...', 'info');
    try {
        ensureReady();
        const result = mx.decompileMtlxToSlx(source);
        editorMxsl.setValue(result);
        logMessage(`MTLX decompiled to MXSL (${result.length} chars)`, 'success');
    } catch (err) {
        logMessage('Decompile error: ' + err.message, 'error');
    }
}

// Compile: MXSL source -> MTLX XML
async function convertMxslToMtlx() {
    // TODO: Expose compile options in UI.
    const source = editorMxsl.getValue().trim();
    if (!source) {
        logMessage('MXSL editor is empty', 'error');
        return;
    }
    logMessage('Compiling MXSL -> MTLX...', 'info');
    try {
        ensureReady();
        const opts = new mx.CompileOptions();
        const result = mx.compileSlxToMtlx(source, opts);
        editorMtlx.setValue(result);
        logMessage(`MXSL compiled to MTLX (${result.length} chars)`, 'success');
    } catch (err) {
        logMessage('Compile error: ' + err.message, 'error');
    }
}

// ---------------------------------------------------------------------------
// Expose handlers to the global scope (used by inline onclick in index.html).
// ES module top-level bindings are module-scoped, so they must be attached to
// window explicitly.
// ---------------------------------------------------------------------------

window.loadFile = loadFile;
window.clearLog = clearLog;
window.convertMtlxToMxsl = convertMtlxToMxsl;
window.convertMxslToMtlx = convertMxslToMtlx;

// ---------------------------------------------------------------------------
// Bootstrap
// ---------------------------------------------------------------------------

// Register the custom MXSL CodeMirror mode before creating the editors.
registerMxslMode(
    MXSL_KEYWORDS.dataTypes,
    MXSL_KEYWORDS.control,
    MXSL_KEYWORDS.functions
);

initEditors();

document.getElementById('file-input').addEventListener('change', onFileChosen);

function setWasmStatus(label, icon, cls) {
    const el = document.getElementById('wasm-status');
    if (!el) return;
    el.innerHTML = `<i class="bi ${icon} me-1"></i>${label}`;
    el.className = 'wasm-status ' + (cls || '');
}

// Load the WebAssembly module. The .wasm and .data files are fetched relative
// to this script (see locateFile), so this page must be served over HTTP.
(async function loadWasm() {
    logMessage('Loading WebAssembly module...', 'info');
    try {
        // The following will fetch JsMxslc.wasm and JsMxslc.data from the same folder as this script.
        // import.meta.url is the URL of this script, so we can use it to locate the other files. 
        mx = await Mxslc({
            locateFile: (file) => new URL('./lib/' + file, import.meta.url).href,
        });
        logMessage('WebAssembly module loaded (' +
            MXSL_KEYWORDS.control.length + ' control keywords, ' +
            MXSL_KEYWORDS.functions.length + ' built-in functions)', 'success');

        // Use the WASM module's MaterialX nodedef categories as the highlight
        // set; only fall back to the embedded static keyword list if the WASM
        // module returns no nodedefs at all.
        try {
            const defs = mx.getMtlxDefinitionNames();
            const count = defs.size();
            if (count > 0) {
                const nodedefSet = new Set();
                for (let i = 0; i < count; i++) 
                    nodedefSet.add(defs.get(i));
                mxslFunctionSet = nodedefSet;
                logMessage(`Loaded ${count} MaterialX definition categories.`, 'success');
            } else {
                // No nodedefs returned — keep the static keyword list fallback.
                logMessage('No MaterialX definition categories returned. Using backup list.', 'info');
            }
            // Force the MXSL editor to re-tokenize with the updated set.
            editorMxsl.setOption('mode', 'text/plain');
            editorMxsl.setOption('mode', 'mxsl');
        } catch (err) {
            logMessage('Could not load MaterialX nodedefs: ' + err.message, 'error');
        }

        setWasmStatus('WASM ready', 'bi-check-circle-fill', 'text-success');
    } catch (err) {
        logMessage('Failed to load WebAssembly module: ' + err.message, 'error');
        setWasmStatus('WASM failed', 'bi-x-circle-fill', 'text-danger');
    }
})();
