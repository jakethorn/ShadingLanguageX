// The Outline view and breadcrumbs: functions, classes, type aliases, globals, constants and shaders.
import * as vscode from 'vscode';
import type { SymbolIndex } from './symbolIndex';
import type { SlxSymbol } from './symbols';

const SHADER_TYPES = new Set(['material', 'surfaceshader', 'displacementshader', 'volumeshader', 'lightshader']);

function symbolKind(symbol: SlxSymbol): vscode.SymbolKind | undefined {
    switch (symbol.kind) {
        case 'function':
            return vscode.SymbolKind.Function;
        case 'method':
            return vscode.SymbolKind.Method;
        case 'constructor':
            return vscode.SymbolKind.Constructor;
        case 'class':
            return vscode.SymbolKind.Class;
        case 'alias':
            return vscode.SymbolKind.Struct;
        case 'field':
            return vscode.SymbolKind.Field;
        case 'variable':
            if (symbol.modifiers.includes('global')) return vscode.SymbolKind.Variable;
            if (symbol.modifiers.includes('const')) return vscode.SymbolKind.Constant;
            if (symbol.type && SHADER_TYPES.has(symbol.type)) return vscode.SymbolKind.Object;
            return undefined;
        default:
            return undefined;
    }
}

function symbolDetail(symbol: SlxSymbol): string {
    if (symbol.parameters) return symbol.signature.slice(symbol.signature.indexOf('('));
    if (symbol.kind === 'variable' || symbol.kind === 'field') return symbol.signature.slice(0, -symbol.name.length).trim();
    return '';
}

export class SlxDocumentSymbolProvider implements vscode.DocumentSymbolProvider {
    constructor(private readonly index: SymbolIndex) {}

    provideDocumentSymbols(document: vscode.TextDocument): vscode.DocumentSymbol[] {
        const { scanned } = this.index.index(document).file;
        const toDocumentSymbol = (symbol: SlxSymbol): vscode.DocumentSymbol | undefined => {
            const kind = symbolKind(symbol);
            if (kind === undefined) return undefined;
            const range = new vscode.Range(document.positionAt(symbol.start), document.positionAt(symbol.end));
            const selection = new vscode.Range(document.positionAt(symbol.nameStart), document.positionAt(symbol.nameEnd));
            const result = new vscode.DocumentSymbol(symbol.name, symbolDetail(symbol), kind, range.union(selection), selection);
            result.children = (symbol.members ?? []).map(toDocumentSymbol).filter((s): s is vscode.DocumentSymbol => s !== undefined);
            return result;
        };
        return scanned.symbols.map(toDocumentSymbol).filter((s): s is vscode.DocumentSymbol => s !== undefined);
    }
}
