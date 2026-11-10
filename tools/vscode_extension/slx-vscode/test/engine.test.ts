// Runs the real WebAssembly compiler. Needs `npm run build` and the JsMxslc
// files in media/lib (gen_extension.py --converter copies them there); the
// tests are skipped when the files are missing.
import assert from 'node:assert/strict';
import * as fs from 'node:fs';
import * as path from 'node:path';
import { after, test } from 'node:test';
import { locateCompileError } from '../src/compileErrors';
import { ENGINE_MISSING_ERROR, EngineError, SlxEngine } from '../src/engine';
import type { CompileSettings } from '../src/engineProtocol';
import { collectSources } from '../src/includes';

const extensionDir = path.resolve(__dirname, '../..');
const workerPath = path.join(extensionDir, 'dist/engineWorker.mjs');
const libDir = path.join(extensionDir, 'media/lib');
const fixtures = path.join(extensionDir, 'test/fixtures');

const engine = new SlxEngine(workerPath, libDir);
const skip = engine.isBundled ? false : 'the JsMxslc files are not in media/lib';
after(() => engine.dispose());

const settings: CompileSettings = {
    materialxVersion: '1.39.5',
    reduceGraph: true,
    defaultFunctionBehaviour: 'nodedef',
    errorOnMissingGlobals: true,
};

function readFile(filePath: string): string | undefined {
    try {
        return fs.readFileSync(filePath, 'utf8');
    } catch {
        return undefined;
    }
}

async function compileFixture(name: string, overrides: Partial<CompileSettings> = {}) {
    const filePath = path.join(fixtures, name);
    const text = fs.readFileSync(filePath, 'utf8');
    const sourceSet = collectSources(filePath, text, [], readFile);
    const sources = sourceSet.sources.map(({ key, text }) => ({ key, text }));
    try {
        return { output: await engine.compile(text, sources, { ...settings, ...overrides }) };
    } catch (error) {
        assert.ok(error instanceof EngineError);
        return { reports: locateCompileError(error.message, text, sourceSet) };
    }
}

test('compiles a document whose includes include other files', { skip }, async () => {
    const { output } = await compileFixture('main.mxsl');
    assert.match(output!, /<standard_surface /);
    assert.match(output!, /value="1, 0, 0"/);
});

test('reports an error in an included file in that file and on the #include line', { skip }, async () => {
    const { reports } = await compileFixture('broken_include.mxsl');
    assert.deepEqual(
        reports!.map((r) => [r.span.filePath, r.span.line]),
        [
            [path.join(fixtures, 'sub/broken.mxsl'), 2],
            [undefined, 0],
        ],
    );
    assert.match(reports![0].message, /undefined_value/);
});

test('reports a missing include on its directive', { skip }, async () => {
    const { reports } = await compileFixture('missing_include.mxsl');
    assert.equal(reports!.length, 1);
    assert.deepEqual(reports![0].span, { filePath: undefined, line: 1, start: 9, end: 30 });
});

test('applies the compile settings', { skip }, async () => {
    const source = 'float add_one(float x) { return x + 1.0; }\nfloat y = add_one(2.0);\n';
    const nodedef = await engine.compile(source, [], settings);
    assert.match(nodedef, /<nodedef /);
    const inline = await engine.compile(source, [], { ...settings, defaultFunctionBehaviour: 'inline' });
    assert.doesNotMatch(inline, /<nodedef /);
    const older = await engine.compile(source, [], { ...settings, materialxVersion: '1.38.10' });
    assert.match(older, /<materialx version="1.38"/);
});

test('passes print output to the logger of the request that printed it', { skip }, async () => {
    const first: string[] = [];
    const second: string[] = [];
    await Promise.all([
        engine.compile('print "first";\n', [], settings, (stream, text) => first.push(`${stream}: ${text}`)),
        engine.compile('print "second";\n', [], settings, (stream, text) => second.push(`${stream}: ${text}`)),
    ]);
    assert.ok(first.includes('stdout: "first"') && !first.includes('stdout: "second"'), first.join('\n'));
    assert.ok(second.includes('stdout: "second"') && !second.includes('stdout: "first"'), second.join('\n'));
});

test('keeps working after a compile error', { skip }, async () => {
    await assert.rejects(engine.compile('float x = ;\n', [], settings), EngineError);
    assert.match(await engine.compile('float x = 1.0;\n', [], settings), /<materialx /);
});

test('decompiles MaterialX and rejects invalid XML', { skip }, async () => {
    const mtlx = await engine.compile('float x = sin(position().x);\n', [], settings);
    assert.match(await engine.decompile(mtlx), /sin\(/);
    await assert.rejects(engine.decompile('<materialx version="1.39"><bad'), /XML parse error/);
});

test('rejects requests when the compiler is not bundled', async () => {
    const missing = new SlxEngine(workerPath, path.join(extensionDir, 'no-such-folder'));
    await assert.rejects(missing.compile('float x = 1.0;', [], settings), { name: ENGINE_MISSING_ERROR });
});

test('replaces the worker after too many failed compiles', { skip }, async () => {
    const recycling = new SlxEngine(workerPath, libDir, 2);
    try {
        for (let i = 0; i < 2; i++) await assert.rejects(recycling.compile('float x = ;\n', [], settings));
        assert.match(await recycling.compile('float x = 1.0;\n', [], settings), /<materialx /);
        assert.equal(recycling.workersStarted, 2);
    } finally {
        recycling.dispose();
    }
});
