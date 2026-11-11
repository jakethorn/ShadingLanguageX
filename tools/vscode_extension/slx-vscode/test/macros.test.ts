import assert from 'node:assert/strict';
import { test } from 'node:test';
import { tokenize } from '../src/lexer';
import { PREDEFINED_MACROS, findDefinedMacros, findMacroUses } from '../src/macros';

/** Returns each use as "name" with flags, from the text. */
function describeUses(text: string, macros: string[]) {
    return findMacroUses(tokenize(text), new Set([...PREDEFINED_MACROS, ...macros])).map(
        (use) => `${text.slice(use.start, use.end)}${use.isDefinition ? ' (definition)' : ''}${use.isPredefined ? ' (predefined)' : ''}`,
    );
}

test('findDefinedMacros finds #define names, but not commented-out ones', () => {
    const text = '#define SCALE 2.0\n  # define USE_RAIN\n// #define OLD 1\n/*\n#define GONE\n*/\nfloat x = 1.0;\n';
    assert.deepEqual(findDefinedMacros(text), ['SCALE', 'USE_RAIN']);
});

test('findMacroUses finds macros used in code, but not fields with the same name', () => {
    const text = '#define SCALE 2.0\nfloat t = 0.1 * SCALE;\nfloat u = s.SCALE;\nfloat v = SCALED;\n';
    assert.deepEqual(describeUses(text, ['SCALE']), ['SCALE (definition)', 'SCALE']);
});

test('findMacroUses finds names in #ifdef-like lines whether or not they are defined', () => {
    const text = '#ifdef USE_RAIN\n#ifndef __MAIN__\n#undef SCALE\n#elifdef OTHER\n#endif\n';
    assert.deepEqual(describeUses(text, ['SCALE']), ['USE_RAIN', '__MAIN__ (predefined)', 'SCALE', 'OTHER']);
});

test('findMacroUses finds only known macros in #if lines and #define values', () => {
    const text = '#if SCALE > LIMIT\n#define DOUBLE SCALE * 2\n#define dot dotproduct\n#endif\n';
    assert.deepEqual(describeUses(text, ['SCALE', 'DOUBLE', 'dot']), ['SCALE', 'DOUBLE (definition)', 'SCALE', 'dot (definition)']);
});

test('findMacroUses ignores strings and comments in directives, and #include paths', () => {
    const text = '#include "SCALE.mxsl"\n#define NAME "SCALE" // SCALE\n#ifdef __MAIN__ /* SCALE */\n';
    assert.deepEqual(describeUses(text, ['SCALE', 'NAME']), ['NAME (definition)', '__MAIN__ (predefined)']);
});

test('findMacroUses gives the offsets of each name', () => {
    const text = 'x = 1;\n  #  define   SCALE 2.0\n';
    const [use] = findMacroUses(tokenize(text), new Set(['SCALE']));
    assert.equal(text.slice(use.start, use.end), 'SCALE');
});
