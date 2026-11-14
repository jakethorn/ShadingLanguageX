// SLX: Open MaterialX Preview to the Side: a read-only view of the compiled
// MaterialX that updates as the source is edited.
import * as path from 'node:path';
import * as vscode from 'vscode';
import type { CheckEvent, SlxDiagnostics } from './diagnostics';
import { isSlxDocument } from './documents';

export const PREVIEW_SCHEME = 'slx-preview';

interface PreviewContent {
    /** The last MaterialX the source compiled to. */
    output?: string;
    /** The source has changed since and no longer compiles. */
    stale: boolean;
}

// The placeholders start with an XML declaration, as the compiled MaterialX does. VS Code chooses the preview's language
// when it opens, from the first line if no extension claims .mtlx files, and a declaration makes that XML.
const XML_DECLARATION = '<?xml version="1.0"?>\n';
const COMPILING = `${XML_DECLARATION}<!-- Compiling... -->\n`;
const HAS_ERRORS = `${XML_DECLARATION}<!-- The source has errors, so it can't be shown as MaterialX. See the Problems panel. -->\n`;
const STALE = '<!-- The source has errors, so this is the last version that compiled. See the Problems panel. -->';

/**
 * The preview of a source document. Its path ends in .mtlx, so an extension for MaterialX files, such as the MaterialX
 * Playground, can highlight it. Its query holds the source's URI.
 */
function toPreviewUri(source: vscode.Uri): vscode.Uri {
    const name = path.posix.basename(source.path).replace(/\.[^.]*$/, '');
    return vscode.Uri.from({ scheme: PREVIEW_SCHEME, path: `/${name}.mtlx`, query: source.toString() });
}

function toSourceUri(preview: vscode.Uri): vscode.Uri {
    return vscode.Uri.parse(preview.query);
}

/** Puts the comment after the XML declaration, where a comment is allowed. */
function insertComment(output: string, comment: string): string {
    const declarationEnd = output.startsWith('<?xml') ? output.indexOf('?>') + 2 : 0;
    return `${output.slice(0, declarationEnd)}\n${comment}${output.slice(declarationEnd)}`;
}

export class MaterialXPreview implements vscode.TextDocumentContentProvider, vscode.Disposable {
    private readonly changes = new vscode.EventEmitter<vscode.Uri>();
    readonly onDidChange = this.changes.event;
    /** Content of each open preview, by the source's URI. */
    private readonly contents = new Map<string, PreviewContent>();
    private readonly disposables: vscode.Disposable[];

    constructor(private readonly diagnostics: SlxDiagnostics) {
        this.disposables = [
            this.changes,
            vscode.workspace.registerTextDocumentContentProvider(PREVIEW_SCHEME, this),
            diagnostics.onDidCheck((event) => this.update(event)),
            vscode.workspace.onDidCloseTextDocument((document) => {
                if (document.uri.scheme !== PREVIEW_SCHEME) return;
                const source = toSourceUri(document.uri);
                this.contents.delete(source.toString());
                this.diagnostics.keepUpdated(source, false);
            }),
            vscode.commands.registerCommand('slx.showPreview', () => this.show()),
        ];
    }

    dispose(): void {
        for (const disposable of this.disposables) disposable.dispose();
    }

    provideTextDocumentContent(uri: vscode.Uri): string {
        const source = toSourceUri(uri);
        const content = this.contents.get(source.toString());
        if (!content) {
            // A preview restored from a previous session: compile its source.
            void this.track(source);
            return COMPILING;
        }
        if (content.output === undefined) return content.stale ? HAS_ERRORS : COMPILING;
        return content.stale ? insertComment(content.output, STALE) : content.output;
    }

    private async show(): Promise<void> {
        const document = vscode.window.activeTextEditor?.document;
        if (!document || !isSlxDocument(document)) {
            void vscode.window.showWarningMessage('SLX: open an .mxsl file to preview it.');
            return;
        }
        void this.track(document.uri);
        const preview = await vscode.workspace.openTextDocument(toPreviewUri(document.uri));
        await vscode.window.showTextDocument(preview, { viewColumn: vscode.ViewColumn.Beside, preserveFocus: true, preview: false });
    }

    /** Starts keeping the preview of a source up to date, and compiles the source now. */
    private async track(source: vscode.Uri): Promise<void> {
        const key = source.toString();
        if (!this.contents.has(key)) this.contents.set(key, { stale: false });
        this.diagnostics.keepUpdated(source, true);
        try {
            const document = await vscode.workspace.openTextDocument(source);
            await this.diagnostics.check(document);
        } catch {
            this.contents.set(key, { stale: true });
            this.changes.fire(toPreviewUri(source));
        }
    }

    private update({ document, result }: CheckEvent): void {
        const key = document.uri.toString();
        const previous = this.contents.get(key);
        if (!previous) return;
        this.contents.set(key, result.output !== undefined ? { output: result.output, stale: false } : { output: previous.output, stale: true });
        this.changes.fire(toPreviewUri(document.uri));
    }
}
