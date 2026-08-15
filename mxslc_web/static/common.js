import { MXSL_KEYWORDS } from './keywords.js';

// The engine (compile / decompile / keyword provider) is injected by the host
// app (app.js) via setEngine() before start() is called. The shared functions
// below reference this module-level binding.
let engine = null;
export function setEngine(e) { engine = e; }

// ============================================================
//  Logging / status helpers
// ============================================================

export function logMessage(message, type = 'info') {
    const el = document.getElementById('message-log');
    const timestamp = new Date().toLocaleTimeString();
    const icons = { error: '[\u2716]', success: '[\u2714]', info: '[\u2139]' };
    const icon = icons[type] || icons.info;
    el.value += `[${timestamp}] ${icon} ${message}\n`;
    el.scrollTop = el.scrollHeight;
}

// Defensively extract a readable message from anything thrown.
function errorMessage(err) {
    return (err && err.message) ? err.message : String(err);
}

function clearLog() {
    document.getElementById('message-log').value = '';
}

// Update the server-status badge in the log header.
function setServerStatus(label, icon, cls) {
    const el = document.getElementById('server-status');
    if (!el) return;
    el.innerHTML = `<i class="bi ${icon} me-1"></i>${label}`;
    el.className = 'server-status ' + (cls || '');
}

// ============================================================
//  CodeMirror MXSL mode
// ============================================================

function registerMxslMode(dataTypes, control, builtins, functions, funcStyle) {
    dataTypes = new Set(dataTypes || []);
    control = new Set(control || []);
    builtins = new Set(builtins || []);
    functions = new Set(functions || []);
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
                    if (builtins.has(word)) return 'builtin';
                    if (functions.has(word) && stream.match(/\s*\(/, false)) return funcStyle;
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

// ============================================================
//  Editors
// ============================================================

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
            placeholder: 'Paste MTLX XML here, or load a file...'
        })
    );
    editorMxsl = CodeMirror.fromTextArea(
        document.getElementById('editor-mxsl'),
        Object.assign({}, sharedOptions, {
            theme: 'darcula',
            mode: 'mxsl',
            placeholder: 'Paste MXSL code here, or load a file...'
        })
    );
    editorMtlx.refresh();
    editorMxsl.refresh();
}

// ============================================================
//  Keyword resolution — MaterialX list first, static backup second
// ============================================================

async function loadKeywords() {
    try {
        const kw = await engine.getKeywords();
        const functions = kw.functions || [];
        if (functions.length > 0) {
            registerMxslMode(kw.data_types, kw.control, kw.builtins, functions);
            logMessage(`Loaded ${functions.length} MaterialX keywords.`, 'success');
            return 'materialx';
        }
        logMessage('No MaterialX keywords returned; using static backup.', 'info');
    } catch (err) {
        logMessage('Failed to load MaterialX keywords: ' + errorMessage(err), 'error');
    }
    // Backup: static keyword list embedded in keywords.js.
    registerMxslMode(MXSL_KEYWORDS.dataTypes, MXSL_KEYWORDS.control, [], MXSL_KEYWORDS.functions);
    logMessage(`Using static keyword backup (${MXSL_KEYWORDS.functions.length} functions).`, 'info');
    return 'backup';
}

// ============================================================
//  File loading
// ============================================================

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

// Read compile options from the page. The shape is engine-specific; the WASM
// engine maps these onto its CompileOptions object internally.
function getCompileOptions() {
    return {
        version: document.getElementById('opt-version').value,
        reduce_graph: document.getElementById('opt-reduce-graph').checked,
        error_on_missing_globals: document.getElementById('opt-error-missing').checked,
        error_on_unused_globals: document.getElementById('opt-error-unused').checked,
    };
}

// ============================================================
//  Conversion
// ============================================================

function ensureReady() {
    if (!engine.isReady()) {
        throw new Error('Engine is still loading. Please wait a moment and try again.');
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
        const result = await engine.decompile(source);
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
        const result = await engine.compile(source, options);
        editorMtlx.setValue(result);
        logMessage(`MXSL compiled to MTLX (${result.length} chars)`, 'success');
    } catch (err) {
        logMessage('Compile error: ' + errorMessage(err), 'error');
    }
}

// ============================================================
//  Expose handlers (used by inline onclick in index.html).
//  ES module top-level bindings are module-scoped, so they must be
//  attached to window explicitly.
// ============================================================

window.loadFile = loadFile;
window.clearLog = clearLog;
window.toggleOptions = toggleOptions;
window.convertMtlxToMxsl = convertMtlxToMxsl;
window.convertMxslToMtlx = convertMxslToMtlx;

// ============================================================
//  Bootstrap
// ============================================================

document.getElementById('file-input').addEventListener('change', onFileChosen);

// Called by the host app after setEngine(). Loads the engine, resolves the
// keyword list (MaterialX first, static backup), and initializes the editors.
export async function start() {
    logMessage('Starting...', 'info');
    try {
        await engine.init();              // load backend / WASM
        setServerStatus('Server ready', 'bi-check-circle-fill', 'text-success');
    } catch (err) {
        logMessage('Failed to initialize: ' + errorMessage(err), 'error');
        setServerStatus('Server failed', 'bi-x-circle-fill', 'text-danger');
    }
    // Always create the editors. loadKeywords uses the MaterialX list first and
    // falls back to the static keyword list, so editing works even if the
    // engine failed to load.
    await loadKeywords();
    initEditors();
}
