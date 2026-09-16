import { test, expect } from '@playwright/test';
import { loadMxslc } from './testHelpers.js';

test.describe('Compile (SLX -> MTLX)', () =>
{
    let mx;
    const fixtureDir = '/fixtures';

    function ensureFixtureDir()
    {
        try
        {
            mx.FS.mkdir(fixtureDir);
        }
        catch (error)
        {
            if (error?.code !== 'EEXIST')
                throw error;
        }
    }

    test.beforeAll(async () =>
    {
        mx = await loadMxslc();
        ensureFixtureDir();
    });

    test('compiles an SLX string to a valid MTLX document', () =>
    {
        const opts = new mx.CompileOptions();
        const slx = 'float z = add(1.0, 2.0);';
        const mtlx = mx.compileSlxToMtlx(slx, opts);
        opts.delete();

        expect(mtlx).toContain('<materialx');
    });

    test('produces nodes when graph reduction is disabled', () =>
    {
        const opts = new mx.CompileOptions();
        opts.reduceGraph = false;

        const mtlx = mx.compileSlxToMtlx('float z = add(1.0, 2.0);', opts);

        expect(mtlx).toContain('<materialx');
        expect(mtlx).toContain('name="z"');

        opts.delete();
    });

    test('compiles a surface material to nodes with default options', () =>
    {
        const slx = [
            'standard_surface(',
            '    normal = vec3{1, 0, 0}',
            ');',
        ].join('\n');

        const opts = new mx.CompileOptions();
        const mtlx = mx.compileSlxToMtlx(slx, opts);
        opts.delete();

        expect(mtlx).toContain('<materialx');
        expect(mtlx).toContain('standard_surface');
    });

    test('supports configuring CompileOptions', () =>
    {
        const opts = new mx.CompileOptions();
        opts.outputFile = '/fixtures/configured-output.mtlx';
        opts.version = '1.39.5';
        opts.macros = ['HELLO', ['WORLD', 'UNIVERSE']];
        opts.searchDirectories = ['/fixtures', '/libraries'];
        opts.includes = ['math.mxsl'];
        opts.libraries = ['stdlib_defs.mtlx'];
        opts.globals = {
            base: 0.8,
            enabled: true,
            nested: [1, 'two'],
        };
        opts.errorOnMissingGlobals = false;
        opts.errorOnUnusedGlobals = false;
        opts.funcName = 'my_custom_node';
        opts.funcArgs = [1, [1, 0.5, 0.2], 'diffuse.png'];
        opts.validateGraph = false;
        opts.reduceGraph = false;

        const macros = opts.macros.map((macro) =>
            Array.isArray(macro) ? macro.join('=') : macro
        ).sort();

        expect(opts.outputFile).toBe('/fixtures/configured-output.mtlx');
        expect(opts.version).toBe('1.39.5');
        expect(macros).toEqual(['HELLO', 'WORLD=UNIVERSE']);
        expect(opts.searchDirectories).toEqual(['/fixtures', '/libraries']);
        expect(opts.includes).toEqual(['math.mxsl']);
        expect(opts.libraries).toEqual(['stdlib_defs.mtlx']);
        expect(opts.globals.base).toBeCloseTo(0.8);
        expect(opts.globals.enabled).toBe(true);
        expect(opts.globals.nested).toEqual([1, 'two']);
        expect(opts.errorOnMissingGlobals).toBe(false);
        expect(opts.errorOnUnusedGlobals).toBe(false);
        expect(opts.funcName).toBe('my_custom_node');
        expect(opts.funcArgs).toEqual([1, [1, 0.5, 0.2], 'diffuse.png']);
        expect(opts.validateGraph).toBe(false);
        expect(opts.reduceGraph).toBe(false);

        const mtlx = mx.compileSlxToMtlx('float z = add(1.0, 2.0);', opts);

        expect(mtlx).toContain('<materialx');

        opts.delete();
    });

    test('compiles an SLX file in the WASM filesystem to an MTLX string', () =>
    {
        const sourcePath = `${fixtureDir}/simple.mxsl`;
        mx.FS.writeFile(sourcePath, 'float f = randomfloat() + 1.0;');

        const mtlx = mx.compileSlxFileToMtlx(sourcePath);

        expect(mtlx).toContain('<materialx');
    });

    test('compiles an SLX file in the WASM filesystem to an MTLX file', () =>
    {
        const sourcePath = `${fixtureDir}/simple-file.mxsl`;
        const outputPath = `${fixtureDir}/custom-output.mtlx`;
        const opts = new mx.CompileOptions();
        opts.outputFile = outputPath;

        mx.FS.writeFile(sourcePath, 'float f = randomfloat() + 1.0;');

        const returnedPath = mx.compileSlxFileToMtlxFile(sourcePath, opts);
        const mtlx = mx.FS.readFile(outputPath, { encoding: 'utf8' });

        expect(returnedPath).toBe(outputPath);
        expect(mtlx).toContain('<materialx');

        opts.delete();
    });

    test('throws when the SLX source is invalid', () =>
    {
        const opts = new mx.CompileOptions();
        expect(() => mx.compileSlxToMtlx('float = ;', opts)).toThrow();
        opts.delete();
    });
});
