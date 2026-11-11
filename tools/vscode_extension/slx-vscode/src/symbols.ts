// Finds the declarations in SLX source: functions, classes, type aliases,
// variables and parameters, with where each one can be used.
//
// It matches declarations by their shape rather than compiling the source, so
// it works on files with errors in them, but it knows nothing about types.
import { tokenize, type LexedText, type Token } from './lexer';

export type SymbolKind = 'function' | 'method' | 'constructor' | 'class' | 'alias' | 'variable' | 'field' | 'parameter';

export interface SlxParameter {
    name: string;
    /** The parameter as written, e.g. "out float s" or "float a = 1.0". */
    label: string;
    doc?: string;
}

export interface SlxSymbol {
    kind: SymbolKind;
    name: string;
    nameStart: number;
    nameEnd: number;
    /** The whole declaration, from its attributes and modifiers to its end. */
    start: number;
    end: number;
    /** The declaration as shown in hovers, e.g. "inline float add_one(float in)". */
    signature: string;
    /** Declared type, or the return type of a function. */
    type?: string;
    modifiers: string[];
    /** From a @doc attribute, or the comment just above the declaration. */
    doc?: string;
    parameters?: SlxParameter[];
    /** Fields, methods and constructors of a class. */
    members?: SlxSymbol[];
    /** Name of the class a member belongs to. */
    container?: string;
    /** Where the name can be used. For variables and parameters, from the declaration to the end of its block. */
    scopeStart: number;
    scopeEnd: number;
}

export interface ScannedSource {
    lexed: LexedText;
    /** Declarations at the top level of the file. Classes hold their members. */
    symbols: SlxSymbol[];
    /** Every declaration, including members, local variables and parameters. */
    all: SlxSymbol[];
}

const MODIFIERS = new Set([
    'const', 'mutable', 'consteval', 'comptime', 'global', 'geomprop', 'nodegraph', 'nodedef', 'inline',
    'uniform', 'varying', 'ref', 'out',
]);

// Keywords that can't be the type at the start of a declaration.
const STATEMENT_KEYWORDS = new Set([
    'if', 'else', 'for', 'from', 'to', 'return', 'break', 'switch', 'case', 'default', 'using', 'class',
    'namespace', 'print', 'typeof', 'this', 'null', 'true', 'false',
]);

function collapseWhitespace(text: string): string {
    return text.replace(/\s+/g, ' ').trim();
}

function unquote(text: string): string {
    return text.startsWith('"') ? text.slice(1, text.endsWith('"') && text.length > 1 ? -1 : undefined) : text;
}

/** Maps each opening bracket token to its closing token, and back. */
function matchBrackets(tokens: Token[]): Map<number, number> {
    const matches = new Map<number, number>();
    const stack: number[] = [];
    const pairs: Record<string, string> = { ')': '(', ']': '[', '}': '{', ']]': '[[' };
    tokens.forEach((token, index) => {
        if (token.kind !== 'punctuation') return;
        if (token.text === '(' || token.text === '[' || token.text === '{' || token.text === '[[') {
            stack.push(index);
        } else if (pairs[token.text]) {
            // Pop to the matching opener, dropping unmatched ones in between.
            for (let i = stack.length - 1; i >= 0; i--) {
                if (tokens[stack[i]].text === pairs[token.text]) {
                    matches.set(stack[i], index);
                    matches.set(index, stack[i]);
                    stack.length = i;
                    break;
                }
            }
        }
    });
    return matches;
}

interface Attributes {
    doc?: string;
}

class DeclarationScanner {
    readonly all: SlxSymbol[] = [];
    private readonly tokens: Token[];
    private readonly matches: Map<number, number>;

    constructor(
        private readonly text: string,
        private readonly lexed: LexedText,
    ) {
        this.tokens = lexed.tokens;
        this.matches = matchBrackets(this.tokens);
    }

    private token(index: number): Token | undefined {
        return this.tokens[index];
    }

    private is(index: number, text: string): boolean {
        const token = this.tokens[index];
        return token !== undefined && token.kind !== 'string' && token.text === text;
    }

    private isIdentifier(index: number): boolean {
        return this.tokens[index]?.kind === 'identifier';
    }

