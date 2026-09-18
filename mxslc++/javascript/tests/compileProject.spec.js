import { test, expect } from '@playwright/test';
import { loadMxslc } from './testHelpers.js';

test.describe('Compile project (SLX -> MTLX, multi-file / #include)', () =>
{
    let mx;

    test.beforeAll(async () =>
    {
        mx = await loadMxslc();
    });

    // The colors.mxsl / include_example.mxsl pair from
    // docs/LanguageSpecification.md's File Inclusion section, trimmed to
    // one constant.
    const ROOT_WITH_INCLUDE = '#include "colors.mxsl"\nstandard_surface(base_color=RED);';
    const COLORS_FILE = 'const color3 RED = color3{1, 0, 0};';

    test('resolves #include against a supplied virtual file', () =>
    {
        const opts = new mx.CompileOptions();
        const mtlx = mx.compileProjectToMtlx(ROOT_WITH_INCLUDE, { 'colors.mxsl': COLORS_FILE }, opts);
        opts.delete();

        expect(mtlx).toContain('<materialx');
        expect(mtlx).toContain('standard_surface');
    });

    test('treats a missing/empty files argument as a self-contained compile', () =>
    {
        const slx = 'float z = add(1.0, 2.0);';

        for (const files of [undefined, null, {}])
        {
            const opts = new mx.CompileOptions();
            const mtlx = mx.compileProjectToMtlx(slx, files, opts);
            opts.delete();
            expect(mtlx).toContain('<materialx');
        }
    });

    test('throws when an #include target is not among the supplied files', () =>
    {
        const opts = new mx.CompileOptions();
        expect(() => mx.compileProjectToMtlx(ROOT_WITH_INCLUDE, {}, opts)).toThrow();
        opts.delete();
    });

    test('does not leak virtual files from one call into the next', () =>
    {
        const opts1 = new mx.CompileOptions();
        const mtlx = mx.compileProjectToMtlx(ROOT_WITH_INCLUDE, { 'colors.mxsl': COLORS_FILE }, opts1);
        opts1.delete();
        expect(mtlx).toContain('<materialx');

        // Same root, same file NAME required, but this call supplies no
        // files — if the previous call's scratch directory were not
        // cleared, colors.mxsl would still resolve here.
        const opts2 = new mx.CompileOptions();
        expect(() => mx.compileProjectToMtlx(ROOT_WITH_INCLUDE, {}, opts2)).toThrow();
        opts2.delete();
    });

    test('rejects a virtual file path that is absolute or escapes with \'..\'', () =>
    {
        const slx = 'float z = add(1.0, 2.0);';

        const opts1 = new mx.CompileOptions();
        expect(() => mx.compileProjectToMtlx(slx, { '../escape.mxsl': 'x' }, opts1)).toThrow();
        opts1.delete();

        const opts2 = new mx.CompileOptions();
        expect(() => mx.compileProjectToMtlx(slx, { '/absolute.mxsl': 'x' }, opts2)).toThrow();
        opts2.delete();
    });
});
