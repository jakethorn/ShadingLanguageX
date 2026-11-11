// Hovers: the declaration and docs of variables, functions, MaterialX nodes and their inputs.
import * as vscode from 'vscode';
import { formatDeclarationHover, formatInputHover, formatNodeHover } from './signatures';
import {
    findDeclarations,
    findIncludedVariables,
    findLibraryNodes,
    findNodeSignatures,
    type DocumentIndex,
    type LocatedSymbol,
    type SymbolIndex,
} from './symbolIndex';
import { findVisibleVariables } from './symbols';
import { findWordContext, type WordContext } from './wordContext';

function describeOrigin(located: LocatedSymbol[]): string | undefined {
    const included = located.find((l) => l.file.key !== '');
    return included ? `Defined in \`${included.file.key}\`` : undefined;
}

export class SlxHoverProvider implements vscode.HoverProvider {
    constructor(private readonly index: SymbolIndex) {}

    provideHover(document: vscode.TextDocument, position: vscode.Position): vscode.Hover | undefined {
        const indexed = this.index.index(document);
        const context = findWordContext(document, position, indexed.file.scanned.lexed);
        if (!context) return undefined;
        const markdown = this.describe(indexed, context);
        return markdown ? new vscode.Hover(new vscode.MarkdownString(markdown), context.range) : undefined;
    }

    private describe(indexed: DocumentIndex, context: WordContext): string | undefined {
        const { word } = context;
        if (context.afterDot) {
            const members = findDeclarations(indexed, (s) => s.container !== undefined && s.kind !== 'constructor' && s.name === word);
            return members.length > 0 ? formatDeclarationHover(members.map((m) => m.symbol), describeOrigin(members)) : undefined;
        }
        if (context.argumentOf) return this.describeArgument(indexed, context.argumentOf.name, word);

        // The declaration itself, or a variable or parameter in scope.
        const scanned = indexed.file.scanned;
        const declared = scanned.all.find((s) => s.nameStart === context.start && s.kind !== 'function' && s.kind !== 'method');
        const variable = declared ?? findVisibleVariables(scanned, context.start).find((s) => s.name === word);
        if (variable) return formatDeclarationHover([variable]);
        const included = findIncludedVariables(indexed).filter((l) => l.symbol.name === word);
        if (included.length > 0) return formatDeclarationHover([included[0].symbol], describeOrigin(included));

        const declarations = findDeclarations(indexed, (s) => (s.kind === 'function' || s.kind === 'class' || s.kind === 'alias') && s.name === word);
        if (declarations.length > 0) return formatDeclarationHover(declarations.map((d) => d.symbol), describeOrigin(declarations));

        // MaterialX nodes, nodes from #library files and standard library functions, which can share a name.
        const language = this.index.language;
        const parts: string[] = [];
        const signatures = language?.nodeSignatures(word, indexed.version) ?? [];
        if (signatures.length > 0) parts.push(formatNodeHover(word, signatures, indexed.version, language!.specLink(word)));
        const library = findLibraryNodes(indexed, word);
        for (const file of new Set(library.map((l) => l.file))) {
            const fileSignatures = library.filter((l) => l.file === file).map((l) => l.node.signature);
            parts.push(formatNodeHover(word, fileSignatures, indexed.version, undefined, file.key));
        }
        const stdlib = language?.stdlibOverloads(word) ?? [];
        if (stdlib.length > 0) parts.push(formatDeclarationHover(stdlib, parts.length > 0 ? 'Also defined in the SLX standard library' : 'SLX standard library'));
        return parts.length > 0 ? parts.join('\n\n---\n\n') : undefined;
    }

    /** The parameter or node input passed by name, as in `base_color` in `standard_surface(base_color = c)`. */
    private describeArgument(indexed: DocumentIndex, functionName: string, name: string): string | undefined {
        const functions = findDeclarations(indexed, (s) => (s.kind === 'function' || s.kind === 'constructor') && s.name === functionName);
        for (const { symbol } of functions) {
            const parameter = symbol.parameters?.find((p) => p.name === name);
            if (parameter) return ['```slx', parameter.label, '```', parameter.doc ?? ''].join('\n').trim();
        }
        for (const signature of findNodeSignatures(indexed, this.index.language, functionName)) {
            const input = signature.inputs.find((i) => i.name === name);
            if (input) return formatInputHover(functionName, input);
        }
        return undefined;
    }
}
