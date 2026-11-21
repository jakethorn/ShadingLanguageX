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

function registerMxslMode(keywords, types, functions) {
    keywords = new Set(keywords);
    types = new Set(types);
    functions = new Set(functions);

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
                    if (types.has(word)) return 'type';
                    if (keywords.has(word)) return 'keyword';
                    if (functions.has(word) && stream.match(/\s*\(/, false)) return 'function';
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

    // Set default text for mxsl
    let defaultMxsl = `
    material main()
    {
        surfaceshader goldsrf = standard_surface();
        goldsrf.base = 1.0;
        goldsrf.base_color = color3{0.944, 0.776, 0.373};
        goldsrf.specular = 1.0;
        goldsrf.specular_color = color3{0.998, 0.981, 0.751};
        goldsrf.specular_roughness = 0.02;
        goldsrf.metalness = 1.0;

        return surfacematerial(goldsrf);
    }
    `;
    editorMxsl.setValue(defaultMxsl);

    // Set default text for mtlx
    let defaultMtlx = `
<?xml version="1.0"?>
<materialx version="1.39">
  <standard_surface name="goldsrf" type="surfaceshader">
    <input name="base" type="float" value="1" />
    <input name="base_color" type="color3" value="0.944, 0.776, 0.373" />
    <input name="specular" type="float" value="1" />
    <input name="specular_color" type="color3" value="0.998, 0.981, 0.751" />
    <input name="specular_roughness" type="float" value="0.02" />
    <input name="metalness" type="float" value="1" />
  </standard_surface>
  <surfacematerial name="goldmat" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="goldsrf" />
  </surfacematerial>
</materialx>
`;
    editorMtlx.setValue(defaultMtlx);
    logMessage('Editors loaded', 'success');
}

// ============================================================
//  Keywords, types and functions, from the compiler
// ============================================================

async function loadKeywords() {
    try {
        const { keywords, types, functions } = await engine.getKeywords();
        registerMxslMode(keywords, types, functions);
        logMessage(`Loaded ${keywords.length} keywords, ${types.length} types and ${functions.length} functions.`, 'success');
    } catch (err) {
        // Without the compiler there is nothing to highlight them with, but comments, strings and numbers still are.
        registerMxslMode([], [], []);
        logMessage('Failed to load the keywords, types and functions: ' + errorMessage(err), 'error');
    }
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
        default_function_behaviour: document.getElementById('opt-default-function-behaviour').value,
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

// Called by the host app after setEngine(). Loads the engine, gets the
// keywords, types and functions from it, and initializes the editors.
export async function start() {
    logMessage('Starting...', 'info');
    try {
        await engine.init();              // load backend / WASM
        setServerStatus('Server ready', 'bi-check-circle-fill', 'text-success');
    } catch (err) {
        logMessage('Failed to initialize: ' + errorMessage(err), 'error');
        setServerStatus('Server failed', 'bi-x-circle-fill', 'text-danger');
    }
    // Always create the editors, so editing works even if the engine failed to
    // load. Then nothing is highlighted as a keyword, type or function.
    await loadKeywords();
    initEditors();
}
