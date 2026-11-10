import assert from 'node:assert/strict';
import { test } from 'node:test';
import { parseNodeDefs } from '../src/libraryNodes';

const library = `<?xml version="1.0"?>
<materialx version="1.39">
  <!-- <nodedef name="ND_commented_out" node="commented_out" /> -->
  <nodedef name="ND_tint_color3" node="tint" nodegroup="adjustment">
    <input name="in" type="color3" value="1, 1, 1" uiname="Colour" />
    <input name="mode" type="string" value="multiply" enum="multiply,add" uniform="true" />
    <input name="n" type="vector3" defaultgeomprop="Nworld" />
    <output name="out" type="color3" />
  </nodedef>
  <nodedef name="ND_tint_float" node="tint">
    <input name="in" type="float" value="1" />
    <output name="out" type="float" />
  </nodedef>
  <nodegraph name="NG_tint_float" nodedef="ND_tint_float" />
</materialx>
`;

test('parseNodeDefs reads each node definition, its inputs and outputs', () => {
    const nodes = parseNodeDefs(library);
    assert.deepEqual(
        nodes.map((n) => n.name),
        ['tint', 'tint'],
    );
    assert.deepEqual(nodes[0].signature, {
        nodedef: 'ND_tint_color3',
        group: 'adjustment',
        inputs: [
            { name: 'in', type: 'color3', value: '1, 1, 1', uiname: 'Colour' },
            { name: 'mode', type: 'string', value: 'multiply', enum: 'multiply,add', uniform: true },
            { name: 'n', type: 'vector3', defaultgeomprop: 'Nworld' },
        ],
        outputs: [{ name: 'out', type: 'color3' }],
        versions: [],
    });
    assert.equal(library.slice(nodes[1].offset).split('\n')[0], '<nodedef name="ND_tint_float" node="tint">');
});

test('parseNodeDefs reads self-closing definitions, old single-output types and escaped values', () => {
    const nodes = parseNodeDefs('<nodedef name="ND_a" node="a" type="float"/><nodedef name="ND_b" node="b"><input name="s" type="string" value="&quot;x&quot; &amp; y"/></nodedef>');
    assert.deepEqual(nodes[0].signature.outputs, [{ name: 'out', type: 'float' }]);
    assert.equal(nodes[1].signature.inputs[0].value, '"x" & y');
});
