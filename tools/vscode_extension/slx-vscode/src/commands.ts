// The SLX: Compile to MaterialX, SLX: Compile to MaterialX As... and SLX: Decompile to SLX commands.
import * as os from 'node:os';
import * as path from 'node:path';
import * as vscode from 'vscode';
import { CompiledFiles, exists, isVisible, writeCompiledFile } from './compiledFiles';
import { createCommandLogger } from './compilerOutput';
import type { SlxDiagnostics } from './diagnostics';
import { isMaterialXDocument, isSlxDocument } from './documents';
import type { EngineError } from './engine';
import { toSaveDialogDefault } from './saveDialog';

const SHOW_PROBLEMS = 'Show Problems';
const SHOW_OUTPUT = 'Show Output';

function describeDocument(document: vscode.TextDocument): string {
    return document.isUntitled ? document.uri.path : path.basename(document.fileName);
}

function findActiveDocument(): vscode.TextDocument | undefined {
    return vscode.window.activeTextEditor?.document;
}

/**
 * Where the save dialog starts for a file that hasn't been compiled before: the source's folder and name, with a .mtlx extension. An
 * unsaved file has no folder, so it starts in the workspace folder, or the home folder.
 */
function findDefaultOutputUri(document: vscode.TextDocument): vscode.Uri {
    if (document.isUntitled) {
        const folder = vscode.workspace.workspaceFolders?.[0]?.uri ?? vscode.Uri.file(os.homedir());
        return vscode.Uri.joinPath(folder, `${path.posix.basename(document.uri.path)}.mtlx`);
    }
    return document.uri.with({ path: document.uri.path.replace(/(\.[^./]*)?$/, '.mtlx') });
}

function isLocalWindows(): boolean {
    return process.platform === 'win32' && vscode.env.remoteName === undefined;
}

async function openBeside(content: string, language: string): Promise<void> {
    const document = await vscode.workspace.openTextDocument({ content, language });
    await vscode.window.showTextDocument(document, { viewColumn: vscode.ViewColumn.Beside, preview: true });
}

async function runWithStatus<T>(message: string, task: () => Promise<T>): Promise<T> {
    const status = vscode.window.setStatusBarMessage(`$(sync~spin) ${message}`);
    try {
        return await task();
    } finally {
        status.dispose();
    }
}

async function showFailure(log: vscode.LogOutputChannel, action: string, document: vscode.TextDocument, error: EngineError): Promise<void> {
    log.error(`${action} ${describeDocument(document)} failed: ${error.message}`);
    const firstLine = error.message.split('\n')[0];
    const actions = error.isEngineFailure ? [SHOW_OUTPUT] : [SHOW_PROBLEMS, SHOW_OUTPUT];
    const choice = await vscode.window.showErrorMessage(`SLX: ${firstLine}`, ...actions);
    if (choice === SHOW_PROBLEMS) await vscode.commands.executeCommand('workbench.actions.view.problems');
    else if (choice === SHOW_OUTPUT) log.show(true);
}

export function registerCommands(diagnostics: SlxDiagnostics, workspaceState: vscode.Memento, log: vscode.LogOutputChannel): vscode.Disposable[] {
    const compilerLog = createCommandLogger(log);
    const compiledFiles = new CompiledFiles(workspaceState);

    /** Compiles the active SLX file. With chooseFile, or if it hasn't been compiled before, asks where to save it. */
    const compile = async (chooseFile: boolean) => {
        const document = findActiveDocument();
        if (!document || !isSlxDocument(document)) {
            void vscode.window.showWarningMessage('SLX: open an .mxsl file to compile it.');
            return;
        }
        const result = await runWithStatus(`SLX: compiling ${describeDocument(document)}`, () => diagnostics.check(document, compilerLog));
        if (result.output === undefined) {
            await showFailure(log, 'Compiling', document, result.error!);
            return;
        }
        const previous = compiledFiles.find(document.uri);
        let target = !chooseFile && previous && (await exists(previous)) ? previous : undefined;
        target ??= await vscode.window.showSaveDialog({
            title: `Save the MaterialX Compiled from ${describeDocument(document)}`,
            // A plain object rather than a vscode.Uri, for UNC paths: see toSaveDialogDefault.
            defaultUri: toSaveDialogDefault(previous ?? findDefaultOutputUri(document), isLocalWindows()) as vscode.Uri,
            filters: { MaterialX: ['mtlx'] },
        });
        if (!target) return;
        const name = target.scheme === 'file' ? vscode.workspace.asRelativePath(target) : target.toString();
        try {
            if (!(await writeCompiledFile(target, result.output))) return;
        } catch (error) {
            log.error(`Couldn't write ${name}: ${error instanceof Error ? error.message : String(error)}`);
            void vscode.window.showErrorMessage(`SLX: couldn't save ${name}.`, SHOW_OUTPUT).then((choice) => {
                if (choice === SHOW_OUTPUT) log.show(true);
            });
            return;
        }
        await compiledFiles.remember(document.uri, target);
        log.info(`Compiled ${describeDocument(document)} to ${name}.`);
        // An open editor shows the change, so a message in the status bar is enough.
        if (isVisible(target)) {
            vscode.window.setStatusBarMessage(`$(check) SLX: compiled to ${name}`, 5000);
            return;
        }
        const open = 'Open';
        void vscode.window.showInformationMessage(`SLX: compiled to ${name}.`, open).then((choice) => {
            if (choice === open) void vscode.window.showTextDocument(target, { viewColumn: vscode.ViewColumn.Beside });
        });
    };

    const decompile = async () => {
        const document = findActiveDocument();
        if (!document || !isMaterialXDocument(document)) {
            void vscode.window.showWarningMessage('SLX: open a .mtlx file to decompile it.');
            return;
        }
        const result = await runWithStatus(`SLX: decompiling ${describeDocument(document)}`, () =>
            diagnostics.checkMaterialX(document, compilerLog),
        );
        if (result.output === undefined) {
            await showFailure(log, 'Decompiling', document, result.error!);
            return;
        }
        log.info(`Decompiled ${describeDocument(document)} (${result.output.length} characters).`);
        await openBeside(result.output, 'slx');
    };

    return [
        vscode.commands.registerCommand('slx.compile', () => compile(false)),
        vscode.commands.registerCommand('slx.compileAs', () => compile(true)),
        vscode.commands.registerCommand('slx.decompile', decompile),
    ];
}
