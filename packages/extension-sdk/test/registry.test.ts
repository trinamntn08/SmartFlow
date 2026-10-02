import assert from 'node:assert/strict';
import test from 'node:test';
import { ExtensionRegistry, validateParameter, type ExtensionDefinition } from '../src/index.ts';

function extension(): ExtensionDefinition {
  return {
    id: 'test',
    version: '1',
    apiVersion: 1,
    dependencies: [],
    types: [{ id: 'test.number@1', version: 1, accepts: (value) => typeof value === 'number' }],
    nodes: [
      {
        id: 'number',
        version: 1,
        label: 'Number',
        inputs: [{ id: 'in', typeId: 'test.number@1', required: false }],
        outputs: [{ id: 'out', typeId: 'test.number@1', required: true }],
        parameters: [
          {
            id: 'value',
            label: 'Value',
            control: 'number',
            defaultValue: 0,
            minimum: -10,
            maximum: 10,
          },
        ],
        capabilities: ['browser'],
        execute: async () => ({ out: 1 }),
      },
    ],
    viewers: [],
    migrations: [],
  };
}
test('registry resolves exact persisted IDs and versions, and types ports without domain imports', () => {
  const registry = new ExtensionRegistry([extension()]);
  const node = { id: 'n', packageId: 'test', typeId: 'number', version: 1, parameters: {} };
  assert.equal(registry.node(node)?.label, 'Number');
  assert.equal(registry.node({ ...node, version: 2 }), undefined);
  assert.equal(registry.extension('test', '2'), undefined);
  assert.equal(registry.compatible(node, 'out', node, 'in'), true);
  assert.equal(registry.compatible(node, 'missing', node, 'in'), false);
});
test('registration rejects duplicate IDs, missing dependencies and unknown types', () => {
  assert.throws(() => new ExtensionRegistry([extension(), extension()]));
  const unknown = extension();
  unknown.nodes[0]!.outputs[0]!.typeId = 'missing';
  assert.throws(() => new ExtensionRegistry([unknown]), /Unknown port type/);
  const dependent = extension();
  dependent.dependencies = [{ id: 'missing', version: '1' }];
  assert.throws(() => new ExtensionRegistry([dependent]), /Missing extension dependency/);
});
test('parameter controls enforce primitive type/range and selections', () => {
  const parameter = extension().nodes[0]!.parameters[0]!;
  for (const value of [Infinity, -11, 11, '1', null])
    assert.throws(() => validateParameter(parameter, value));
  validateParameter(parameter, 3.5);
  assert.throws(() =>
    validateParameter(
      { id: 's', label: 'Choice', control: 'select', defaultValue: 'a', options: ['a'] },
      'b',
    ),
  );
});
test('registered contracts are independent immutable copies', () => {
  const source = extension();
  const registry = new ExtensionRegistry([source]);
  source.nodes[0]!.label = 'Changed';
  assert.equal(registry.extension('test')!.nodes[0]!.label, 'Number');
  assert.throws(() => {
    registry.extension('test')!.nodes[0]!.label = 'Mutated';
  });
});
