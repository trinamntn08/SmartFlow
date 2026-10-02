import assert from 'node:assert/strict';
import test from 'node:test';
import { readFileSync } from 'node:fs';
import {
  parseProject,
  parseComponent,
  serializeComponent,
  validateComponent,
  extractComponent,
} from '../src/index.ts';
const file = () =>
  parseProject(
    readFileSync(new URL('../../../examples/components.smartflow', import.meta.url), 'utf8'),
  );
test('standalone component transport preserves snapshots and opaque fields', () => {
  const definition = validateComponent(file().project.graphs[0]!.nodes[1]!.component);
  definition.future = { retained: true };
  assert.deepEqual(parseComponent(serializeComponent(definition)), definition);
  assert.throws(
    () =>
      parseComponent(
        serializeComponent(definition).replace('"version": 1', '"version": 1, "version": 1'),
      ),
    /Duplicate/,
  );
});
test('component validation rejects nested/cyclic bodies, conflicting targets and incomplete boundaries', () => {
  const definition = validateComponent(file().project.graphs[0]!.nodes[1]!.component);
  const nested = structuredClone(definition);
  nested.graph.nodes[0]!.packageId = 'smartflow.components';
  nested.graph.nodes[0]!.typeId = 'instance';
  assert.throws(() => validateComponent(nested), /Nested/);
  const cycle = structuredClone(definition);
  cycle.graph.connections.push({
    id: 'cycle',
    source: { nodeId: 'summary', portId: 'out' },
    target: { nodeId: 'filter', portId: 'in' },
  });
  assert.throws(() => validateComponent(cycle), /cycle/);
  const duplicate = structuredClone(definition);
  duplicate.inputs.push({ ...duplicate.inputs[0]!, id: 'second' });
  assert.throws(() => validateComponent(duplicate), /Duplicate exposed/);
  const source = parseProject(
    readFileSync(new URL('../../../examples/data.smartflow', import.meta.url), 'utf8'),
  );
  assert.throws(
    () => extractComponent(source, 'graph', ['filter'], { ...definition, inputs: [], outputs: [] }),
    /boundary/,
  );
});