    /** Index of the token closing the bracket at index, or the end of the range if it is unclosed. */
    private closing(index: number, end: number): number {
        return Math.min(this.matches.get(index) ?? end, end);
    }

    private textBetween(startToken: number, endToken: number): string {
        if (endToken < startToken) return '';
        return collapseWhitespace(this.text.slice(this.tokens[startToken].start, this.tokens[endToken].end));
    }

    /** End offset of the range of tokens, for scopes. */
    private offsetAfter(endToken: number): number {
        return this.tokens[endToken]?.end ?? this.text.length;
    }

    /** The comment directly above an offset, with no blank line between. */
    private findLeadingComment(offset: number): string | undefined {
        const comments = this.lexed.comments;
        // The last comment that ends before the offset.
        let low = 0;
        let high = comments.length - 1;
        let last = -1;
        while (low <= high) {
            const middle = (low + high) >> 1;
            if (comments[middle][1] <= offset) {
                last = middle;
                low = middle + 1;
            } else {
                high = middle - 1;
            }
        }
        const lines: string[] = [];
        let limit = offset;
        for (let c = last; c >= 0; c--) {
            const [start, end] = comments[c];
            const gap = this.text.slice(end, limit);
            if (gap.trim() !== '' || (gap.match(/\n/g)?.length ?? 0) > 1) break;
            const comment = this.text.slice(start, end);
            const body = comment.startsWith('//')
                ? comment.slice(2)
                : comment.slice(2, comment.endsWith('*/') ? -2 : undefined).replace(/^\s*\*+ ?/gm, '');
            lines.unshift(body.trim());
            limit = start;
            if (!comment.startsWith('//')) break;
        }
        const doc = lines.join('\n').trim();
        return doc === '' ? undefined : doc;
    }

    /** Reads @attributes from index, returning the index after them. */
    private readAttributes(index: number, end: number, attributes: Attributes): number {
        while (index < end && (this.is(index, '@') || this.is(index, '@@'))) {
            index++;
            let name = this.token(index)?.text;
            index++;
            let port: string | undefined;
            if (this.is(index, '.')) {
                port = name;
                name = this.token(index + 1)?.text;
                index += 2;
            }
            if (this.is(index, '-')) index++;
            const value = this.token(index);
            if (value && value.kind !== 'punctuation') {
                if (name === 'doc' && port === undefined) attributes.doc = unquote(value.text);
                index++;
            }
        }
        return index;
    }

    /** Reads modifiers, including [[modifier]] groups, returning the index after them. */
    private readModifiers(index: number, end: number, modifiers: string[]): number {
        while (index < end) {
            if (this.is(index, '[[')) {
                const close = this.closing(index, end);
                for (let i = index + 1; i < close; i++) if (this.isIdentifier(i)) modifiers.push(this.tokens[i].text);
                index = close + 1;
            } else if (this.isIdentifier(index) && MODIFIERS.has(this.tokens[index].text)) {
                modifiers.push(this.tokens[index].text);
                index++;
            } else {
                break;
            }
        }
        return index;
    }

    /** Reads a type (a name or an unnamed struct), returning the index after it, or undefined if there is none. */
    private readType(index: number, end: number): number | undefined {
        if (this.is(index, '{')) {
            const close = this.closing(index, end);
            return close < end ? close + 1 : undefined;
        }
        if (this.isIdentifier(index) && !STATEMENT_KEYWORDS.has(this.tokens[index].text)) return index + 1;
        return undefined;
    }

    /** Index of the ';' ending a statement, skipping brackets, or end. */
    private findSemicolon(index: number, end: number): number {
        while (index < end && !this.is(index, ';')) {
            index = this.matches.has(index) && this.matches.get(index)! > index ? this.closing(index, end) + 1 : index + 1;
        }
        return index;
    }

