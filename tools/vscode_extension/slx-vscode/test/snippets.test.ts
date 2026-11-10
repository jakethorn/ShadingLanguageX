// Expands each snippet with its default values and checks that it compiles.
// Needs the JsMxslc files in media/lib; skipped when they are missing.
import assert from 'node:assert/strict';
import * as fs from 'node:fs';
import * as path from 'node:path';
import { after, test } from 'node:test';
import { SlxEngine } from '../src/engine';
import type { CompileSettings } from '../src/engineProtocol';

const extensionDir = path.resolve(__dirname, '../..');
const engine = new SlxEngine(path.join(extensionDir, 'dist/engineWorker.mjs'), path.join(extensionDir, 'media/lib'));
const skip = engine.isBundled ? false : 'the JsMxslc files are not in media/lib';
after(() => engine.dispose());

const settings: CompileSettings = {
    materialxVersion: '1.39.5',
    reduceGraph: true,
    defaultFunctionBehaviour: 'nodedef',
    errorOnMissingGlobals: false,
};

const snippets: Record<string, { prefix: string; body: string[] }> = JSON.parse(
    fs.readFileSync(path.join(extensionDir, 'snippets/slx.code-snippets'), 'utf8'),
);

/** Replaces each placeholder with its default value, as accepting every default would. */
function expandSnippet(body: string[]): string {
    const values = new Map<string, string>();
    let text = body.join('\n');
    // A default can contain braces, escaped closing braces and references to
    // earlier tab stops, but not nested placeholders.
    const placeholder = /\$\{(\d+):((?:[^}$\\]|\\.|\$\d+)*)\}/;
    for (let match = placeholder.exec(text); match; match = placeholder.exec(text)) {
        const value = match[2].replace(/\$(\d+)/g, (_, n) => values.get(n) ?? '').replace(/\\(.)/g, '$1');
        values.set(match[1], value);
        text = text.slice(0, match.index) + value + text.slice(match.index + match[0].length);
    }
    return text.replace(/\$(\d+)/g, (_, n) => values.get(n) ?? '');
}

// Snippets that are expressions or directives need surrounding code to compile.
const contexts: Record<string, (snippet: string) => string> = {
    'If expression': (s) => `float a = 1.0;\nbool condition = a > 0.5;\nfloat value_if_true = 1.0;\nfloat value_if_false = 0.0;\nfloat x = ${s};\n`,
    'Switch expression': (s) => `int index = 1;\nfloat value_0 = 0.0;\nfloat value_1 = 1.0;\nfloat value_2 = 2.0;\nfloat x = ${s};\n`,
};

test('expandSnippet uses the default values', () => {
    assert.equal(expandSnippet(['${1:a} ${2:{b\\}} $1 ${0:$2}']), 'a {b} a {b}');
});

for (const [name, snippet] of Object.entries(snippets)) {
    test(`the "${name}" snippet compiles`, { skip }, async () => {
        const expanded = expandSnippet(snippet.body);
        const source = contexts[name] ? contexts[name](expanded) : expanded;
        await engine.compile(source, [], settings);
    });
}
