import { test, expect } from '@playwright/test';
import { loadMxslc } from './testHelpers.js';

test.describe('Compile (SLX -> MTLX, in-memory sources)', () =>
{
    let mx;

    test.beforeAll(async () =>
    {
        mx = await loadMxslc();
    });

    // A root that #includes a sibling file the browser only has as a string.
    const ROOT = '#include "values.mxsl"\nfloat z = value + 1.0;';
    const VALUES = 'float value = 12;';

    test('resolves #include against an in-memory source', () =>
    {
        const opts = new mx.CompileOptions();
        // Graph reduction would fold the constant float away, leaving nothing
        // to assert on, so keep the nodes.
        opts.reduceGraph = false;
        opts.addSource('values.mxsl', VALUES);

        const mtlx = mx.compileSlxToMtlx(ROOT, opts);
        opts.delete();

        expect(mtlx).toContain('<materialx');
        // `value` only exists in the in-memory source, so its constant
        // appearing in the graph proves the contents were used, not just that
        // some file of that name was resolved.
        expect(mtlx).toContain('name="z"');
        expect(mtlx).toContain('value="12"');
    });

    test('resolves a nested #include against in-memory sources', () =>
    {
        const opts = new mx.CompileOptions();
        opts.reduceGraph = false;
        opts.addSource('values.mxsl', '#include "base.mxsl"\nfloat value = base + base;');
        opts.addSource('base.mxsl', 'float base = 6;');

        const mtlx = mx.compileSlxToMtlx(ROOT, opts);
        opts.delete();

        expect(mtlx).toContain('<materialx');
        expect(mtlx).toContain('name="z"');
    });

    test('throws when an #include target has not been supplied', () =>
    {
        const opts = new mx.CompileOptions();
        expect(() => mx.compileSlxToMtlx(ROOT, opts)).toThrow();
        opts.delete();
    });

    test('in-memory sources do not leak between CompileOptions instances', () =>
    {
        const withSource = new mx.CompileOptions();
        withSource.addSource('values.mxsl', VALUES);
        mx.compileSlxToMtlx(ROOT, withSource);
        withSource.delete();

        // A fresh instance knows nothing about the sources above, so the same
        // root must fail to resolve its #include.
        const withoutSource = new mx.CompileOptions();
        expect(() => mx.compileSlxToMtlx(ROOT, withoutSource)).toThrow();
        withoutSource.delete();
    });

    test('resolves nested #include relative to the including source', () =>
    {
        const opts = new mx.CompileOptions();
        opts.reduceGraph = false;
        // values.mxsl includes "base.mxsl", which must resolve to
        // sub/base.mxsl rather than to a file at the top of the overlay.
        opts.addSource('sub/values.mxsl', '#include "base.mxsl"\nfloat value = base + base;');
        opts.addSource('sub/base.mxsl', 'float base = 6;');

        const mtlx = mx.compileSlxToMtlx(
            '#include "sub/values.mxsl"\nfloat z = value + 1.0;',
            opts,
        );
        opts.delete();

        expect(mtlx).toContain('name="z"');
    });

    test('clearSources removes previously added sources', () =>
    {
        const opts = new mx.CompileOptions();
        opts.addSource('values.mxsl', VALUES);
        opts.clearSources();

        expect(() => mx.compileSlxToMtlx(ROOT, opts)).toThrow();
        opts.delete();
    });
});