    private readParameters(open: number, close: number): SlxParameter[] {
        const parameters: SlxParameter[] = [];
        let index = open + 1;
        while (index < close) {
            let paramEnd = index;
            while (paramEnd < close && !this.is(paramEnd, ',')) {
                paramEnd = this.matches.has(paramEnd) && this.matches.get(paramEnd)! > paramEnd ? this.closing(paramEnd, close) + 1 : paramEnd + 1;
            }
            const attributes: Attributes = {};
            const labelStart = this.readAttributes(index, paramEnd, attributes);
            const afterModifiers = this.readModifiers(labelStart, paramEnd, []);
            const afterType = this.readType(afterModifiers, paramEnd);
            if (afterType !== undefined && this.isIdentifier(afterType)) {
                parameters.push({
                    name: this.tokens[afterType].text,
                    label: this.textBetween(labelStart, paramEnd - 1),
                    doc: attributes.doc,
                });
            }
            index = paramEnd + 1;
        }
        return parameters;
    }

    private addParameters(open: number, close: number, scopeStart: number, scopeEnd: number): void {
        let index = open + 1;
        for (const parameter of this.readParameters(open, close)) {
            while (index < close && !(this.isIdentifier(index) && this.tokens[index].text === parameter.name && this.isParameterName(index))) index++;
            const token = this.tokens[index];
            if (!token) break;
            this.all.push({
                kind: 'parameter',
                name: parameter.name,
                nameStart: token.start,
                nameEnd: token.end,
                start: token.start,
                end: token.end,
                signature: parameter.label,
                type: parameter.label.replace(/\s*=.*$/, '').split(' ').slice(0, -1).join(' '),
                modifiers: [],
                doc: parameter.doc,
                scopeStart,
                scopeEnd,
            });
            index++;
        }
    }

    /** True when the identifier at index is followed by ',', ')' or '=' as a parameter name is. */
    private isParameterName(index: number): boolean {
        return this.is(index + 1, ',') || this.is(index + 1, ')') || this.is(index + 1, '=');
    }

    /** Reads the declarations in tokens [from, to), whose names can be used until scopeEnd. */
    scanBlock(from: number, to: number, scopeEnd: number, container?: string): SlxSymbol[] {
        const declarations: SlxSymbol[] = [];
        let index = from;
        while (index < to) {
            if (this.is(index, ';') || this.token(index)?.kind === 'directive') {
                index++;
                continue;
            }
            const start = index;
            const attributes: Attributes = {};
            index = this.readAttributes(index, to, attributes);
            const modifiers: string[] = [];
            index = this.readModifiers(index, to, modifiers);
            const doc = attributes.doc ?? this.findLeadingComment(this.tokens[start].start);
            const declarationStart = this.tokens[start].start;

            const scanned =
                this.scanClass(index, to, declarationStart, doc, declarations) ??
                this.scanAlias(index, to, declarationStart, doc, declarations) ??
                this.scanForLoop(index, to) ??
                this.scanConstructor(index, to, declarationStart, modifiers, doc, container, declarations) ??
                this.scanFunctionOrVariables(index, to, declarationStart, modifiers, doc, scopeEnd, container, declarations);
            index = scanned ?? this.skipStatement(index, to);
            if (index <= start) index = start + 1;
        }
        return declarations;
    }

    private scanClass(index: number, to: number, start: number, doc: string | undefined, out: SlxSymbol[]): number | undefined {
        if (!this.is(index, 'class') || !this.isIdentifier(index + 1) || !this.is(index + 2, '{')) return undefined;
        const name = this.tokens[index + 1];
        const close = this.closing(index + 2, to);
        const members = this.scanBlock(index + 3, close, this.offsetAfter(close), name.text);
        const symbol: SlxSymbol = {
            kind: 'class',
            name: name.text,
            nameStart: name.start,
            nameEnd: name.end,
            start,
            end: this.offsetAfter(close),
            signature: `class ${name.text}`,
            modifiers: [],
            doc,
            members,
            scopeStart: 0,
            scopeEnd: this.text.length,
        };
        out.push(symbol);
        this.all.push(symbol);
        return close + 1;
    }

    private scanAlias(index: number, to: number, start: number, doc: string | undefined, out: SlxSymbol[]): number | undefined {
        if (!this.is(index, 'using') || !this.isIdentifier(index + 1) || !this.is(index + 2, '=')) return undefined;
        const name = this.tokens[index + 1];
        const semicolon = this.findSemicolon(index + 3, to);
        const symbol: SlxSymbol = {
            kind: 'alias',
            name: name.text,
            nameStart: name.start,
            nameEnd: name.end,
            start,
            end: this.offsetAfter(Math.min(semicolon, to - 1)),
            signature: this.textBetween(index, semicolon - 1),
            type: this.textBetween(index + 3, semicolon - 1),
            modifiers: [],
            doc,
            scopeStart: 0,
            scopeEnd: this.text.length,
        };
        out.push(symbol);
        this.all.push(symbol);
        return semicolon + 1;
    }

