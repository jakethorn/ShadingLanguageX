// Works out what the word at a position is, for hovers and Go to Definition.
import * as vscode from 'vscode';
import { findCallContext, type CallContext } from './callContext';
import { isInCommentOrString, type LexedText } from './lexer';

export interface WordContext {
    word: string;
    range: vscode.Range;
    /** Offsets of the word. */
    start: number;
    end: number;
    /** The word follows a ".", so it names a field, method, port or swizzle. */
    afterDot: boolean;
    /** Set when the word is the name in a `name = value` argument of this call. */
    argumentOf?: CallContext;
}

export function findWordContext(document: vscode.TextDocument, position: vscode.Position, lexed: LexedText): WordContext | undefined {
    const range = document.getWordRangeAtPosition(position, /[A-Za-z_]\w*/);
    if (!range) return undefined;
    const start = document.offsetAt(range.start);
    const end = document.offsetAt(range.end);
    if (isInCommentOrString(lexed, start + 1)) return undefined;

    const lineText = document.lineAt(range.start.line).text;
    if (/^\s*#/.test(lineText)) return undefined;
    const before = lineText.slice(0, range.start.character);
    // Attribute names, as in @doc or @in.doc, aren't declared anywhere.
    if (/@@?\s*$/.test(before) || /@\w+\.\s*$/.test(before)) return undefined;

    const after = lineText.slice(range.end.character);
    let argumentOf: CallContext | undefined;
    if (/^\s*=(?!=)/.test(after)) {
        const call = findCallContext(lexed, end);
        if (call?.atArgumentName) argumentOf = call;
    }
    return { word: document.getText(range), range, start, end, afterDot: /\.\s*$/.test(before), argumentOf };
}
