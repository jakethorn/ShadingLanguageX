// ============================================================
//  MXSL custom CodeMirror mode — built from API keywords
// ============================================================

/** Register a custom MXSL mode with categorized keywords from the API.
 *  @param {string} [funcStyle='def'] - Token class for function calls:
 *    'def' uses theme's cm-def color, 'function' uses custom .cm-function color. */
function registerMxslMode(dataTypes, control, builtins, functions, funcStyle) {
    dataTypes = new Set(dataTypes || []);
    control = new Set(control || []);
    functions = new Set(functions || []);
    funcStyle = funcStyle || 'function'; // or 'def'

    CodeMirror.defineMode('mxsl', function() {
        return {
            startState: function() { return { inComment: false }; },
            token: function(stream, state) {
                // Multi-line comments
                if (state.inComment) {
                    const m = stream.match(/.*?\*\//);
                    if (m) { state.inComment = false; return 'comment'; }
                    stream.skipToEnd();
                    return 'comment';
                }
                // Skip whitespace
                if (stream.eatSpace()) return null;

                // Single-line comment
                if (stream.match(/\/\/.*/)) return 'comment';

                // Multi-line comment start
                if (stream.match(/\/\*/)) { state.inComment = true; return 'comment'; }

                // String literals
                if (stream.match(/"(?:[^"\\]|\\.)*"/)) return 'string';
                if (stream.match(/'(?:[^'\\]|\\.)*'/)) return 'string';

                // Numbers
                if (stream.match(/\b\d+\.?\d*(?:[eE][+-]?\d+)?\b/)) return 'number';

                // Preprocessor / directives
                if (stream.match(/#\w+/)) return 'meta';

                // Operators
                if (stream.match(/[+\-*/%=<>!&|^~]+/)) return 'operator';

                // Identifiers and keywords
                if (stream.match(/[a-zA-Z_$][\w$]*/)) {
                    const word = stream.current();
                    if (dataTypes.has(word)) return 'type';
                    if (control.has(word)) return 'keyword';
                    if (builtins && builtins.includes(word)) return 'builtin';
                    if (functions.has(word) && stream.match(/\s*\(/, false)) return funcStyle;
                    if (word === word.toUpperCase() && word.length >= 2) return 'atom';
                    return 'variable';
                }

                // Braces and punctuation
                if (stream.match(/[{}()\[\];,.]/)) return null;

                stream.next();
                return null;
            }
        };
    });
}

// ============================================================
//  CodeMirror initialisation (after keywords are fetched)
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

let editorMtlx, editorMxsl;

// Fetch keywords from backend, register MXSL mode, then init editors
fetch('/api/keywords')
    .then(r => r.json())
    .then(data => {
        registerMxslMode(data.data_types, data.control, data.builtins, data.functions);
        logMessage('Loaded ' + (data.all || []).length + ' keywords and ' +
            (data.builtins || []).length + ' builtins', 'success');

        // MTLX editor – XML mode
        editorMtlx = CodeMirror.fromTextArea(
            document.getElementById('editor-mtlx'),
            Object.assign({}, sharedOptions, {
                theme: 'dracula',
                mode: 'xml',
                placeholder: 'Paste MTLX XML here, or load a file...'
            })
        );

        // MXSL editor – custom mxsl mode with real keywords
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
    })
    .catch(err => {
        logMessage('Failed to load keywords: ' + err.message, 'error');
        // Fallback: init editors with clike mode
        editorMtlx = CodeMirror.fromTextArea(
            document.getElementById('editor-mtlx'),
            Object.assign({}, sharedOptions, {
                theme: 'darcula',
                mode: 'xml',
                placeholder: 'Paste MTLX XML here, or load a file...'
            })
        );
        editorMxsl = CodeMirror.fromTextArea(
            document.getElementById('editor-mxsl'),
            Object.assign({}, sharedOptions, {
                theme: 'monokai',
                mode: 'text/x-csrc',
                placeholder: 'Paste MXSL code here, or load a file...'
            })
        );
    });

// ============================================================
//  Status helpers
// ============================================================

function logMessage(message, type) {
    const el = document.getElementById('message-log');
    const timestamp = new Date().toLocaleTimeString();
    const icons = {
        error: '[\u2716]',
        success: '[\u2714]',
        info: '[\u2139]'
    };
    const icon = icons[type] || '[\u2139]';
    const text = `[${timestamp}] ${icon} ${message}\n`;
    el.value += text;
    el.scrollTop = el.scrollHeight;
}

function clearLog() {
    const el = document.getElementById('message-log');
    el.value = '';
}

// Update the server-status badge in the log header.
function setServerStatus(label, icon, cls) {
    const el = document.getElementById('server-status');
    if (!el) return;
    el.innerHTML = `<i class="bi ${icon} me-1"></i>${label}`;
    el.className = 'server-status ' + (cls || '');
}

// ============================================================
//  Load file
// ============================================================

let pendingTarget = null;

function loadFile(target) {
    pendingTarget = target;
    const input = document.getElementById('file-input');
    input.accept = '.mtlx,.mxsl,.xml,.txt';
    input.value = '';
    input.click();
}

document.getElementById('file-input').addEventListener('change', function(e) {
    const file = e.target.files[0];
    if (!file || !pendingTarget) return;

    logMessage('Loading ' + file.name + '…', 'info');
    const reader = new FileReader();
    reader.onload = function(ev) {
        const content = ev.target.result;
        if (pendingTarget === 'mtlx') {
            editorMtlx.setValue(content);
            logMessage('Loaded ' + file.name + ' into MTLX editor', 'success');
        } else {
            editorMxsl.setValue(content);
            logMessage('Loaded ' + file.name + ' into MXSL editor', 'success');
        }
        pendingTarget = null;
    };
    reader.onerror = function() {
        logMessage('Failed to read file', 'error');
        pendingTarget = null;
    };
    reader.readAsText(file);
});

function toggleOptions() {
    const body = document.getElementById('options-body');
    const label = document.getElementById('options-toggle-label');
    const isHidden = body.classList.contains('d-none');
    body.classList.toggle('d-none');
    label.textContent = isHidden ? 'hide' : 'show';
}

function getCompileOptions() {
    return {
        version: document.getElementById('opt-version').value,
        reduce_graph: document.getElementById('opt-reduce-graph').checked,
        error_on_missing_globals: document.getElementById('opt-error-missing').checked,
        error_on_unused_globals: document.getElementById('opt-error-unused').checked,
    };
}

// ============================================================
//  Convert: MTLX → MXSL (decompile)
// ============================================================

async function convertMtlxToMxsl() {
    const source = editorMtlx.getValue().trim();
    if (!source) {
        logMessage('MTLX editor is empty', 'error');
        return;
    }

    logMessage('Decompiling MTLX → MXSL…', 'info');
    try {
        const resp = await fetch('/api/decompile', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ source })
        });
        const data = await resp.json();
        if (data.error) {
            logMessage('Decompile error: ' + data.error, 'error');
            return;
        }
        editorMxsl.setValue(data.result);
        logMessage('MTLX decompiled to MXSL (' + data.result.length + ' chars)', 'success');
    } catch (err) {
        logMessage('Network error: ' + err.message, 'error');
    }
}

// ===================== =======================================
//  Convert: MXSL → MTLX (compile)
// ============================================================

async function convertMxslToMtlx() {
    const source = editorMxsl.getValue().trim();
    if (!source) {
        logMessage('MXSL editor is empty', 'error');
        return;
    }

    const options = getCompileOptions();
    logMessage('Compiling MXSL → MTLX (version=' + options.version + ', reduce=' + options.reduce_graph + ')…', 'info');
    try {
        const resp = await fetch('/api/compile', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ source, options })
        });
        const data = await resp.json();
        if (data.error) {
            logMessage('Compile error: ' + data.error, 'error');
            return;
        }
        editorMtlx.setValue(data.result);
        logMessage('MXSL compiled to MTLX (' + data.result.length + ' chars)', 'success');
    } catch (err) {
        logMessage('Network error: ' + err.message, 'error');
    }
}

// ============================================================
//  Bootstrap
// ============================================================

// The Flask backend is already serving this page, so there is no async module
// to wait for — mark the server ready immediately on load.
setServerStatus('Flask ready', 'bi-check-circle-fill', 'text-success');
    
