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

// Defensively extract a readable message from anything thrown, so an unexpected
// non-Error value never renders as 'undefined' in the log.
function errorMessage(err) {
    return (err && err.message) ? err.message : String(err);
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
    extraKeys: {
        'Ctrl-S': function(cm) { cm.save(); },
        'Cmd-S': function(cm) { cm.save(); }
    }
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

function toggleOptions() {
    const body = document.getElementById('options-body');
    const label = document.getElementById('options-toggle-label');
    const isHidden = body.classList.contains('d-none');
    body.classList.toggle('d-none');
    label.textContent = isHidden ? 'hide' : 'show';
}

// Get page options and return a CompileOptions instance 
function getCompileOptions() {
    const opts = new mx.CompileOptions();
    opts.version = document.getElementById('opt-version').value;
    opts.reduceGraph = document.getElementById('opt-reduce-graph').checked;
    opts.errorOnMissingGlobals = document.getElementById('opt-error-missing').checked;
    opts.errorOnUnusedGlobals = document.getElementById('opt-error-unused').checked;
    return opts;
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
        logMessage('Decompile error: ' + errorMessage(err), 'error');
    }
}

// Compile: MXSL source -> MTLX XML
async function convertMxslToMtlx() {
    const options = getCompileOptions();

    const source = editorMxsl.getValue().trim();
    if (!source) {
        logMessage('MXSL editor is empty', 'error');
        return;
    }
    logMessage('Compiling MXSL -> MTLX (version=' + options.version +
        ', reduce=' + options.reduce_graph + ')...', 'info');
    try {
        ensureReady();
        const result = mx.compileSlxToMtlx(source, options);
        editorMtlx.setValue(result);
        logMessage(`MXSL compiled to MTLX (${result.length} chars)`, 'success');
    } catch (err) {
        logMessage('Compile error: ' + errorMessage(err), 'error');
    }
    options.delete();
}

// ---------------------------------------------------------------------------
// Expose handlers to the global scope (used by inline onclick in index.html).
// ES module top-level bindings are module-scoped, so they must be attached to
// window explicitly.
// ---------------------------------------------------------------------------

window.loadFile = loadFile;
window.clearLog = clearLog;
window.toggleOptions = toggleOptions;
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

let version_select = document.getElementById('version_select');
version_select.style.display = 'none';  // Hide version select for now, until we support multiple versions

document.getElementById('file-input').addEventListener('change', onFileChosen);

function setWasmStatus(label, icon, cls) {
    const el = document.getElementById('server-status');
    if (!el) return;
    el.innerHTML = `<i class="bi ${icon} me-1"></i>${label}`;
    el.className = 'server-status ' + (cls || '');
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
            logMessage('Could not load MaterialX nodedefs: ' + errorMessage(err), 'error');
        }

        setWasmStatus('Wasm ready', 'bi-check-circle-fill', 'text-success');
    } catch (err) {
        logMessage('Failed to load WebAssembly module: ' + errorMessage(err), 'error');
        setWasmStatus('Wasm failed', 'bi-x-circle-fill', 'text-danger');
    }
})();
