// Code completion: argument names, variables, functions, MaterialX nodes, types and keywords.
import * as vscode from 'vscode';
import { findCallContext, type CallContext } from './callContext';
import type { NodeInput } from './languageData';
import { isInCommentOrString } from './lexer';
import { describeInput, formatDeclarationHover, formatNodeHover } from './signatures';
import { findDeclarations, findIncludedVariables, type DocumentIndex, type SymbolIndex } from './symbolIndex';
import { findVisibleVariables, type SlxSymbol } from './symbols';

// Groups shown in this order when items match equally well.
const SORT_ARGUMENT = '0';
const SORT_VARIABLE = '1';
const SORT_DECLARATION = '2';
const SORT_BUILTIN = '3';
const SORT_LANGUAGE = '4';

/** A MaterialX node or standard library function, whose documentation is filled in when it is selected. */
class BuiltinCompletionItem extends vscode.CompletionItem {
    constructor(
        readonly functionName: string,
        readonly version: string,
    ) {
        super(functionName, vscode.CompletionItemKind.Function);
        this.sortText = SORT_BUILTIN + functionName;
    }
}

function declarationKind(symbol: SlxSymbol): vscode.CompletionItemKind {
    switch (symbol.kind) {
        case 'class':
            return vscode.CompletionItemKind.Class;
        case 'alias':
            return vscode.CompletionItemKind.Struct;
        case 'function':
            return vscode.CompletionItemKind.Function;
        default:
            return symbol.modifiers.includes('const') ? vscode.CompletionItemKind.Constant : vscode.CompletionItemKind.Variable;
    }
}

function declarationItem(symbols: SlxSymbol[], sortGroup: string): vscode.CompletionItem {
    const [first] = symbols;
    const item = new vscode.CompletionItem(first.name, declarationKind(first));
    item.detail = symbols.length > 1 ? `${first.signature} (+${symbols.length - 1} overloads)` : first.signature;
    if (first.doc) item.documentation = first.doc;
    item.sortText = sortGroup + first.name;
    return item;
}

export class SlxCompletionProvider implements vscode.CompletionItemProvider<vscode.CompletionItem> {
    constructor(private readonly index: SymbolIndex) {}

    provideCompletionItems(document: vscode.TextDocument, position: vscode.Position): vscode.CompletionItem[] | undefined {
        const indexed = this.index.index(document);
        const offset = document.offsetAt(position);
        const lexed = indexed.file.scanned.lexed;
        if (isInCommentOrString(lexed, offset)) return undefined;

        const linePrefix = document.lineAt(position.line).text.slice(0, position.character);
        if (/^\s*#/.test(linePrefix)) return undefined;
        // Attribute names, and fields, ports and swizzles after a "." need types to complete.
        if (/[@.]\s*\w*$/.test(linePrefix)) return undefined;

        const items: vscode.CompletionItem[] = [];
        const names = new Set<string>();
        const add = (item: vscode.CompletionItem) => {
            const label = typeof item.label === 'string' ? item.label : item.label.label;
            if (names.has(label)) return;
            names.add(label);
            items.push(item);
        };

        const call = findCallContext(lexed, offset);
        if (call?.atArgumentName) this.addArgumentNames(indexed, call, items);
        this.addVariables(indexed, offset, add);
        this.addDeclarations(indexed, add);
        this.addBuiltins(indexed, add);
        this.addLanguage(add);
        return items;
    }

    resolveCompletionItem(item: vscode.CompletionItem): vscode.CompletionItem {
        if (!(item instanceof BuiltinCompletionItem) || item.documentation) return item;
        const language = this.index.language!;
        const signatures = language.nodeSignatures(item.functionName, item.version);
        const markdown =
            signatures.length > 0
                ? formatNodeHover(item.functionName, signatures, item.version, language.specLink(item.functionName))
                : formatDeclarationHover(language.stdlibOverloads(item.functionName), 'SLX standard library');
        item.documentation = new vscode.MarkdownString(markdown);
        return item;
    }

    /** `name = ` for each parameter of the function being called that isn't passed yet. */
    private addArgumentNames(indexed: DocumentIndex, call: CallContext, items: vscode.CompletionItem[]): void {
        const inputs = new Map<string, NodeInput>();
        for (const signature of this.index.language?.nodeSignatures(call.name, indexed.version) ?? []) {
            for (const input of signature.inputs) if (!inputs.has(input.name)) inputs.set(input.name, input);
        }
        const parameters = new Map<string, string>();
        const functions = findDeclarations(indexed, (s) => (s.kind === 'function' || s.kind === 'constructor') && s.name === call.name);
        for (const { symbol } of functions) for (const p of symbol.parameters ?? []) if (!parameters.has(p.name)) parameters.set(p.name, p.label);

        const add = (name: string, detail: string, documentation?: string) => {
            if (call.usedNames.includes(name)) return;
            const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Field);
            item.insertText = `${name} = `;
            item.detail = detail;
            if (documentation) item.documentation = new vscode.MarkdownString(documentation);
            item.sortText = SORT_ARGUMENT + String(items.length).padStart(3, '0');
            items.push(item);
        };
        for (const [name, label] of parameters) add(name, label);
        for (const [name, input] of inputs) if (!parameters.has(name)) add(name, `${input.type} ${name}`, describeInput(input));
    }

    private addVariables(indexed: DocumentIndex, offset: number, add: (item: vscode.CompletionItem) => void): void {
        const variables = [...findVisibleVariables(indexed.file.scanned, offset), ...findIncludedVariables(indexed).map((l) => l.symbol)];
        for (const variable of variables) add(declarationItem([variable], SORT_VARIABLE));
    }

    private addDeclarations(indexed: DocumentIndex, add: (item: vscode.CompletionItem) => void): void {
        const byName = new Map<string, SlxSymbol[]>();
        for (const { symbol } of findDeclarations(indexed, (s) => s.kind === 'function' || s.kind === 'class' || s.kind === 'alias')) {
            byName.set(symbol.name, [...(byName.get(symbol.name) ?? []), symbol]);
        }
        for (const symbols of byName.values()) add(declarationItem(symbols, SORT_DECLARATION));
    }

    private addBuiltins(indexed: DocumentIndex, add: (item: vscode.CompletionItem) => void): void {
        const language = this.index.language;
        if (!language) return;
        for (const name of language.nodeNames(indexed.version)) {
            const item = new BuiltinCompletionItem(name, indexed.version);
            item.detail = 'MaterialX node';
            add(item);
        }
        for (const name of language.stdlibFunctionNames()) {
            const item = new BuiltinCompletionItem(name, indexed.version);
            item.detail = 'SLX standard library';
            add(item);
        }
    }

    private addLanguage(add: (item: vscode.CompletionItem) => void): void {
        const language = this.index.language;
        if (!language) return;
        for (const type of language.data.types) {
            const item = new vscode.CompletionItem(type, vscode.CompletionItemKind.TypeParameter);
            item.sortText = SORT_LANGUAGE + type;
            add(item);
        }
        for (const keyword of language.data.keywords.all) {
            const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
            item.sortText = SORT_LANGUAGE + keyword;
            add(item);
        }
    }
}
