// Parameter hints: the signatures of the function being called, with the current argument highlighted.
import * as vscode from 'vscode';
import { findCallContext } from './callContext';
import { isInCommentOrString } from './lexer';
import { functionSignatureHint, nodeSignatureHints, type SignatureHint } from './signatures';
import { findDeclarations, type SymbolIndex } from './symbolIndex';

function toSignatureInformation(hint: SignatureHint): vscode.SignatureInformation {
    const information = new vscode.SignatureInformation(hint.label, hint.documentation ? new vscode.MarkdownString(hint.documentation) : undefined);
    information.parameters = hint.parameters.map(
        (p) => new vscode.ParameterInformation(p.range, p.documentation ? new vscode.MarkdownString(p.documentation) : undefined),
    );
    return information;
}

export class SlxSignatureHelpProvider implements vscode.SignatureHelpProvider {
    constructor(private readonly index: SymbolIndex) {}

    provideSignatureHelp(
        document: vscode.TextDocument,
        position: vscode.Position,
        _token: vscode.CancellationToken,
        context: vscode.SignatureHelpContext,
    ): vscode.SignatureHelp | undefined {
        const indexed = this.index.index(document);
        const offset = document.offsetAt(position);
        const lexed = indexed.file.scanned.lexed;
        if (isInCommentOrString(lexed, offset)) return undefined;
        const call = findCallContext(lexed, offset);
        if (!call) return undefined;

        // Functions written in SLX hide the nodes and standard library functions with the same name.
        const declared = findDeclarations(indexed, (s) => (s.kind === 'function' || s.kind === 'constructor') && s.name === call.name);
        let hints = declared.map((d) => functionSignatureHint(d.symbol));
        const language = this.index.language;
        if (hints.length === 0 && language) {
            let signatures = language.nodeSignatures(call.name, indexed.version);
            if (call.templateArguments.length === 1) {
                const type = language.canonicalType(call.templateArguments[0]);
                const matching = signatures.filter((s) => s.outputs.length === 1 && s.outputs[0].type === type);
                if (matching.length > 0) signatures = matching;
            }
            hints = [
                ...nodeSignatureHints(call.name, signatures, language.specLink(call.name)),
                ...language.stdlibOverloads(call.name).map(functionSignatureHint),
            ];
        }
        if (hints.length === 0) return undefined;

        const fits = (hint: SignatureHint) =>
            call.namedArgument !== undefined ? hint.parameters.some((p) => p.name === call.namedArgument) : hint.parameters.length > call.argumentIndex;
        const previous = context.activeSignatureHelp?.activeSignature;
        const activeSignature =
            previous !== undefined && previous < hints.length && fits(hints[previous]) ? previous : Math.max(0, hints.findIndex(fits));

        const help = new vscode.SignatureHelp();
        help.signatures = hints.map(toSignatureInformation);
        help.activeSignature = activeSignature;
        const active = hints[activeSignature];
        help.activeParameter =
            call.namedArgument !== undefined ? active.parameters.findIndex((p) => p.name === call.namedArgument) : call.argumentIndex;
        return help;
    }
}
