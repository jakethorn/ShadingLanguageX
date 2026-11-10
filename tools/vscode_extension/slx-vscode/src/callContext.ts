// Finds the function call an offset is in, for parameter hints and argument name completion.
import { findTokenBefore, type LexedText, type Token } from './lexer';

export interface CallContext {
    /** Name of the function being called. */
    name: string;
    nameStart: number;
    /** Types in `name<types>(...)`. */
    templateArguments: string[];
    /** 0-based index of the argument the offset is in. */
    argumentIndex: number;
    /** Name of the argument, when it is passed by name and the offset is in its value. */
    namedArgument?: string;
    /** True when the offset is where an argument name goes: the start of an argument, or in the word starting it. */
    atArgumentName: boolean;
    /** Names of the arguments already passed by name. */
    usedNames: string[];
}

const NOT_CALLS = new Set(['if', 'for', 'switch', 'return', 'print', 'typeof']);

function isOpening(token: Token): boolean {
    return token.kind === 'punctuation' && (token.text === '(' || token.text === '[' || token.text === '{');
}

function isClosing(token: Token): boolean {
    return token.kind === 'punctuation' && (token.text === ')' || token.text === ']' || token.text === '}');
}

/** Splits the tokens of an argument list, at the top level, into arguments. */
function splitArguments(tokens: Token[]): Token[][] {
    const args: Token[][] = [[]];
    let depth = 0;
    for (const token of tokens) {
        if (isOpening(token)) depth++;
        else if (isClosing(token)) depth--;
        if (depth === 0 && token.text === ',' && token.kind === 'punctuation') args.push([]);
        else args[args.length - 1].push(token);
    }
    return args;
}

/** Returns the name of an argument passed as `name = value`, if it is one. */
function findArgumentName(tokens: Token[]): string | undefined {
    const equals = tokens.findIndex((t) => t.kind === 'punctuation' && t.text === '=');
    if (equals < 1) return undefined;
    const name = tokens[equals - 1];
    return name.kind === 'identifier' ? name.text : undefined;
}

/** True when the identifier token at index is the name in a call, as in `name(` or `name<types>(`. */
export function isCallName(tokens: Token[], index: number): boolean {
    if (tokens[index]?.kind !== 'identifier' || NOT_CALLS.has(tokens[index].text)) return false;
    let next = index + 1;
    if (tokens[next]?.text === '<') {
        next++;
        while (tokens[next] && (tokens[next].kind === 'identifier' || tokens[next].text === ',')) next++;
        if (tokens[next]?.text !== '>') return false;
        next++;
    }
    return tokens[next]?.kind === 'punctuation' && tokens[next].text === '(';
}

export function findCallContext(lexed: LexedText, offset: number): CallContext | undefined {
    const tokens = lexed.tokens;
    const last = findTokenBefore(tokens, offset);

    // Walk back to the unclosed '(' the offset is in.
    let depth = 0;
    let open = -1;
    for (let i = last; i >= 0; i--) {
        const token = tokens[i];
        if (token.kind === 'directive' || (token.kind === 'punctuation' && token.text === ';')) return undefined;
        if (isClosing(token)) {
            depth++;
        } else if (isOpening(token)) {
            if (depth > 0) {
                depth--;
                continue;
            }
            if (token.text !== '(') return undefined;
            open = i;
            break;
        }
    }
    if (open < 1) return undefined;

    // The name, and any template arguments: name<a, b>(
    let nameIndex = open - 1;
    const templateArguments: string[] = [];
    if (tokens[nameIndex].text === '>') {
        let i = nameIndex - 1;
        while (i >= 0 && (tokens[i].kind === 'identifier' || tokens[i].text === ',')) {
            if (tokens[i].kind === 'identifier') templateArguments.unshift(tokens[i].text);
            i--;
        }
        if (i < 0 || tokens[i].text !== '<') return undefined;
        nameIndex = i - 1;
    }
    const nameToken = tokens[nameIndex];
    if (!nameToken || nameToken.kind !== 'identifier' || NOT_CALLS.has(nameToken.text)) return undefined;

    const args = splitArguments(tokens.slice(open + 1, last + 1));
    const current = args[args.length - 1];
    const namedArgument = findArgumentName(current);
    const typing = current.length === 1 && current[0].kind === 'identifier' && current[0].end >= offset;
    return {
        name: nameToken.text,
        nameStart: nameToken.start,
        templateArguments,
        argumentIndex: args.length - 1,
        namedArgument,
        atArgumentName: namedArgument === undefined && (current.length === 0 || typing),
        usedNames: args.slice(0, -1).map(findArgumentName).filter((n): n is string => n !== undefined),
    };
}
