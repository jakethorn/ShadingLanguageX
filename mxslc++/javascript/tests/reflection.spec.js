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

    test('reflects the types', () =>
    {
        const names = mx.reflect().types.map((type) => type.name);

        expect(names).toContain('vector3');
        expect(names).toContain('vec3');
        expect(names).toContain('void');
        expect(names).not.toContain('float3');
    });

    test('reflects the functions', () =>
    {
        const names = mx.reflect().functions.map((func) => func.name);

        expect(new Set(names).size).toBe(names.length);
        expect(names).toContain('add');
        expect(names).toContain('standard_surface');
        expect(names).toContain('combine');
        expect(names).not.toContain('and');
        expect(names).not.toContain('__add__');
    });

    test('reflects the functions of a MaterialX version', () =>
    {
        const opts = new mx.CompileOptions();
        opts.version = '1.38.10';
        const names = mx.reflect(opts).functions.map((func) => func.name);
        opts.delete();

        expect(names).toContain('add');
        expect(names).not.toContain('hextiledimage');
    });

    test('reflects which types are aliases and built in', () =>
    {
        const source = 'class Light\n{\n    vec3 pos;\n}\n\nusing Brightness = float;\n';
        const types = new Map(mx.reflectFromString(source).types.map((type) => [type.name, type]));

        expect(types.get('vec3')).toMatchObject({ name: 'vec3', isAlias: true, source: mx.Source.SlxStandardLibrary });
        expect(types.get('Brightness')).toMatchObject({ name: 'Brightness', isAlias: true, source: mx.Source.UserDefined });
        expect(types.get('vector3')).toMatchObject({ name: 'vector3', isAlias: false, source: mx.Source.MaterialX });
        expect(types.get('Light')).toMatchObject({ name: 'Light', isAlias: false, source: mx.Source.UserDefined });
    });

    test('reflects where types are defined', () =>
    {
        const opts = new mx.CompileOptions();
        opts.addSource('types.mxsl', 'class Light\n{\n    vec3 pos;\n}\n');
        const source = '#include "types.mxsl"\nusing Brightness = float;\n';
        const types = new Map(mx.reflectFromString(source, opts).types.map((type) => [type.name, type]));
        opts.delete();

        expect(types.get('Light')).toMatchObject({ path: 'types.mxsl', line: 1 });
        expect(types.get('Brightness')).toMatchObject({ path: null, line: 2 });
        expect(types.get('vec3').path).toMatch(/^\/.*\/stdlib\.mxsl$/);
        expect(types.get('float').path).toMatch(/^\/.*\/stdlib_defs\.mtlx$/);
        expect(types.get('float').line).toBeNull();
        expect(types.get('void')).toMatchObject({ path: null, line: null });
    });

    test('reflects aliased types, fields and methods', () =>
    {
        const source = `
class Light
{
    vec3 pos;
    float intensity;

    float brightness()
    {
        return intensity * 2.0;
    }
}

using Ray = {vec3 origin, vec3 dir};
`;
        const types = new Map(mx.reflectFromString(source).types.map((type) => [type.name, type]));

        expect(types.get('vec3').aliasedType).toBe('vector3');
        expect(types.get('Light').aliasedType).toBeNull();
        expect(types.get('Light').fields).toEqual([{ name: 'pos', type: 'vector3' }, { name: 'intensity', type: 'float' }]);
        expect(types.get('Light').methods.map((method) => method.name)).toEqual(['brightness']);
        expect(types.get('Light').methods[0].overloads[0].classType).toBe('Light');
        expect(types.get('Ray').aliasedType).toBe('{vector3 origin, vector3 dir}');
    });

    test('reflects function overloads', () =>
    {
        const source = 'inline float twice(uniform float x = 1.0)\n{\n    return x * 2.0;\n}\n';
        const functions = new Map(mx.reflectFromString(source).functions.map((func) => [func.name, func]));
        const twice = functions.get('twice').overloads[0];
        const add = functions.get('add').overloads.find((overload) => overload.templateType === 'float');

        expect(twice.parameters).toEqual([{ name: 'x', type: 'float', defaultExpression: '1.0', modifiers: ['uniform'] }]);
        expect(twice).toMatchObject({ returnType: 'float', templateType: null, modifiers: ['inline'], line: 1 });
        expect(twice.source).toBe(mx.Source.UserDefined);
        expect(add).toMatchObject({ returnType: 'float', templateType: 'float', modifiers: [], line: null });
        expect(add.source).toBe(mx.Source.MaterialX);
        expect(add.path).toMatch(/\/stdlib_defs\.mtlx$/);
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

    test('reflects the types and functions of a source', () =>
    {
        const source = 'class Light\n{\n    vec3 pos;\n}\n\nfloat brighten(float x)\n{\n    return x * 2.0;\n}\n';
        const reflection = mx.reflectFromString(source);

        expect(reflection.types.map((type) => type.name)).toContain('Light');
        expect(reflection.functions.map((func) => func.name)).toContain('brighten');
        expect(reflection.functions.map((func) => func.name)).toContain('add');
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
