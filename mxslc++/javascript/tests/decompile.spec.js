import { test, expect } from '@playwright/test';
import { loadMxslc } from './testHelpers.js';

// Compound nodegraph test.
const NODE_GRAPH_MTLX = `<?xml version="1.0"?>
<materialx version="1.39">
  <nodegraph name="NG_scale">
    <input name="x" type="float" value="1.0" />
    <multiply name="m" type="float">
      <input name="in1" type="float" interfacename="x" />
      <input name="in2" type="float" value="2.0" />
    </multiply>
    <output name="out" type="float" nodename="m" />
  </nodegraph>
</materialx>`;

// Definition test which also tests attribute vs tag ordering.
const NODE_DEF_MTLX = `<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_fwidth" node="fwidth" nodegroup="math">
    <output name="out" type="float" />
    <input name="value" type="float" value="0" />
  </nodedef>
  <nodegraph name="NG_fwidth" nodedef="ND_fwidth">
    <output name="out" type="float" interfacename="value" />
  </nodegraph>
</materialx>`;

test.describe('Decompile (MTLX -> SLX)', () =>
{
    let mx;

    test.beforeAll(async () =>
    {
        mx = await loadMxslc();
    });

    test('decompiles a MTLX string to an SLX string', () =>
    {
        const mtlx = `<?xml version="1.0"?>
<materialx version="1.39">
  <add name="z" type="float">
    <input name="in1" type="float" value="1" />
    <input name="in2" type="float" value="2" />
  </add>
  <combine2 name="st" type="vector2">
    <input name="in1" type="float" nodename="z" />
    <input name="in2" type="float" value="4" />
  </combine2>
</materialx>`;

        const slx = mx.decompileMtlxToSlx(mtlx);

        expect(slx).toContain('z');
        expect(slx).toContain('st');
    });

    test('omits graph modifiers by default', () =>
    {
        // Both a compound nodegraph and a functional nodegraph omit the modifiers by default.
        const nodeGraphSlx = mx.decompileMtlxToSlx(NODE_GRAPH_MTLX);
        expect(nodeGraphSlx).not.toContain('[[nodegraph]]');
        expect(nodeGraphSlx).not.toContain('[[nodedef]]');

        const nodeDefSlx = mx.decompileMtlxToSlx(NODE_DEF_MTLX);
        expect(nodeDefSlx).not.toContain('[[nodegraph]]');
        expect(nodeDefSlx).not.toContain('[[nodedef]]');
    });

    test('emits graph modifiers when enabled', () =>
    {
        const opts = new mx.DecompileOptions();
        opts.emitFunctionModifiers = true;

        // Check for `[[nodegraph]]` and `[[nodedef]]` as appropriate.
        const nodeGraphSlx = mx.decompileMtlxToSlx(NODE_GRAPH_MTLX, opts);
        expect(nodeGraphSlx).toContain('[[nodegraph]]');
        expect(nodeGraphSlx).not.toContain('[[nodedef]]');

        const nodeDefSlx = mx.decompileMtlxToSlx(NODE_DEF_MTLX, opts);
        expect(nodeDefSlx).toContain('[[nodedef]]');
        expect(nodeDefSlx).not.toContain('[[nodegraph]]');

        opts.delete();

        // Attributes are parsed before modifiers, so `@nodegroup` must come first.
        const attrPos = nodeDefSlx.indexOf('@nodegroup "math"');
        const modifierPos = nodeDefSlx.indexOf('[[nodedef]]');
        expect(attrPos).toBeGreaterThanOrEqual(0);
        expect(modifierPos).toBeGreaterThanOrEqual(0);
        expect(attrPos).toBeLessThan(modifierPos);

        // The interface input is emitted as a (defaulted) parameter, but a
        // `[[nodegraph]]` function cannot be passed arguments, so the emitted
        // invocation must be argument-less.
        expect(nodeGraphSlx).toContain('scale(float x');
        expect(nodeGraphSlx).toContain('= scale();');

        const compileOpts = new mx.CompileOptions();
        let mtlx;
        expect(() => { mtlx = mx.compileSlxToMtlx(nodeGraphSlx, compileOpts); }).not.toThrow();
        compileOpts.delete();

        expect(mtlx).toContain('<materialx');
        expect(mtlx).toContain('nodegraph');
    });
});
