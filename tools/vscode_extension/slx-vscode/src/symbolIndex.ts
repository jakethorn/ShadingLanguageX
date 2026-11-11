// The declarations a document can use: its own and those of the files it
// includes, scanned once per version of each file's text.
import * as vscode from 'vscode';
import { collectDocumentSources } from './documents';
import { findVersionDirective, type NodeSignature, type SlxLanguage } from './languageData';
import { parseNodeDefs, type LibraryNode } from './libraryNodes';
import { readSettings } from './settings';
import { scanSymbols, type ScannedSource, type SlxSymbol } from './symbols';

export interface IndexedFile {
    uri: vscode.Uri;
    /** The file's name in the include tree, e.g. "sub/colors.mxsl"; empty for the document itself. */
    key: string;
    text: string;
    scanned: ScannedSource;
}

/** A MaterialX file the document loads with #library. */
export interface LibraryFile {
    uri: vscode.Uri;
    /** The file's name in the include tree, e.g. "nodes.mtlx". */
    key: string;
    text: string;
    nodes: LibraryNode[];
}

export interface DocumentIndex {
    file: IndexedFile;
    /** The SLX files the document includes, directly or through other includes. */
    includes: IndexedFile[];
    /** The MaterialX files the document and its includes load with #library. */
    libraries: LibraryFile[];
    /** The MaterialX version the document compiles against. */
    version: string;
}

/** A declaration, with the file it is in. */
export interface LocatedSymbol {
    symbol: SlxSymbol;
    file: IndexedFile;
}

/** Converts an offset in a file's text to a position, for files that may not be open. */
export function positionAt(text: string, offset: number): vscode.Position {
    let line = 0;
    let lineStart = 0;
    for (let i = text.indexOf('\n'); i !== -1 && i < offset; i = text.indexOf('\n', i + 1)) {
        line++;
        lineStart = i + 1;
    }
    return new vscode.Position(line, offset - lineStart);
}

export function rangeOf(file: IndexedFile, start: number, end: number): vscode.Range {
    return new vscode.Range(positionAt(file.text, start), positionAt(file.text, end));
}

export class SymbolIndex implements vscode.Disposable {
    private readonly scans = new Map<string, { text: string; scanned: ScannedSource }>();
    private readonly libraryNodes = new Map<string, { text: string; nodes: LibraryNode[] }>();
    private readonly closeListener: vscode.Disposable;

    constructor(readonly language: SlxLanguage | undefined) {
        this.closeListener = vscode.workspace.onDidCloseTextDocument((document) => {
            this.scans.delete(document.uri.toString());
            this.libraryNodes.delete(document.uri.toString());
        });
    }

    dispose(): void {
        this.closeListener.dispose();
    }

    private scan(uri: vscode.Uri, text: string): ScannedSource {
        const key = uri.toString();
        const cached = this.scans.get(key);
        if (cached && cached.text === text) return cached.scanned;
        const scanned = scanSymbols(text);
        this.scans.set(key, { text, scanned });
        return scanned;
    }

    private parseLibrary(uri: vscode.Uri, text: string): LibraryNode[] {
        const key = uri.toString();
        const cached = this.libraryNodes.get(key);
        if (cached && cached.text === text) return cached.nodes;
        const nodes = parseNodeDefs(text);
        this.libraryNodes.set(key, { text, nodes });
        return nodes;
    }

    index(document: vscode.TextDocument): DocumentIndex {
        const text = document.getText();
        const settings = readSettings(document.uri);
        const file: IndexedFile = { uri: document.uri, key: '', text, scanned: this.scan(document.uri, text) };

        const seen = new Set<string>();
        const includes: IndexedFile[] = [];
        const libraries: LibraryFile[] = [];
        for (const source of collectDocumentSources(document, text, settings.includePaths).sources) {
            if (seen.has(source.filePath)) continue;
            seen.add(source.filePath);
            const uri = vscode.Uri.file(source.filePath);
            if (source.origin.directive.kind === 'library') {
                libraries.push({ uri, key: source.key, text: source.text, nodes: this.parseLibrary(uri, source.text) });
            } else {
                includes.push({ uri, key: source.key, text: source.text, scanned: this.scan(uri, source.text) });
            }
        }

        const requested = findVersionDirective(text) ?? settings.compile.materialxVersion;
        const version = this.language ? this.language.resolveVersion(requested) : requested;
        return { file, includes, libraries, version };
    }
}

/** A node from a #library file, with the file. */
export interface LocatedNode {
    node: LibraryNode;
    file: LibraryFile;
}

/** The nodes with this name in the document's #library files. */
export function findLibraryNodes(index: DocumentIndex, name: string): LocatedNode[] {
    return index.libraries.flatMap((file) => file.nodes.filter((node) => node.name === name).map((node) => ({ node, file })));
}

/** The signatures of the nodes with this name: the MaterialX standard ones, then those from #library files. */
export function findNodeSignatures(index: DocumentIndex, language: SlxLanguage | undefined, name: string): NodeSignature[] {
    return [...(language?.nodeSignatures(name, index.version) ?? []), ...findLibraryNodes(index, name).map((l) => l.node.signature)];
}

/** Every declaration in the document and its includes that matches. */
export function findDeclarations(index: DocumentIndex, matches: (symbol: SlxSymbol) => boolean): LocatedSymbol[] {
    return [index.file, ...index.includes].flatMap((file) => file.scanned.all.filter(matches).map((symbol) => ({ symbol, file })));
}

/** Variables declared at the top level of the included files, which the document can use. */
export function findIncludedVariables(index: DocumentIndex): LocatedSymbol[] {
    return index.includes.flatMap((file) =>
        file.scanned.symbols.filter((s) => s.kind === 'variable').map((symbol) => ({ symbol, file })),
    );
}
