// Semantic highlighting of macros. A highlighting grammar only sees one line at a time, so it
// can't tell that SCALE in `t * SCALE` names a macro defined further up; this can.
import * as vscode from 'vscode';
import { PREDEFINED_MACROS, findDefinedMacros, findMacroUses } from './macros';
import type { SymbolIndex } from './symbolIndex';

export const SEMANTIC_TOKENS_LEGEND = new vscode.SemanticTokensLegend(['macro'], ['declaration', 'defaultLibrary']);

export class SlxSemanticTokensProvider implements vscode.DocumentSemanticTokensProvider, vscode.Disposable {
    private readonly changes = new vscode.EventEmitter<void>();
    /** Fired when an SLX file changes, since a macro defined in an included file affects the files including it. */
    readonly onDidChangeSemanticTokens = this.changes.event;
    private readonly listener: vscode.Disposable;

    constructor(private readonly index: SymbolIndex) {
        this.listener = vscode.workspace.onDidChangeTextDocument((event) => {
            if (event.document.languageId === 'slx' && event.contentChanges.length > 0) this.changes.fire();
        });
    }

    dispose(): void {
        this.listener.dispose();
        this.changes.dispose();
    }

    provideDocumentSemanticTokens(document: vscode.TextDocument): vscode.SemanticTokens {
        const indexed = this.index.index(document);
        const macros = new Set([
            ...PREDEFINED_MACROS,
            ...findDefinedMacros(indexed.file.text),
            ...indexed.includes.flatMap((file) => findDefinedMacros(file.text)),
        ]);
        const builder = new vscode.SemanticTokensBuilder(SEMANTIC_TOKENS_LEGEND);
        for (const use of findMacroUses(indexed.file.scanned.lexed, macros)) {
            const position = document.positionAt(use.start);
            const modifiers = (use.isDefinition ? 1 : 0) | (use.isPredefined ? 2 : 0);
            builder.push(position.line, position.character, use.end - use.start, 0, modifiers);
        }
        return builder.build();
    }
}