    /** `for (type name from ...) { ... }`: the loop variable can be used in the body. */
    private scanForLoop(index: number, to: number): number | undefined {
        if (!this.is(index, 'for') || !this.is(index + 1, '(')) return undefined;
        const close = this.closing(index + 1, to);
        if (!this.is(close + 1, '{')) return undefined;
        const bodyClose = this.closing(close + 1, to);
        const afterModifiers = this.readModifiers(index + 2, close, []);
        const afterType = this.readType(afterModifiers, close);
        if (afterType !== undefined && this.isIdentifier(afterType) && this.is(afterType + 1, 'from')) {
            const name = this.tokens[afterType];
            this.all.push({
                kind: 'variable',
                name: name.text,
                nameStart: name.start,
                nameEnd: name.end,
                start: this.tokens[afterModifiers].start,
                end: name.end,
                signature: this.textBetween(afterModifiers, afterType),
                type: this.textBetween(afterModifiers, afterType - 1),
                modifiers: [],
                scopeStart: name.end,
                scopeEnd: this.offsetAfter(bodyClose),
            });
        }
        this.scanBlock(close + 2, bodyClose, this.offsetAfter(bodyClose));
        return bodyClose + 1;
    }

    private scanConstructor(
        index: number,
        to: number,
        start: number,
        modifiers: string[],
        doc: string | undefined,
        container: string | undefined,
        out: SlxSymbol[],
    ): number | undefined {
        if (!container || !this.isIdentifier(index) || this.tokens[index].text !== container || !this.is(index + 1, '(')) return undefined;
        const close = this.closing(index + 1, to);
        if (!this.is(close + 1, '{')) return undefined;
        const bodyClose = this.closing(close + 1, to);
        const name = this.tokens[index];
        const symbol: SlxSymbol = {
            kind: 'constructor',
            name: name.text,
            nameStart: name.start,
            nameEnd: name.end,
            start,
            end: this.offsetAfter(bodyClose),
            signature: this.textBetween(index, close),
            modifiers,
            doc,
            parameters: this.readParameters(index + 1, close),
            container,
            scopeStart: 0,
            scopeEnd: this.text.length,
        };
        out.push(symbol);
        this.all.push(symbol);
        this.addParameters(index + 1, close, this.tokens[close + 1].start, this.offsetAfter(bodyClose));
        this.scanBlock(close + 2, bodyClose, this.offsetAfter(bodyClose));
        return bodyClose + 1;
    }

    /** Reads `<a, b>` template arguments at index, returning the index after them. */
    private readTemplateArguments(index: number, end: number): number | undefined {
        if (!this.is(index, '<')) return undefined;
        let i = index + 1;
        while (i < end && (this.isIdentifier(i) || this.is(i, ','))) i++;
        return this.is(i, '>') ? i + 1 : undefined;
    }

