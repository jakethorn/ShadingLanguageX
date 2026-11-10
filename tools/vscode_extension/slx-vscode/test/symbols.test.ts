import assert from 'node:assert/strict';
import { test } from 'node:test';
import { isInCommentOrString, tokenize } from '../src/lexer';
import { findGlobalDeclarations, findVisibleVariables, scanSymbols } from '../src/symbols';

const summary = (text: string) =>
    scanSymbols(text).symbols.map((s) => `${s.kind} ${s.name}: ${s.signature}`);

test('tokenize skips comments and preprocessor lines and keeps operators whole', () => {
    const { tokens, comments } = tokenize('#include "a.mxsl"\nx == .5; // c\n/* d */ y');
    assert.deepEqual(
        tokens.map((t) => `${t.kind}:${t.text}`),
        ['directive:#include "a.mxsl"', 'identifier:x', 'punctuation:==', 'number:.5', 'punctuation:;', 'identifier:y'],
    );
    assert.equal(comments.length, 2);
});

test('isInCommentOrString is true inside comments and strings only', () => {
    const text = 'float a = 1.0; // note\nstring s = "abc";';
    const lexed = tokenize(text);
    assert.equal(isInCommentOrString(lexed, text.indexOf('note')), true);
    assert.equal(isInCommentOrString(lexed, text.indexOf('abc') + 1), true);
    assert.equal(isInCommentOrString(lexed, text.indexOf('"abc"')), false);
    assert.equal(isInCommentOrString(lexed, text.indexOf('= 1.0')), false);
});

test('scanSymbols finds functions with their modifiers, templates and parameters', () => {
    const text = '[[inline]] float add_one(float in)\n{\n    return in + 1.0;\n}\nT one_minus<float, vec2>(T v) { return 1.0 - v; }\n';
    assert.deepEqual(summary(text), [
        'function add_one: inline float add_one(float in)',
        'function one_minus: T one_minus<float, vec2>(T v)',
    ]);
    const [addOne] = scanSymbols(text).symbols;
    assert.deepEqual(addOne.parameters, [{ name: 'in', label: 'float in', doc: undefined }]);
    assert.equal(text.slice(addOne.nameStart, addOne.nameEnd), 'add_one');
});

test('scanSymbols reads @doc attributes, parameter docs and leading comments', () => {
    const text = '// Adds one.\n// Really.\nfloat a(float x) { return x; }\n\n@doc "Doubles."\nfloat b(@doc "the value" float x) { return x; }\n';
    const [a, b] = scanSymbols(text).symbols;
    assert.equal(a.doc, 'Adds one.\nReally.');
    assert.equal(b.doc, 'Doubles.');
    assert.equal(b.parameters![0].doc, 'the value');
});

test('scanSymbols finds variables, including multi-variable and struct declarations', () => {
    const text = 'global color3 tint;\nconst float a, b = 1.0, 2.0;\nfloat c, int d = 1.0, 2;\nglobal { vec3 pos, string space } light;\nx = 3;\nsurface.base_color = c;\n';
    assert.deepEqual(summary(text), [
        'variable tint: global color3 tint',
        'variable a: const float a',
        'variable b: const float b',
        'variable c: float c',
        'variable d: int d',
        'variable light: global { vec3 pos, string space } light',
    ]);
});

test('scanSymbols finds classes with their members, and type aliases', () => {
    const text = 'using Ray = {vec3 orig, vec3 dir};\nclass Point\n{\n    Point(float v) { this.x = v; }\n    float length() { return x; }\n    float x;\n}\n';
    const [alias, point] = scanSymbols(text).symbols;
    assert.equal(alias.signature, 'using Ray = {vec3 orig, vec3 dir}');
    assert.equal(point.kind, 'class');
    assert.deepEqual(
        point.members!.map((m) => `${m.kind} ${m.name}`),
        ['constructor Point', 'method length', 'field x'],
    );
});

test('scanSymbols ignores calls, assignments and keywords that look like declarations', () => {
    const text = 'standard_surface(base_color = c);\nreturn foo(x);\nfloat area = comptime get_area(10);\nprint a, b;\n';
    assert.deepEqual(summary(text), ['variable area: float area']);
});

test('findVisibleVariables follows block scopes and declaration order', () => {
    const text = 'float top = 1.0;\nfloat f(float p)\n{\n    float local = p;\n    for (int i from 0 to 3)\n    {\n        float inner = i;\n    }\n    return local;\n}\nfloat after = 2.0;\n';
    const scanned = scanSymbols(text);
    const namesAt = (marker: string) => findVisibleVariables(scanned, text.indexOf(marker)).map((s) => s.name).sort();
    assert.deepEqual(namesAt('float inner'), ['i', 'local', 'p', 'top']);
    assert.deepEqual(namesAt('return local'), ['local', 'p', 'top']);
    assert.deepEqual(namesAt('float after'), ['top']);
    assert.deepEqual(namesAt('= 2.0'), ['after', 'top']);
});

test('findVisibleVariables prefers the innermost declaration of a name', () => {
    const text = 'float a = 1.0;\nvoid f(float a)\n{\n    print a;\n}\n';
    const scanned = scanSymbols(text);
    const [a] = findVisibleVariables(scanned, text.indexOf('print a'));
    assert.equal(a.kind, 'parameter');
});

test('scanSymbols finds every function in the example shaders shape', () => {
    const text = 'vec3 get_waves(vec2 uv, float t)\n{\n    mutable vec3 n = vec3{0.0};\n    n = if (t > 1.0) { vec3{1.0} } else { n };\n    return n;\n}\nsurfacematerial(standard_surface(base_color = get_waves(texcoord<vec2>(), 1.0)));\n';
    const scanned = scanSymbols(text);
    assert.deepEqual(
        findGlobalDeclarations(scanned).map((s) => s.name),
        ['get_waves'],
    );
    assert.deepEqual(
        scanned.all.filter((s) => s.kind === 'variable').map((s) => s.name),
        ['n'],
    );
});
