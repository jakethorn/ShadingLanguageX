import assert from 'node:assert/strict';
import * as path from 'node:path';
import { test } from 'node:test';
import { findMissingFile, locateCompileError, locateDecompileError, parseCompileError } from '../src/compileErrors';
import { collectSources, type ReadSource } from '../src/includes';

function readFrom(files: Record<string, string>): ReadSource {
    const resolved = new Map(Object.entries(files).map(([file, text]) => [path.resolve(file), text]));
    return (filePath) => resolved.get(path.resolve(filePath));
}

const root = path.resolve('/work/main.mxsl');

test('parseCompileError reads the file and line prefix', () => {
    assert.deepEqual(parseCompileError('line 2: Invalid expression'), { fileName: undefined, line: 2, message: 'Invalid expression' });
    assert.deepEqual(parseCompileError('colors.mxsl, line 14: Unexpected token: ;'), {
        fileName: 'colors.mxsl',
        line: 14,
        message: 'Unexpected token: ;',
    });
});

test('parseCompileError reads scanning errors and errors without a location', () => {
    assert.deepEqual(parseCompileError('Scanning error on line 3, character: $'), {
        line: 3,
        message: 'Scanning error on line 3, character: $',
    });
    assert.deepEqual(parseCompileError('MaterialX version 9.9.9 libraries could not be found.\n'), {
        message: 'MaterialX version 9.9.9 libraries could not be found.',
    });
});

test('parseCompileError strips terminal colour codes', () => {
    assert.equal(parseCompileError('\x1b[31mline 1: Bad\x1b[0m').message, 'Bad');
});

test('findMissingFile reads the path from a missing file error', () => {
    assert.equal(findMissingFile('File colors.mxsl could not be found.\nSearched paths:\ncolors.mxsl\n'), 'colors.mxsl');
    assert.equal(findMissingFile('line 1: Invalid expression'), undefined);
});

test('locateCompileError puts an error in the compiled document on its line', () => {
    const text = 'float a = 1.0;\n    float b = nope;   \n';
    const reports = locateCompileError('line 2: Variable or parameterless function not defined: nope', text, {
        sources: [],
        missing: [],
    });
    assert.deepEqual(reports, [
        {
            span: { filePath: undefined, line: 1, start: 4, end: 19 },
            message: 'Variable or parameterless function not defined: nope',
        },
    ]);
});

test('locateCompileError reports an error in a nested include in that file and on the top-level #include', () => {
    const text = 'float a = 1.0;\n#include "sub/a.mxsl"\n';
    const sourceSet = collectSources(
        root,
        text,
        [],
        readFrom({
            '/work/sub/a.mxsl': '#include "b.mxsl"\n',
            '/work/sub/b.mxsl': '\nfloat b = nope;\n',
        }),
    );
    const reports = locateCompileError('b.mxsl, line 2: Bad', text, sourceSet);
    const inFile = { filePath: path.resolve('/work/sub/b.mxsl'), line: 1, start: 0, end: 15 };
    assert.deepEqual(reports, [
        { span: inFile, message: 'Bad' },
        {
            span: { filePath: undefined, line: 1, start: 9, end: 21 },
            message: 'Error in included file "sub/b.mxsl": Bad',
            related: { span: inFile, message: 'Bad' },
        },
    ]);
});

test('locateCompileError puts a missing include on its directive', () => {
    const text = 'float a = 1.0;\n#include "gone.mxsl"\n';
    const sourceSet = collectSources(root, text, [], readFrom({}));
    const [report] = locateCompileError('File gone.mxsl could not be found.\nSearched paths:\ngone.mxsl\n', text, sourceSet);
    assert.deepEqual(report.span, { filePath: undefined, line: 1, start: 9, end: 20 });
    assert.match(report.message, /^Cannot find "gone.mxsl"/);
});

test('locateCompileError puts a missing include in an included file on both directives', () => {
    const text = '#include "a.mxsl"\n';
    const sourceSet = collectSources(root, text, [], readFrom({ '/work/a.mxsl': '\n#include "gone.mxsl"\n' }));
    const reports = locateCompileError('File gone.mxsl could not be found.\n', text, sourceSet);
    assert.deepEqual(
        reports.map((r) => r.span),
        [
            { filePath: path.resolve('/work/a.mxsl'), line: 1, start: 9, end: 20 },
            { filePath: undefined, line: 0, start: 9, end: 17 },
        ],
    );
});

test('locateCompileError falls back to the first line for errors without a location', () => {
    const [report] = locateCompileError('MaterialX version 9.9.9 libraries could not be found.', '  #version 9.9.9\n', {
        sources: [],
        missing: [],
    });
    assert.deepEqual(report.span, { filePath: undefined, line: 0, start: 2, end: 16 });
});

test('locateDecompileError uses the character offset of an XML parse error', () => {
    const text = '<?xml version="1.0"?>\n<materialx version="1.39">\n  </addx>\n</materialx>\n';
    const offset = text.indexOf('addx');
    const report = locateDecompileError(`XML parse error (Start-end tags mismatch at character ${offset})`, text);
    assert.deepEqual(report.span, { filePath: undefined, line: 2, start: 2, end: 9 });
});
