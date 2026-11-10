// Go to Definition (Ctrl+Click, F12) for SLX declarations and standard library functions, and links:
// #include and #library paths open their files, and MaterialX node names open the node's section
// of the MaterialX specification.
import * as fs from 'node:fs';
import * as vscode from 'vscode';
import { isCallName } from './callContext';
import { findCandidatePaths, findDirectives } from './includes';
import { readSettings } from './settings';
import {
    findDeclarations,
    findIncludedVariables,
    findLibraryNodes,
    positionAt,
    rangeOf,
    type LocatedSymbol,
    type SymbolIndex,
} from './symbolIndex';
import { findVisibleVariables } from './symbols';
import { findWordContext } from './wordContext';

function toLocation(located: LocatedSymbol): vscode.Location {
    return new vscode.Location(located.file.uri, rangeOf(located.file, located.symbol.nameStart, located.symbol.nameEnd));
}

function isFile(filePath: string): boolean {
    try {
        return fs.statSync(filePath).isFile();
    } catch {
        return false;
    }
}

export class SlxDefinitionProvider implements vscode.DefinitionProvider {
    constructor(private readonly index: SymbolIndex) {}

    provideDefinition(document: vscode.TextDocument, position: vscode.Position): vscode.Location[] | undefined {
        const indexed = this.index.index(document);
        const context = findWordContext(document, position, indexed.file.scanned.lexed);
        if (!context) return undefined;
        const { word } = context;

        let found: LocatedSymbol[];
        if (context.afterDot) {
            // Without types, every field and method with this name could be the one.
            found = findDeclarations(indexed, (s) => s.container !== undefined && s.kind !== 'constructor' && s.name === word);
        } else if (context.argumentOf) {
            const call = context.argumentOf;
            const functions = findDeclarations(indexed, (s) => (s.kind === 'function' || s.kind === 'constructor') && s.name === call.name);
            found = functions.flatMap(({ symbol, file }) =>
                file.scanned.all
                    .filter((p) => p.kind === 'parameter' && p.name === word && p.nameStart > symbol.start && p.nameEnd < symbol.end)
                    .map((p) => ({ symbol: p, file })),
            );
        } else {
            const variable = findVisibleVariables(indexed.file.scanned, context.start).find((s) => s.name === word);
            if (variable) {
                found = [{ symbol: variable, file: indexed.file }];
            } else {
                found = [
                    ...findIncludedVariables(indexed).filter((l) => l.symbol.name === word),
                    ...findDeclarations(indexed, (s) => (s.kind === 'function' || s.kind === 'class' || s.kind === 'alias') && s.name === word),
                ];
            }
        }
        if (found.length > 0) return found.map(toLocation);
        if (context.afterDot || context.argumentOf) return undefined;
        // Nodes from #library files: their <nodedef> elements.
        const libraryNodes = findLibraryNodes(indexed, word);
        if (libraryNodes.length > 0) {
            return libraryNodes.map(({ node, file }) => new vscode.Location(file.uri, positionAt(file.text, node.offset)));
        }
        return this.findStandardLibraryDefinitions(word, indexed.version);
    }

    /**
     * The definitions of a standard library function in the extension's copy of
     * stdlib.mxsl. Only for functions that aren't also MaterialX nodes: Ctrl+Click
     * on a node name follows its link to the specification instead.
     */
    private findStandardLibraryDefinitions(name: string, version: string): vscode.Location[] | undefined {
        const language = this.index.language;
        const stdlibFile = language?.stdlibFile;
        if (!language || !stdlibFile || language.nodeSignatures(name, version).length > 0) return undefined;
        const overloads = language.stdlibOverloads(name);
        if (overloads.length === 0) return undefined;
        const uri = vscode.Uri.file(stdlibFile.path);
        return overloads.map(
            (s) => new vscode.Location(uri, new vscode.Range(positionAt(stdlibFile.text, s.nameStart), positionAt(stdlibFile.text, s.nameEnd))),
        );
    }
}

export class SlxDocumentLinkProvider implements vscode.DocumentLinkProvider {
    constructor(private readonly index: SymbolIndex) {}

    provideDocumentLinks(document: vscode.TextDocument): vscode.DocumentLink[] {
        const settings = readSettings(document.uri);
        return [...this.findDirectiveLinks(document, settings.includePaths), ...(settings.nodeLinks ? this.findNodeLinks(document) : [])];
    }

    private findDirectiveLinks(document: vscode.TextDocument, includePaths: string[]): vscode.DocumentLink[] {
        const fromFilePath = document.uri.scheme === 'file' ? document.uri.fsPath : undefined;
        const links: vscode.DocumentLink[] = [];
        for (const directive of findDirectives(document.getText())) {
            const target = findCandidatePaths(directive.target, fromFilePath, includePaths).find(isFile);
            if (!target) continue;
            // The path inside the quotes.
            const range = new vscode.Range(directive.line, directive.start + 1, directive.line, directive.end - 1);
            const link = new vscode.DocumentLink(range, vscode.Uri.file(target));
            link.tooltip = `Open ${directive.target}`;
            links.push(link);
        }
        return links;
    }

    /** Calls of MaterialX nodes, linked to the node's section of the specification. */
    private findNodeLinks(document: vscode.TextDocument): vscode.DocumentLink[] {
        const language = this.index.language;
        if (!language) return [];
        const indexed = this.index.index(document);
        // Functions written in SLX hide the nodes with the same name.
        const declared = new Set(findDeclarations(indexed, (s) => s.kind === 'function').map((l) => l.symbol.name));
        const tokens = indexed.file.scanned.lexed.tokens;
        const links: vscode.DocumentLink[] = [];
        tokens.forEach((token, index) => {
            if (!isCallName(tokens, index) || tokens[index - 1]?.text === '.' || declared.has(token.text)) return;
            const specLink = language.specLink(token.text);
            if (!specLink || language.nodeSignatures(token.text, indexed.version).length === 0) return;
            const range = new vscode.Range(document.positionAt(token.start), document.positionAt(token.end));
            const link = new vscode.DocumentLink(range, vscode.Uri.parse(specLink));
            link.tooltip = `Open ${token.text} in the MaterialX specification`;
            links.push(link);
        });
        return links;
    }
}
