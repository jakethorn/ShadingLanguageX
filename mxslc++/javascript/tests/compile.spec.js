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
});
