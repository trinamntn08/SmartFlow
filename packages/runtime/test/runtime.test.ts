import assert from 'node:assert/strict';
import test from 'node:test';
import type { ProjectFile } from '@smartflow/core';
import { ExtensionRegistry, type ExtensionDefinition } from '@smartflow/extension-sdk';
import { compileGraph, runGraph, RunGate } from '../src/index.ts';

function setup() {
  const extension: ExtensionDefinition = {
    id: 'test',
    version: '1',
    apiVersion: 1,
    dependencies: [],
    viewers: [],
    migrations: [],
    types: [
      {
        id: 'test.value@1',
        version: 1,
        accepts: (value) =>
          !!value &&
          typeof value === 'object' &&
          'value' in value &&
          typeof value.value === 'number',
      },
    ],
    nodes: [
      {
        id: 'value',
        label: 'Value',
        version: 1,
        inputs: [{ id: 'in', typeId: 'test.value@1', required: false }],
        outputs: [{ id: 'out', typeId: 'test.value@1', required: true }],
        parameters: [],
        capabilities: ['browser'],
        execute: async (_parameters, inputs, context) => {
          const input = inputs.in as { value: number } | undefined;
          if (context.nodeId === 'mutator' && input) input.value = 99;
          return { out: input ?? { value: 1 } };
        },
      },
    ],
  };
  const file: ProjectFile = {
    format: 'smartflow',
    schemaVersion: 1,
    project: {
      id: 'test',
      packages: [{ id: 'test', version: '1' }],
      assets: [],
      graphs: [
        {
          id: 'graph',
          nodes: ['source', 'mutator', 'observer'].map((id) => ({
            id,
            packageId: 'test',
            typeId: 'value',
            version: 1,
            parameters: {},
          })),
          connections: ['mutator', 'observer'].map((id) => ({
            id,
            source: { nodeId: 'source', portId: 'out' },
            target: { nodeId: id, portId: 'in' },
          })),
        },
      ],
    },
    workspace: {},
  };
  return { file, extension, registry: new ExtensionRegistry([extension]) };
}
test('fanout consumers and published outputs own independent values', async () => {
  const { file, registry } = setup();
  const original = structuredClone(file);
  const result = await runGraph(file, 'graph', registry, { clone: structuredClone });
  assert.equal(result.state, 'completed');
  assert.deepEqual(result.outputs.source!.out, { value: 1 });
  assert.deepEqual(result.outputs.mutator!.out, { value: 99 });
  assert.deepEqual(result.outputs.observer!.out, { value: 1 });
  assert.deepEqual(file, original);
});
test('unsupported contracts, parameters, capabilities and bad graph edges fail before any execution', async () => {
  for (const change of [
    (file: ProjectFile) => {
      file.project.packages[0]!.version = '2';
    },
    (file: ProjectFile) => {
      file.project.graphs[0]!.nodes[0]!.version = 2;
    },
    (file: ProjectFile) => {
      file.project.graphs[0]!.nodes[0]!.parameters.future = true;
    },
    (file: ProjectFile) => {
      file.project.graphs[0]!.connections[0]!.source.portId = 'missing';
    },
    (file: ProjectFile) => {
      file.project.graphs[0]!.connections[0]!.source.nodeId = 'missing';
    },
    (file: ProjectFile) => {
      const graph = file.project.graphs[0]!;
      graph.connections.push({ ...structuredClone(graph.connections[0]!), id: 'duplicate' });
    },
    (file: ProjectFile) => {
      file.project.graphs[0]!.connections.push({
        id: 'cycle',
        source: { nodeId: 'mutator', portId: 'out' },
        target: { nodeId: 'source', portId: 'in' },
      });
    },
  ]) {
    const { file, registry } = setup();
    change(file);
    const result = await runGraph(file, 'graph', registry, { clone: structuredClone });
    assert.equal(result.state, 'failed');
    assert.ok(result.diagnostics.length);
    assert.deepEqual(result.outputs, {});
    assert.ok(Object.values(result.statuses).every((value) => value === 'waiting'));
  }
  const { file, extension } = setup();
  extension.nodes[0]!.capabilities = [];
  assert.match(
    compileGraph(file, 'graph', new ExtensionRegistry([extension])).diagnostics[0]!.message,
    /capability/,
  );
  extension.nodes[0]!.inputs[0]!.required = true;
  assert.ok(
    compileGraph(file, 'graph', new ExtensionRegistry([extension])).diagnostics.some((item) =>
      item.message.includes('Required input'),
    ),
  );
});
test('output contract failure and cancellation discard the complete run', async () => {
  const { file, extension, registry } = setup();
  extension.nodes[0]!.execute = async () => ({ out: 'invalid' });
  const failed = await runGraph(file, 'graph', new ExtensionRegistry([extension]), {
    clone: structuredClone,
  });
  assert.equal(failed.state, 'failed');
  assert.equal(failed.statuses.source, 'failed');
  assert.equal(failed.statuses.observer, 'skipped');
  assert.deepEqual(failed.outputs, {});
  let cancelled = false;
  const result = await runGraph(file, 'graph', registry, {
    clone: structuredClone,
    cancelled: () => cancelled,
    yield: async () => {
      cancelled = true;
    },
  });
  assert.equal(result.state, 'cancelled');
  assert.equal(result.statuses.source, 'completed');
  assert.equal(result.statuses.observer, 'cancelled');
  assert.deepEqual(result.outputs, {});
});
test('request identity rejects superseded progress, revisions and graph changes', () => {
  const gate = new RunGate();
  const first = gate.begin(0, 'graph');
  const second = gate.begin(0, 'graph');
  assert.equal(gate.accepts(first, 0, 'graph'), false);
  assert.equal(gate.accepts(second, 0, 'graph'), true);
  assert.equal(gate.accepts(second, 1, 'graph'), false);
  assert.equal(gate.accepts(second, 0, 'other'), false);
  gate.invalidate();
  assert.equal(gate.accepts(second, 0, 'graph'), false);
});
