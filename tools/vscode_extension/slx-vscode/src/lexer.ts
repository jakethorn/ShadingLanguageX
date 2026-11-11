// Splits SLX source into tokens, for finding declarations and calls.
//
// This is a light tokenizer for editor features, not the compiler's: it only
// needs to tell identifiers, literals and punctuation apart, and to skip
// comments, strings and preprocessor lines.

export type TokenKind = 'identifier' | 'number' | 'string' | 'punctuation' | 'directive';

export interface Token {
    kind: TokenKind;
    text: string;
    /** Offset of the first character. */
    start: number;
    /** Offset just after the last character. */
    end: number;
}

export interface LexedText {
    tokens: Token[];
    /** Start and end offsets of each comment. */
    comments: Array<[number, number]>;
}

// Longest first, so that "==" is not read as two "=".
const MULTI_CHARACTER_PUNCTUATION = [
    '[[', ']]', '@@', '==', '!=', '<=', '>=', '+=', '-=', '*=', '/=', '%=', '^=', '&=', '|=',
    '->', '=>', '::', '++', '--', '&&', '||',
];

const IDENTIFIER = /[A-Za-z_]\w*/y;
const NUMBER = /(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?[fF]?/y;

function matchAt(pattern: RegExp, text: string, offset: number): string | undefined {
    pattern.lastIndex = offset;
    return pattern.exec(text)?.[0];
}

function isLineStart(text: string, offset: number): boolean {
    for (let i = offset - 1; i >= 0; i--) {
        if (text[i] === '\n') return true;
        if (text[i] !== ' ' && text[i] !== '\t' && text[i] !== '\r') return false;
    }
    return true;
}

export function tokenize(text: string): LexedText {
    const tokens: Token[] = [];
    const comments: Array<[number, number]> = [];
    let i = 0;
    const push = (kind: TokenKind, end: number) => {
        tokens.push({ kind, text: text.slice(i, end), start: i, end });
        i = end;
    };

    while (i < text.length) {
        const c = text[i];
        const next = text[i + 1];
        if (c === ' ' || c === '\t' || c === '\r' || c === '\n') {
            i++;
        } else if (c === '/' && next === '/') {
            const end = text.indexOf('\n', i);
            const commentEnd = end === -1 ? text.length : end;
            comments.push([i, commentEnd]);
            i = commentEnd;
        } else if (c === '/' && next === '*') {
            const end = text.indexOf('*/', i + 2);
            const commentEnd = end === -1 ? text.length : end + 2;
            comments.push([i, commentEnd]);
            i = commentEnd;
        } else if (c === '#' && isLineStart(text, i)) {
            const end = text.indexOf('\n', i);
            push('directive', end === -1 ? text.length : end);
        } else if (c === '"') {
            const close = text.indexOf('"', i + 1);
            const newline = text.indexOf('\n', i + 1);
            const end = close === -1 || (newline !== -1 && newline < close) ? (newline === -1 ? text.length : newline) : close + 1;
            push('string', end);
        } else if (/[A-Za-z_]/.test(c)) {
            push('identifier', i + matchAt(IDENTIFIER, text, i)!.length);
        } else if (/\d/.test(c) || (c === '.' && next !== undefined && /\d/.test(next))) {
            push('number', i + matchAt(NUMBER, text, i)!.length);
        } else {
            const multi = MULTI_CHARACTER_PUNCTUATION.find((p) => text.startsWith(p, i));
            push('punctuation', i + (multi?.length ?? 1));
        }
    }
    return { tokens, comments };
}

/** True when the offset is inside a comment or a string literal, where code completion makes no sense. */
export function isInCommentOrString(lexed: LexedText, offset: number): boolean {
    if (lexed.comments.some(([start, end]) => start < offset && offset <= end)) return true;
    return lexed.tokens.some((token) => {
        if (token.kind !== 'string' || token.start >= offset) return false;
        const closed = token.text.length >= 2 && token.text.endsWith('"');
        return offset < token.end || (offset === token.end && !closed);
    });
}

/** Returns the index of the last token that starts before the offset, or -1. */
export function findTokenBefore(tokens: Token[], offset: number): number {
    let low = 0;
    let high = tokens.length - 1;
    let found = -1;
    while (low <= high) {
        const middle = (low + high) >> 1;
        if (tokens[middle].start < offset) {
            found = middle;
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }
    return found;
}
