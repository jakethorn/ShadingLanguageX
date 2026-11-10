// Where SLX files compile to, and writing the compiled MaterialX there.
import * as vscode from 'vscode';

const STATE_KEY = 'slx.compiledFiles';

/** A key for a file that ignores case on Windows, where VS Code may spell the same path differently. */
function toKey(uri: vscode.Uri): string {
    const key = uri.toString();
    return process.platform === 'win32' ? key.toLowerCase() : key;
}

/** The .mtlx file each SLX file was last compiled to, kept in the workspace's storage. */
export class CompiledFiles {
    constructor(private readonly state: vscode.Memento) {}

    private read(): Record<string, string> {
        return this.state.get<Record<string, string>>(STATE_KEY, {});
    }

    /** The file the source was last compiled to, if any. Unsaved sources are never remembered. */
    find(source: vscode.Uri): vscode.Uri | undefined {
        const target = source.scheme === 'untitled' ? undefined : this.read()[toKey(source)];
        return target === undefined ? undefined : vscode.Uri.parse(target);
    }

    async remember(source: vscode.Uri, target: vscode.Uri): Promise<void> {
        if (source.scheme === 'untitled') return;
        await this.state.update(STATE_KEY, { ...this.read(), [toKey(source)]: target.toString() });
    }
}

export async function exists(uri: vscode.Uri): Promise<boolean> {
    try {
        await vscode.workspace.fs.stat(uri);
        return true;
    } catch {
        return false;
    }
}

export function isVisible(uri: vscode.Uri): boolean {
    const key = toKey(uri);
    return vscode.window.visibleTextEditors.some((editor) => toKey(editor.document.uri) === key);
}

/**
 * Writes the compiled MaterialX to its file. A file open in VS Code is updated through its document, so its editors
 * show the new MaterialX at once, instead of when VS Code notices the file has changed. That can take a while, for
 * example for files in WSL opened through \\wsl.localhost.
 *
 * Returns false if the user chose not to replace unsaved changes. Throws if writing fails.
 */
export async function writeCompiledFile(target: vscode.Uri, content: string): Promise<boolean> {
    const key = toKey(target);
    const document = vscode.workspace.textDocuments.find((d) => toKey(d.uri) === key);
    if (!document) {
        await vscode.workspace.fs.writeFile(target, Buffer.from(content, 'utf8'));
        return true;
    }
    if (document.isDirty) {
        const replace = 'Replace';
        const choice = await vscode.window.showWarningMessage(
            `${vscode.workspace.asRelativePath(target)} has unsaved changes. Replace them with the compiled MaterialX?`,
            { modal: true },
            replace,
        );
        if (choice !== replace) return false;
    }
    if (document.getText() !== content) {
        const edit = new vscode.WorkspaceEdit();
        edit.replace(document.uri, new vscode.Range(document.positionAt(0), document.positionAt(document.getText().length)), content);
        if (!(await vscode.workspace.applyEdit(edit))) throw new Error('VS Code did not apply the edit');
    }
    if (document.isDirty && !(await document.save())) throw new Error('VS Code did not save the file');
    return true;
}
