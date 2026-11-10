// Tests the pure parts of completion, hover and parameter hints. The tests that
// use the MaterialX data need `gen_extension.py --generate` and are skipped
// without it.
import assert from 'node:assert/strict';
import * as path from 'node:path';
import { test } from 'node:test';
import { findCallContext, isCallName } from '../src/callContext';
import { SlxLanguage, findVersionDirective, type NodeSignature } from '../src/languageData';
import { tokenize } from '../src/lexer';
import { describeInput, formatDefaultValue, formatNodeHover, functionSignatureHint, nodeSignatureHints } from '../src/signatures';
import { scanSymbols } from '../src/symbols';

/** Returns the call context at the "|" in the text. */
function callAt(textWithCursor: string) {
    const offset = textWithCursor.indexOf('|');
    const text = textWithCursor.replace('|', '');
    return findCallContext(tokenize(text), offset);
}

test('findCallContext finds the call, the argument and whether a name goes there', () => {
    assert.deepEqual(callAt('standard_surface(|'), {
        name: 'standard_surface',
        nameStart: 0,
        templateArguments: [],
        argumentIndex: 0,
        namedArgument: undefined,
        atArgumentName: true,
        usedNames: [],
    });
    const second = callAt('mix(a, vec3{1, 2}, b|');
    assert.equal(second?.argumentIndex, 2);
    assert.equal(second?.atArgumentName, true);
    assert.equal(callAt('mix(a + |')?.atArgumentName, false);
});

test('findCallContext reads named arguments and template arguments', () => {
    const named = callAt('standard_surface(base = 1.0, base_color = |');
    assert.equal(named?.namedArgument, 'base_color');
    assert.deepEqual(named?.usedNames, ['base']);
    assert.equal(named?.atArgumentName, false);
    assert.deepEqual(callAt('image<color3>(|')?.templateArguments, ['color3']);
});

test('findCallContext looks through nested calls and stops at statements', () => {
    assert.equal(callAt('outer(inner(1), |')?.name, 'outer');
    assert.equal(callAt('outer(inner(|')?.name, 'inner');
    assert.equal(callAt('foo(1);\nfloat x = |'), undefined);
    assert.equal(callAt('if (|'), undefined);
    assert.equal(callAt('foo(vec3{1, |'), undefined);
});

test('isCallName finds names that are called, with or without template arguments', () => {
    const { tokens } = tokenize('color3 c = image<color3>(file) * mix(a, b, t); float normal = 1.0; if (x) { }');
    const called = tokens.filter((_, i) => isCallName(tokens, i)).map((t) => t.text);
    assert.deepEqual(called, ['image', 'mix']);
});

test('findVersionDirective ignores commented-out directives', () => {
    assert.equal(findVersionDirective('// #version 1.38.10\n#version 1.39.4\n'), '1.39.4');
    assert.equal(findVersionDirective('float a = 1.0;\n'), undefined);
});

const signature = (outputType: string, inputs: NodeSignature['inputs']): NodeSignature => ({
    nodedef: `ND_${outputType}`,
    inputs,
    outputs: [{ name: 'out', type: outputType }],
    versions: ['1.39.5'],
});

test('formatDefaultValue writes defaults as SLX', () => {
    assert.equal(formatDefaultValue({ name: 'c', type: 'color3', value: '0.8, 0.8, 0.8' }), 'color3{0.8, 0.8, 0.8}');
    assert.equal(formatDefaultValue({ name: 'f', type: 'filename', value: 'a.png' }), '"a.png"');
    assert.equal(formatDefaultValue({ name: 'x', type: 'float', value: '1.0' }), '1.0');
    assert.equal(formatDefaultValue({ name: 't', type: 'vector2', defaultgeomprop: 'UV0' }), undefined);
});

test('describeInput gives the doc, default, choices and uniform', () => {
    assert.equal(
        describeInput({ name: 'm', type: 'string', value: 'periodic', enum: 'clamp,periodic', uiname: 'Address Mode', uniform: true }),
        'Address Mode. Defaults to `"periodic"`. One of: `clamp`, `periodic`. Uniform.',
    );
});

