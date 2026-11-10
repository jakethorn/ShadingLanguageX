// Code completion: argument names, variables, functions, MaterialX nodes, types and keywords, and directives, the
// paths in #include and #library directives, and attribute names.
import * as path from 'node:path';
import * as vscode from 'vscode';
import { findCallContext, type CallContext } from './callContext';
import { listDirectiveEntries, type DirectiveKind } from './includes';
import type { NodeInput } from './languageData';
import { isInCommentOrString } from './lexer';
import { describeInput, formatDeclarationHover, formatNodeHover } from './signatures';
import { readSettings } from './settings';
import { findDeclarations, findIncludedVariables, findNodeSignatures, type DocumentIndex, type SymbolIndex } from './symbolIndex';
import { findVisibleVariables, type SlxSymbol } from './symbols';

// Groups shown in this order when items match equally well.
const SORT_ARGUMENT = '0';
const SORT_VARIABLE = '1';
const SORT_DECLARATION = '2';
const SORT_BUILTIN = '3';
const SORT_LANGUAGE = '4';

/** Characters that open completion as they are typed: directives, their paths, and attribute names. */
export const COMPLETION_TRIGGER_CHARACTERS = ['#', '"', '/', '@'];

const DIRECTIVES = ['include', 'library', 'version', 'define', 'undef', 'ifdef', 'ifndef', 'if', 'elif', 'elifdef', 'elifndef', 'else', 'endif'];

/** Common MaterialX attributes, completed after @. Any name can be written. */
const ATTRIBUTES: Record<string, string> = {
    doc: 'Documentation.',
    uiname: 'The name shown in user interfaces.',
    uifolder: 'The folder an input is shown in, such as "Base/Color".',
    uimin: 'The smallest value user interfaces allow.',
    uimax: 'The largest value user interfaces allow.',
    uisoftmin: 'The smallest value of user interface sliders.',
    uisoftmax: 'The largest value of user interface sliders.',
    uistep: 'The step of user interface sliders.',
    uiadvanced: '"true" to show an input only in advanced views.',
    uivisible: '"false" to hide an input in user interfaces.',
    colorspace: 'The colour space of a value or file, such as "srgb_texture".',
    unit: 'The unit of a value, such as "meter".',
    unittype: 'The kind of unit, such as "distance".',
    enum: 'The values an input accepts, separated by commas.',
    enumvalues: 'The values that the enum names stand for, separated by commas.',
    defaultgeomprop: 'The geometric property an input defaults to, such as "Nworld".',
    nodegroup: 'The group of a node definition, such as "math".',
};

/** Attributes of the MaterialX document, completed after @@. */
const DOCUMENT_ATTRIBUTES: Record<string, string> = {
    colorspace: 'The colour space of the values and files in the document.',
    namespace: 'The namespace of the elements in the document.',
    doc: 'Documentation.',
};

const TRIGGER_SUGGEST: vscode.Command = { command: 'editor.action.triggerSuggest', title: 'Suggest' };

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

/** The directive names, after a # at the start of a line. #include and #library go on to complete a path. */
function completeDirectives(range: vscode.Range): vscode.CompletionItem[] {
    return DIRECTIVES.map((name) => {
        const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Keyword);
        item.range = range;
        if (name === 'include' || name === 'library') {
            item.insertText = new vscode.SnippetString(`${name} "$1"`);
            item.command = TRIGGER_SUGGEST;
        }
        return item;
    });
}

function completeAttributes(attributes: Record<string, string>, range: vscode.Range): vscode.CompletionItem[] {
    return Object.entries(attributes).map(([name, documentation]) => {
        const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Property);
        item.range = range;
        item.insertText = new vscode.SnippetString(`${name} "$1"`);
        item.documentation = documentation;
        return item;
    });
}

export class SlxCompletionProvider implements vscode.CompletionItemProvider<vscode.CompletionItem> {
    constructor(private readonly index: SymbolIndex) {}