    private scanFunctionOrVariables(
        index: number,
        to: number,
        start: number,
        modifiers: string[],
        doc: string | undefined,
        scopeEnd: number,
        container: string | undefined,
        out: SlxSymbol[],
    ): number | undefined {
        const typeStart = index;
        const afterType = this.readType(index, to);
        if (afterType === undefined || !this.isIdentifier(afterType)) return undefined;
        const type = this.textBetween(typeStart, afterType - 1);
        const name = this.tokens[afterType];
        const modifierText = modifiers.length > 0 ? `${modifiers.join(' ')} ` : '';

        // A function definition: type name [<types>] (parameters) { body }
        const afterTemplate = this.readTemplateArguments(afterType + 1, to) ?? afterType + 1;
        if (this.is(afterTemplate, '(')) {
            const close = this.closing(afterTemplate, to);
            if (!this.is(close + 1, '{')) return undefined;
            const bodyClose = this.closing(close + 1, to);
            const symbol: SlxSymbol = {
                kind: container ? 'method' : 'function',
                name: name.text,
                nameStart: name.start,
                nameEnd: name.end,
                start,
                end: this.offsetAfter(bodyClose),
                signature: modifierText + this.textBetween(typeStart, close),
                type,
                modifiers,
                doc,
                parameters: this.readParameters(afterTemplate, close),
                container,
                scopeStart: 0,
                scopeEnd: this.text.length,
            };
            out.push(symbol);
            this.all.push(symbol);
            this.addParameters(afterTemplate, close, this.tokens[close + 1].start, this.offsetAfter(bodyClose));
            this.scanBlock(close + 2, bodyClose, this.offsetAfter(bodyClose));
            return bodyClose + 1;
        }

        // Variables: type name [, [type] name]* [= values];
        if (!this.is(afterType + 1, '=') && !this.is(afterType + 1, ';') && !this.is(afterType + 1, ',')) return undefined;
        const semicolon = this.findSemicolon(afterType + 1, to);
        const names: Array<{ token: Token; type: string }> = [{ token: name, type }];
        let i = afterType + 1;
        while (this.is(i, ',')) {
            const itemStart = this.readModifiers(i + 1, semicolon, []);
            if (this.isIdentifier(itemStart) && this.isIdentifier(itemStart + 1)) {
                names.push({ token: this.tokens[itemStart + 1], type: this.tokens[itemStart].text });
                i = itemStart + 2;
            } else if (this.isIdentifier(itemStart)) {
                names.push({ token: this.tokens[itemStart], type: names[names.length - 1].type });
                i = itemStart + 1;
            } else {
                break;
            }
        }
        for (const { token, type: itemType } of names) {
            const symbol: SlxSymbol = {
                kind: container ? 'field' : 'variable',
                name: token.text,
                nameStart: token.start,
                nameEnd: token.end,
                start,
                end: this.offsetAfter(Math.min(semicolon, to - 1)),
                signature: `${modifierText}${itemType} ${token.text}`,
                type: itemType,
                modifiers,
                doc,
                container,
                scopeStart: container ? 0 : token.end,
                scopeEnd,
            };
            out.push(symbol);
            this.all.push(symbol);
        }
        return semicolon + 1;
    }

    /**
     * Skips a statement that declares nothing, scanning any blocks in it for
     * declarations. A statement ends at a ';', or after a block that is not
     * followed by something continuing the expression, as with an if statement.
     */
    private skipStatement(index: number, to: number): number {
        while (index < to) {
            if (this.is(index, ';')) return index + 1;
            if (this.is(index, '{')) {
                const close = this.closing(index, to);
                this.scanBlock(index + 1, close, this.offsetAfter(close));
                index = close + 1;
                const next = this.token(index);
                const continues =
                    next !== undefined &&
                    (next.text === 'else' || (next.kind === 'punctuation' && !['@', '@@', '[[', '{'].includes(next.text)));
                if (!continues) return index;
                continue;
            }
            index = this.matches.has(index) && this.matches.get(index)! > index ? this.closing(index, to) + 1 : index + 1;
        }
        return index;
    }
}

export function scanSymbols(text: string): ScannedSource {
    const lexed = tokenize(text);
    const scanner = new DeclarationScanner(text, lexed);
    const symbols = scanner.scanBlock(0, lexed.tokens.length, text.length);
    return { lexed, symbols, all: scanner.all };
}

/** Variables and parameters that can be used at the offset, innermost first, one per name. */
export function findVisibleVariables(scanned: ScannedSource, offset: number): SlxSymbol[] {
    const visible = scanned.all
        .filter((s) => (s.kind === 'variable' || s.kind === 'parameter') && s.scopeStart <= offset && offset <= s.scopeEnd)
        .sort((a, b) => b.scopeStart - a.scopeStart);
    const seen = new Set<string>();
    return visible.filter((s) => !seen.has(s.name) && seen.add(s.name));
}

/** Functions, classes and type aliases that can be called or used as types anywhere in the file. */
export function findGlobalDeclarations(scanned: ScannedSource): SlxSymbol[] {
    return scanned.all.filter((s) => s.kind === 'function' || s.kind === 'class' || s.kind === 'alias');
}
