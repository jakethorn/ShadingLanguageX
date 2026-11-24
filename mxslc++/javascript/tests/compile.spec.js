import { test, expect } from '@playwright/test';
import { loadMxslc } from './testHelpers.js';

test.describe('Compile (SLX -> MTLX)', () =>
{
    let mx;

    test.beforeAll(async () =>
    {
        mx = await loadMxslc();
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

    test('compiles functions with the default function behaviour', () =>
    {
        const source = 'float twice(float x) { return x * 2.0; }\nfloat y = twice(texcoord<vec2>().x);';
        const opts = new mx.CompileOptions();
        expect(opts.defaultFunctionBehaviour).toBe(mx.FunctionBehaviour.NodeDef);
        expect(mx.compileSlxToMtlx(source, opts)).toContain('<nodedef');

        opts.defaultFunctionBehaviour = mx.FunctionBehaviour.Inline;
        expect(mx.compileSlxToMtlx(source, opts)).not.toContain('<nodedef');

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
        opts.version = '1.39.5';
        opts.errorOnMissingGlobals = false;
        opts.errorOnUnusedGlobals = false;

        const mtlx = mx.compileSlxToMtlx('float z = add(1.0, 2.0);', opts);

        expect(mtlx).toContain('<materialx');

        opts.delete();
    });

    test('throws when the SLX source is invalid', () =>
    {
        const opts = new mx.CompileOptions();
        expect(() => mx.compileSlxToMtlx('float = ;', opts)).toThrow();
        opts.delete();
    });

    test('lists every error in the diagnostics of the CompileError', () =>
    {
        const opts = new mx.CompileOptions();
        let error;
        try
        {
            mx.compileSlxToMtlx('float a = "text";\nstring b = 1.0;', opts);
        }
        catch (e)
        {
            error = e;
        }
        opts.delete();

        expect(error.name).toBe('CompileError');
        expect(error.diagnostics.map((d) => d.line)).toEqual([1, 2]);
        expect(error.diagnostics[0].file).toBe('');
        expect(error.diagnostics[1].message).toBe('Cannot assign an expression of type float to a variable or parameter of type string');
    });

    test('stops after maxErrors errors', () =>
    {
        const opts = new mx.CompileOptions();
        expect(opts.maxErrors).toBe(20);
        opts.maxErrors = 2;
        let error;
        try
        {
            mx.compileSlxToMtlx('float a = "x";\nfloat b = "x";\nfloat c = "x";', opts);
        }
        catch (e)
        {
            error = e;
        }
        opts.delete();

        expect(error.diagnostics.length).toBe(3);
        expect(error.diagnostics[2].message).toBe('Too many errors, stopping compilation');
    });
});
