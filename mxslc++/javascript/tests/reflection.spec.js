import { test, expect } from '@playwright/test';
import { loadMxslc } from './testHelpers.js';

test.describe('Keywords and reflection', () =>
{
    let mx;

    test.beforeAll(async () =>
    {
        mx = await loadMxslc();
    });

    test('returns the SLX keywords', () =>
    {
        const keywords = mx.getKeywords();

        expect(Array.isArray(keywords)).toBe(true);
        expect(keywords).toContain('if');
        expect(keywords).toContain('node');
        expect(keywords).toContain('xor');
    });

    test('reflects the type names', () =>
    {
        const typeNames = mx.reflect().typeNames;

        expect(typeNames).toContain('vector3');
        expect(typeNames).toContain('vec3');
        expect(typeNames).toContain('void');
        expect(typeNames).not.toContain('float3');
    });

    test('reflects the function names', () =>
    {
        const functionNames = mx.reflect().functionNames;

        expect(new Set(functionNames).size).toBe(functionNames.length);
        expect(functionNames).toContain('add');
        expect(functionNames).toContain('standard_surface');
        expect(functionNames).toContain('combine');
        expect(functionNames).not.toContain('and');
        expect(functionNames).not.toContain('__add__');
    });

    test('reflects the function names of a MaterialX version', () =>
    {
        const opts = new mx.CompileOptions();
        opts.version = '1.38.10';
        const functionNames = mx.reflect(opts).functionNames;
        opts.delete();

        expect(functionNames).toContain('add');
        expect(functionNames).not.toContain('hextiledimage');
    });

    test('reflects the type and function names of a source', () =>
    {
        const source = 'class Light\n{\n    vec3 pos;\n}\n\nfloat brighten(float x)\n{\n    return x * 2.0;\n}\n';
        const reflection = mx.reflectFromString(source);

        expect(reflection.typeNames).toContain('Light');
        expect(reflection.functionNames).toContain('brighten');
        expect(reflection.functionNames).toContain('add');
    });

    test('reflects no globals without source', () =>
    {
        expect(mx.reflect().globals).toEqual([]);
    });

    test('reflects the globals of a source', () =>
    {
        const source = `
global float roughness = 0.25;
global color3 tint;
global matrix33 transform;
global float u = texcoord<vec2>().x;
`;
        const globals = mx.reflectFromString(source).globals;

        expect(globals).toEqual([
            { name: 'roughness', type: 'float', defaultValue: 0.25 },
            { name: 'tint', type: 'color3', defaultValue: [0, 0, 0] },
            { name: 'transform', type: 'matrix33', defaultValue: [[0, 0, 0], [0, 0, 0], [0, 0, 0]] },
            { name: 'u', type: 'float', defaultValue: null },
        ]);
    });

    test('reflects a source with compile options', () =>
    {
        const opts = new mx.CompileOptions();
        opts.addSource('values.mxsl', 'global int count = 3;');
        const globals = mx.reflectFromString('#include "values.mxsl"\n', opts).globals;
        opts.delete();

        expect(globals).toEqual([{ name: 'count', type: 'integer', defaultValue: 3 }]);
    });

    test('throws a CompileError for invalid source', () =>
    {
        expect(() => mx.reflectFromString('float x = ;')).toThrow(expect.objectContaining({ name: 'CompileError' }));
    });
});
