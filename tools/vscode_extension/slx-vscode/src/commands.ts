// The SLX: Compile to MaterialX, SLX: Decompile to SLX and SLX: Validate commands.
import * as path from 'node:path';
import * as vscode from 'vscode';
import { createCommandLogger } from './compilerOutput';
import type { CheckResult, SlxDiagnostics } from './diagnostics';
import { isMaterialXDocument, isSlxDocument } from './documents';
import type { EngineError } from './engine';

const SHOW_PROBLEMS = 'Show Problems';
const SHOW_OUTPUT = 'Show Output';

function describeDocument(document: vscode.TextDocument): string {
    return document.isUntitled ? document.uri.path : path.basename(document.fileName);
}

function findActiveDocument(): vscode.TextDocument | undefined {
    return vscode.window.activeTextEditor?.document;
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

export function registerCommands(diagnostics: SlxDiagnostics, log: vscode.LogOutputChannel): vscode.Disposable[] {
    const compilerLog = createCommandLogger(log);

    const compile = async () => {
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
        log.info(`Compiled ${describeDocument(document)} (${result.output.length} characters).`);
        await openBeside(result.output, 'xml');
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

    const validate = async () => {
        const document = findActiveDocument();
        let result: CheckResult;
        if (document && isSlxDocument(document)) {
            result = await runWithStatus(`SLX: validating ${describeDocument(document)}`, () => diagnostics.check(document, compilerLog));
        } else if (document && isMaterialXDocument(document)) {
            result = await runWithStatus(`SLX: validating ${describeDocument(document)}`, () =>
                diagnostics.checkMaterialX(document, compilerLog),
            );
        } else {
            void vscode.window.showWarningMessage('SLX: open an .mxsl or .mtlx file to validate it.');
            return;
        }
        if (result.error) {
            await showFailure(log, 'Validating', document, result.error);
            return;
        }
        log.info(`Validated ${describeDocument(document)}: no errors.`);
        void vscode.window.showInformationMessage(`SLX: ${describeDocument(document)} has no errors.`);
    };

    return [
        vscode.commands.registerCommand('slx.compile', compile),
        vscode.commands.registerCommand('slx.decompile', decompile),
        vscode.commands.registerCommand('slx.validate', validate),
    ];
}
