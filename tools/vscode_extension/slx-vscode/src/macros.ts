// Finds preprocessor macros: the names defined with #define, and where they are used.
import { blankComments } from './includes';
import type { LexedText } from './lexer';

/** Macros the compiler defines itself. */
export const PREDEFINED_MACROS = ['__MAIN__', '__INCLUDE__', '__FILE__', '__LINE__'];

/** Directives whose first argument is a macro name, whether or not it is defined. */
const NAME_DIRECTIVES = new Set(['define', 'undef', 'ifdef', 'ifndef', 'elifdef', 'elifndef']);
/** Directives whose arguments are code that can use macros. */
const CODE_DIRECTIVES = new Set(['define', 'if', 'elif']);

const DEFINE = /^[ \t]*#[ \t]*define[ \t]+([A-Za-z_]\w*)/gm;

/** Returns the names defined with #define in the text. */
export function findDefinedMacros(text: string): string[] {
    return [...blankComments(text).matchAll(DEFINE)].map((match) => match[1]);
}

export interface MacroUse {
    start: number;
    end: number;
    /** The name in its #define. */
    isDefinition: boolean;
    /** One of the macros the compiler defines itself. */
    isPredefined: boolean;
}

/** Finds the macros used in code and named in preprocessor directives. */
export function findMacroUses(lexed: LexedText, macros: ReadonlySet<string>): MacroUse[] {
    const uses: MacroUse[] = [];
    const add = (start: number, name: string, isDefinition: boolean) =>
        uses.push({ start, end: start + name.length, isDefinition, isPredefined: PREDEFINED_MACROS.includes(name) });

    lexed.tokens.forEach((token, index) => {
        if (token.kind === 'identifier') {
            // A name after "." is a field, method or port, not a macro.
            if (macros.has(token.text) && lexed.tokens[index - 1]?.text !== '.') add(token.start, token.text, false);
            return;
        }
        if (token.kind !== 'directive') return;
        const directive = /^#[ \t]*([A-Za-z]+)/.exec(token.text);
        if (!directive) return;
        const name = directive[1];
        if (!NAME_DIRECTIVES.has(name) && !CODE_DIRECTIVES.has(name)) return;
        // The rest of the line, without a trailing comment.
        const body = token.text.slice(directive[0].length).replace(/\/\/.*$|\/\*.*$/, '');
        let first = true;
        for (const word of body.matchAll(/"[^"]*"|[A-Za-z_]\w*/g)) {
            if (word[0].startsWith('"')) continue;
            const start = token.start + directive[0].length + word.index!;
            if (first && NAME_DIRECTIVES.has(name)) add(start, word[0], name === 'define');
            else if (CODE_DIRECTIVES.has(name) && macros.has(word[0])) add(start, word[0], false);
            first = false;
        }
    });
    return uses;
}