test('nodeSignatureHints adds the output type only where the inputs are the same', () => {
    const file = { name: 'file', type: 'filename', uniform: true };
    const hints = nodeSignatureHints('image', [signature('color3', [file]), signature('float', [file])]);
    assert.equal(hints[0].label, 'color3 image<color3>(uniform filename file)');
    const added = nodeSignatureHints('add', [
        signature('float', [{ name: 'in1', type: 'float' }, { name: 'in2', type: 'float' }]),
        signature('color3', [{ name: 'in1', type: 'color3' }, { name: 'in2', type: 'color3' }]),
    ]);
    assert.equal(added[1].label, 'color3 add(color3 in1, color3 in2)');
    const [start, end] = added[1].parameters[1].range;
    assert.equal(added[1].label.slice(start, end), 'color3 in2');
});

test('functionSignatureHint marks each parameter in the label', () => {
    const [symbol] = scanSymbols('inline float mix3(float a, out float b = 1.0) { return a; }').symbols;
    const hint = functionSignatureHint(symbol);
    assert.equal(hint.label, 'inline float mix3(float a, out float b = 1.0)');
    assert.deepEqual(
        hint.parameters.map((p) => hint.label.slice(...p.range)),
        ['float a', 'out float b = 1.0'],
    );
});

const language = SlxLanguage.load(path.resolve(__dirname, '../../data'));
const skip = language ? false : 'data/ has not been generated';

test('the language data has the nodes of each MaterialX version', { skip }, () => {
    const image = language!.nodeSignatures('image', '1.39.5');
    assert.ok(image.length > 1);
    assert.ok(image[0].inputs.some((i) => i.name === 'texcoord' && i.defaultgeomprop === 'UV0'));
    assert.ok(language!.nodeNames('1.39.5').includes('hextiledimage'));
    assert.ok(!language!.nodeNames('1.38.10').includes('hextiledimage'));
    assert.ok(language!.stdlibOverloads('min').length > 0);
    assert.equal(language!.canonicalType('vec3'), 'vector3');
});

test('resolveVersion uses the closest version there is data for', { skip }, () => {
    assert.equal(language!.resolveVersion('1.39.4'), '1.39.4');
    assert.equal(language!.resolveVersion('1.39.9'), '1.39.5');
    assert.equal(language!.resolveVersion('1.0.0'), '1.38.10');
});

test('formatNodeHover lists one input per line for a node with one signature', { skip }, () => {
    const hover = formatNodeHover('standard_surface', language!.nodeSignatures('standard_surface', '1.39.5'), '1.39.5');
    assert.match(hover, /^```slx\nsurfaceshader standard_surface\(\n {4}float base = 1\.0,\n {4}color3 base_color = color3\{0\.8, 0\.8, 0\.8\},/);
    assert.match(hover, /\n```\nMaterialX 1\.39\.5 node · pbr$/);
});

test('formatNodeHover links to the specification', () => {
    const hover = formatNodeHover('add', [signature('float', [{ name: 'in1', type: 'float' }])], '1.39.5', 'https://example.com/spec.md#node-add');
    assert.match(hover, /MaterialX 1\.39\.5 node · \[MaterialX specification\]\(https:\/\/example\.com\/spec\.md#node-add\)$/);
});

test('the language data links nodes to their section of the specification', { skip }, () => {
    const spec = 'https://github.com/AcademySoftwareFoundation/MaterialX/blob/main/documents/Specification/';
    assert.equal(language!.specLink('image'), `${spec}MaterialX.StandardNodes.md#node-image`);
    assert.match(language!.specLink('standard_surface') ?? '', /^https:\/\/github\.com\/.*MaterialX\.PBRSpec\.md#/);
    assert.equal(language!.specLink('no_such_node'), undefined);
});