    provideCompletionItems(
        document: vscode.TextDocument,
        position: vscode.Position,
        _token?: vscode.CancellationToken,
        context?: vscode.CompletionContext,
    ): vscode.CompletionItem[] | undefined {
        const trigger = context?.triggerKind === vscode.CompletionTriggerKind.TriggerCharacter ? context.triggerCharacter : undefined;
        const linePrefix = document.lineAt(position.line).text.slice(0, position.character);
        /** Replaces what has been typed of the name or path being completed. */
        const typedRange = (length: number) => new vscode.Range(position.line, position.character - length, position.line, position.character);

        const pathDirective = /^\s*#\s*(include|library)\s+"([^"]*)$/.exec(linePrefix);
        if (pathDirective) return this.completePath(document, pathDirective[1] as DirectiveKind, pathDirective[2], typedRange);
        const directive = /^\s*#\s*(\w*)$/.exec(linePrefix);
        if (directive) return completeDirectives(typedRange(directive[1].length));
        if (/^\s*#/.test(linePrefix) || (trigger !== undefined && trigger !== '@')) return undefined;

        const indexed = this.index.index(document);
        const offset = document.offsetAt(position);
        const lexed = indexed.file.scanned.lexed;
        if (isInCommentOrString(lexed, offset)) return undefined;

        // @name, @port.name or @@name.
        const attribute = /(@@?)(?:[A-Za-z_][\w-]*\.)?([\w-]*)$/.exec(linePrefix);
        if (attribute) return completeAttributes(attribute[1] === '@@' ? DOCUMENT_ATTRIBUTES : ATTRIBUTES, typedRange(attribute[2].length));
        if (trigger === '@') return undefined;
        // Fields, ports and swizzles after a "." need types to complete.
        if (/\.\s*\w*$/.test(linePrefix)) return undefined;

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

    /** The folders, and the .mxsl or .mtlx files, that the path in an #include or #library directive can continue with. */
    private completePath(
        document: vscode.TextDocument,
        kind: DirectiveKind,
        typed: string,
        typedRange: (length: number) => vscode.Range,
    ): vscode.CompletionItem[] {
        const folderPart = typed.slice(0, typed.lastIndexOf('/') + 1);
        const range = typedRange(typed.length - folderPart.length);
        const filePath = document.uri.scheme === 'file' ? document.uri.fsPath : undefined;
        const searched = [...(filePath ? [path.dirname(filePath)] : []), ...readSettings(document.uri).includePaths];
        const folders = path.isAbsolute(folderPart) ? [folderPart] : searched.map((folder) => path.join(folder, folderPart));
        return listDirectiveEntries(folders, kind === 'include' ? '.mxsl' : '.mtlx', filePath && path.resolve(filePath)).map((entry) => {
            const item = new vscode.CompletionItem(
                entry.isFolder ? `${entry.name}/` : entry.name,
                entry.isFolder ? vscode.CompletionItemKind.Folder : vscode.CompletionItemKind.File,
            );
            item.range = range;
            // Files first, then folders, which open the suggestions again for what is in them.
            item.sortText = (entry.isFolder ? '1' : '0') + entry.name;
            if (entry.isFolder) item.command = TRIGGER_SUGGEST;
            return item;
        });
    }

    /** `name = ` for each parameter of the function being called that isn't passed yet. */
    private addArgumentNames(indexed: DocumentIndex, call: CallContext, items: vscode.CompletionItem[]): void {
        const inputs = new Map<string, NodeInput>();
        for (const signature of findNodeSignatures(indexed, this.index.language, call.name)) {
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
        for (const file of indexed.libraries) {
            for (const name of new Set(file.nodes.map((node) => node.name))) {
                const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Function);
                item.detail = `Node from ${file.key}`;
                const signatures = file.nodes.filter((node) => node.name === name).map((node) => node.signature);
                item.documentation = new vscode.MarkdownString(formatNodeHover(name, signatures, indexed.version, undefined, file.key));
                item.sortText = SORT_BUILTIN + name;
                add(item);
            }
        }
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
