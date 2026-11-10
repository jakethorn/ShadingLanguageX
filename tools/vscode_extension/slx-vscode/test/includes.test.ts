import assert from 'node:assert/strict';
import * as fs from 'node:fs';
import * as os from 'node:os';
import * as path from 'node:path';
import { test } from 'node:test';
import { blankComments, collectSources, findDirectives, joinKey, listDirectiveEntries, type ReadSource } from '../src/includes';

/** A fake file system, keyed by absolute path. */
function readFrom(files: Record<string, string>): ReadSource {
    const resolved = new Map(Object.entries(files).map(([file, text]) => [path.resolve(file), text]));
    return (filePath) => resolved.get(path.resolve(filePath));
}

const root = path.resolve('/work/main.mxsl');

test('findDirectives finds include and library directives with their positions', () => {
    const text = 'float a = 1.0;\n  #include "colors.mxsl"\n#library "nodes.mtlx"\n';
    assert.deepEqual(findDirectives(text), [
        { kind: 'include', target: 'colors.mxsl', line: 1, start: 11, end: 24 },
        { kind: 'library', target: 'nodes.mtlx', line: 2, start: 9, end: 21 },
    ]);
});

test('findDirectives ignores directives in comments', () => {
    const text = '// #include "a.mxsl"\n/*\n#include "b.mxsl"\n*/\n#include "c.mxsl" // #include "d.mxsl"\n';
    assert.deepEqual(
        findDirectives(text).map((d) => d.target),
        ['c.mxsl'],
    );
});

test('blankComments keeps line breaks and string contents', () => {
    assert.equal(blankComments('a /* b\nc */ "// d" // e\r\nf'), 'a     \n     "// d"     \r\nf');
});

test('joinKey joins to the directory of the including key', () => {
    assert.equal(joinKey('', 'a.mxsl'), 'a.mxsl');
    assert.equal(joinKey('', './a.mxsl'), 'a.mxsl');
    assert.equal(joinKey('sub/a.mxsl', 'b.mxsl'), 'sub/b.mxsl');
    assert.equal(joinKey('sub/a.mxsl', '../c.mxsl'), 'c.mxsl');
    assert.equal(joinKey('../shared/a.mxsl', 'b.mxsl'), '../shared/b.mxsl');
    assert.equal(joinKey('sub/a.mxsl', '/abs/x.mxsl'), '/abs/x.mxsl');
});

test('collectSources keys nested includes by their place in the include tree', () => {
    const read = readFrom({
        '/work/sub/a.mxsl': '#include "b.mxsl"\n',
        '/work/sub/b.mxsl': 'const float B = 1.0;\n',
    });
    const { sources, missing } = collectSources(root, '#include "sub/a.mxsl"\n', [], read);
    assert.deepEqual(
        sources.map((s) => [s.key, s.filePath]),
        [
            ['sub/a.mxsl', path.resolve('/work/sub/a.mxsl')],
            ['sub/b.mxsl', path.resolve('/work/sub/b.mxsl')],
        ],
    );
    assert.equal(missing.length, 0);
    assert.equal(sources[1].origin.fromKey, 'sub/a.mxsl');
});

test('collectSources looks next to the including file before the include paths', () => {
    const read = readFrom({
        '/work/lib.mxsl': 'next to the file',
        '/libs/lib.mxsl': 'in the include path',
        '/libs/only_here.mxsl': 'only in the include path',
    });
    const { sources } = collectSources(root, '#include "lib.mxsl"\n#include "only_here.mxsl"\n', ['/libs'], read);
    assert.deepEqual(
        sources.map((s) => [s.key, s.text]),
        [
            ['lib.mxsl', 'next to the file'],
            ['only_here.mxsl', 'only in the include path'],
        ],
    );
});

test('collectSources resolves an unsaved document only through the include paths', () => {
    const read = readFrom({ '/libs/a.mxsl': 'A' });
    const { sources, missing } = collectSources(undefined, '#include "a.mxsl"\n#include "b.mxsl"\n', ['/libs'], read);
    assert.deepEqual(
        sources.map((s) => s.key),
        ['a.mxsl'],
    );
    assert.deepEqual(
        missing.map((m) => m.directive.target),
        ['b.mxsl'],
    );
});

test('collectSources records missing files with the directive that wanted them', () => {
    const read = readFrom({ '/work/a.mxsl': '\n#include "gone.mxsl"\n' });
    const { missing } = collectSources(root, '#include "a.mxsl"\n', [], read);
    assert.equal(missing.length, 1);
    assert.equal(missing[0].fromKey, 'a.mxsl');
    assert.equal(missing[0].directive.line, 1);
});

test('collectSources stops at include cycles', () => {
    const read = readFrom({
        '/work/a.mxsl': '#include "b.mxsl"\n',
        '/work/b.mxsl': '#include "a.mxsl"\n',
    });
    const { sources } = collectSources(root, '#include "a.mxsl"\n', [], read);
    assert.deepEqual(
        sources.map((s) => s.key),
        ['a.mxsl', 'b.mxsl'],
    );
});

test('collectSources also registers libraries under the path as written', () => {
    const read = readFrom({
        '/work/sub/a.mxsl': '#library "nodes.mtlx"\n',
        '/work/sub/nodes.mtlx': '<materialx />',
    });
    const { sources } = collectSources(root, '#include "sub/a.mxsl"\n', [], read);
    assert.deepEqual(
        sources.map((s) => s.key),
        ['sub/a.mxsl', 'sub/nodes.mtlx', 'nodes.mtlx'],
    );
});

test('listDirectiveEntries lists folders and files with the extension, once each, without hidden ones or the excluded file', () => {
    const temp = fs.mkdtempSync(path.join(os.tmpdir(), 'slx-entries-'));
    try {
        const first = path.join(temp, 'first');
        const second = path.join(temp, 'second');
        for (const file of ['first/a.mxsl', 'first/main.mxsl', 'first/b.mtlx', 'first/.hidden.mxsl', 'second/a.mxsl', 'second/c.MXSL']) {
            fs.mkdirSync(path.dirname(path.join(temp, file)), { recursive: true });
            fs.writeFileSync(path.join(temp, file), '');
        }
        fs.mkdirSync(path.join(first, 'sub'));
        const entries = listDirectiveEntries([first, second, path.join(temp, 'missing')], '.mxsl', path.join(first, 'main.mxsl'));
        assert.deepEqual(
            entries.map((e) => `${e.name}${e.isFolder ? '/' : ''}`).sort(),
            ['a.mxsl', 'c.MXSL', 'sub/'],
        );
    } finally {
        fs.rmSync(temp, { recursive: true, force: true });
    }
});
