// MXSL VS Code extension entry point.
// Two commands backed by the bundled WebAssembly engine:
//   mxsl.convert  - convert the open file between MXSL and MTLX, show result in a new editor
//   mxsl.validate - run the conversion without creating a new file (success/error only)
const vscode = require('vscode');
const path = require('path');
const { pathToFileURL } = require('url');

// Lazily-loaded WebAssembly engine, shared across conversions.
let enginePromise = null;

// Persistent output channel that logs conversion results and errors.
let outputChannel = null;

function activate(context) {
    context.subscriptions.push(
        vscode.commands.registerCommand('mxsl.convert', () => convertActiveFile(context)),
        vscode.commands.registerCommand('mxsl.validate', () => validateActiveFile(context))
    );
}

function deactivate() {}

function getLibDir(context) {
    return path.join(context.extensionUri.fsPath, 'media', 'lib');
}

// Load (once) and return the mxslc WebAssembly engine. The locateFile callback
// must return an absolute filesystem path so the bundled .wasm / .data files
// are found regardless of the process working directory (this is what prevents
// the load from hanging on the data package).
async function loadEngine(context) {
    if (!enginePromise) {
        enginePromise = (async () => {
            const libDir = getLibDir(context);
            const mod = await import(pathToFileURL(path.join(libDir, 'JsMxslc.mjs')).href);
            const factory = mod.default;
            return factory({ locateFile: (file) => path.join(libDir, file) });
        })();
    }
    return enginePromise;
}

function errorMessage(err) {
    return (err && err.message) ? err.message : String(err);
}

// Lazy Output Channel under View -> Output -> MXSL.
function getOutputChannel() {
    if (!outputChannel) {
        outputChannel = vscode.window.createOutputChannel('MXSL');
    }
    return outputChannel;
}

// Append a timestamped line to the output channel.
function log(prefix, ...parts) {
    const line = '[' + new Date().toLocaleTimeString() + '] ' + prefix + ' ' +
        parts.join(' ').replace(/\s+$/g, '');
    getOutputChannel().appendLine(line);
    return line;
}

// Build a full, multi-line error description (name + message + stack).
function errorDetail(err) {
    const name = (err && err.name) ? err.name : 'Error';
    let detail = name + ': ' + errorMessage(err);
    if (err && err.stack) {
        detail += '\n' + err.stack;
    }
    return detail;
}

// Determine the conversion direction for a document, or null if unsupported.
function getKind(doc) {
    const ext = path.extname(doc.fileName).toLowerCase();
    const lang = doc.languageId;
    const isMxsl = lang === 'mxsl' || ext === '.mxsl';
    const isMtlx = lang === 'xml' || ext === '.mtlx' || ext === '.xml';
    if (!isMxsl && !isMtlx) return null;
    return { isMxsl, isMtlx, label: isMxsl ? 'MXSL → MTLX' : 'MTLX → MXSL' };
}

// Resolve the active editor and its conversion kind, warning if not usable.
async function getActiveTarget() {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
        vscode.window.showWarningMessage('MXSL: open a .mxsl or .mtlx file first.');
        return null;
    }
    const doc = editor.document;
    const kind = getKind(doc);
    if (!kind) {
        vscode.window.showWarningMessage(
            'MXSL: unsupported file type "' + path.extname(doc.fileName).toLowerCase() +
            '". Open a .mxsl or .mtlx file.'
        );
        return null;
    }
    return { editor, doc, kind };
}

// Run the conversion and return the resulting string.
function runConversion(mx, kind, source) {
    if (kind.isMxsl) {
        const opts = new mx.CompileOptions();
        opts.version = '1.39.5';
        opts.reduceGraph = true;
        opts.errorOnMissingGlobals = true;
        opts.errorOnUnusedGlobals = true;
        try {
            return mx.compileSlxToMtlx(source, opts);
        } finally {
            opts.delete();
        }
    }
    return mx.decompileMtlxToSlx(source);
}

// Log an error to the output channel and show a toast with a 'Show Output' action.
async function reportError(prefix, e) {
    const detail = errorDetail(e);
    log('ERROR', detail);
    const action = await vscode.window.showErrorMessage(
        prefix + ': ' + errorMessage(e),
        'Show Output'
    );
    if (action === 'Show Output') {
        getOutputChannel().show();
    }
}

// Convert the open file and show the result in a new untitled editor.
async function convertActiveFile(context) {
    const target = await getActiveTarget();
    if (!target) return;
    const { doc, kind } = target;
    const status = vscode.window.setStatusBarMessage('MXSL: ' + kind.label + ' ...');
    try {
        const mx = await loadEngine(context);
        const result = runConversion(mx, kind, doc.getText());
        const outDoc = await vscode.workspace.openTextDocument({
            content: result,
            language: kind.isMxsl ? 'xml' : 'mxsl'
        });
        await vscode.window.showTextDocument(outDoc, {
            preview: true,
            viewColumn: vscode.ViewColumn.Beside
        });
        log('OK', kind.label, '→', result.length, 'chars');
        vscode.window.showInformationMessage('MXSL: converted ' + kind.label + '.');
    } catch (e) {
        await reportError('MXSL conversion failed', e);
    } finally {
        status.dispose();
    }
}

// Validate the open file: run the conversion but do NOT create a new file.
async function validateActiveFile(context) {
    const target = await getActiveTarget();
    if (!target) return;
    const { doc, kind } = target;
    const status = vscode.window.setStatusBarMessage('MXSL: validating ' + kind.label + ' ...');
    try {
        const mx = await loadEngine(context);
        const result = runConversion(mx, kind, doc.getText());
        log('VALIDATE OK', kind.label, '→', result.length, 'chars');
        vscode.window.showInformationMessage(
            'MXSL: validation passed (' + kind.label + ', ' + result.length + ' chars).'
        );
    } catch (e) {
        await reportError('MXSL validation failed', e);
    } finally {
        status.dispose();
    }
}

module.exports = { activate, deactivate };
