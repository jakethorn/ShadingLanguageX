// slx.compileOnSave: writes the compiled MaterialX next to an SLX file when it is saved.
import * as vscode from 'vscode';
import type { SlxDiagnostics } from './diagnostics';
import { isSlxDocument } from './documents';
import { readSettings } from './settings';

/** The .mtlx file next to a source, with the same name. */
export function toOutputUri(source: vscode.Uri): vscode.Uri {
    return source.with({ path: source.path.replace(/(\.[^./]*)?$/, '.mtlx') });
}

export function registerCompileOnSave(diagnostics: SlxDiagnostics, log: vscode.LogOutputChannel): vscode.Disposable {
    return vscode.workspace.onDidSaveTextDocument(async (document) => {
        if (!isSlxDocument(document) || document.uri.scheme !== 'file' || !readSettings(document.uri).compileOnSave) return;
        const result = await diagnostics.check(document);
        const output = toOutputUri(document.uri);
        if (result.output === undefined) {
            log.warn(`Didn't write ${vscode.workspace.asRelativePath(output)}: ${document.fileName} has errors.`);
            return;
        }
        try {
            await vscode.workspace.fs.writeFile(output, Buffer.from(result.output, 'utf8'));
            log.info(`Wrote ${vscode.workspace.asRelativePath(output)}.`);
        } catch (error) {
            log.error(`Couldn't write ${output.fsPath}: ${error instanceof Error ? error.message : String(error)}`);
            void vscode.window.showErrorMessage(`SLX: couldn't write ${vscode.workspace.asRelativePath(output)}.`);
        }
    });
}
