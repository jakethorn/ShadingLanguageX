// Hovers: the declaration and docs of variables, functions, MaterialX nodes and their inputs.
import * as vscode from 'vscode';
import { formatDeclarationHover, formatInputHover, formatNodeHover } from './signatures';
import { findDeclarations, findIncludedVariables, type DocumentIndex, type LocatedSymbol, type SymbolIndex } from './symbolIndex';
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

        const language = this.index.language;
        if (!language) return undefined;
        const signatures = language.nodeSignatures(word, indexed.version);
        const stdlib = language.stdlibOverloads(word);
        if (signatures.length > 0) {
            const hover = formatNodeHover(word, signatures, indexed.version, language.specLink(word));
            return stdlib.length > 0 ? `${hover}\n\n---\n\n${formatDeclarationHover(stdlib, 'Also defined in the SLX standard library')}` : hover;
        }
        return stdlib.length > 0 ? formatDeclarationHover(stdlib, 'SLX standard library') : undefined;
    }

    /** The parameter or node input passed by name, as in `base_color` in `standard_surface(base_color = c)`. */
    private describeArgument(indexed: DocumentIndex, functionName: string, name: string): string | undefined {
        const functions = findDeclarations(indexed, (s) => (s.kind === 'function' || s.kind === 'constructor') && s.name === functionName);
        for (const { symbol } of functions) {
            const parameter = symbol.parameters?.find((p) => p.name === name);
            if (parameter) return ['```slx', parameter.label, '```', parameter.doc ?? ''].join('\n').trim();
        }
        for (const signature of this.index.language?.nodeSignatures(functionName, indexed.version) ?? []) {
            const input = signature.inputs.find((i) => i.name === name);
            if (input) return formatInputHover(functionName, input);
        }
        return undefined;
    }
}
