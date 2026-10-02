import assert from 'node:assert/strict';
import test from 'node:test';
import { readFileSync } from 'node:fs';
import { parseProject, validateComponent } from '@smartflow/core';
import { ExtensionRegistry } from '@smartflow/extension-sdk';
import { dataExtension } from '@smartflow/data';
import { runGraph } from '@smartflow/runtime';
import { EditorSession } from '../src/editor-session.ts';
const registry = new ExtensionRegistry([dataExtension]);
const file = () =>
  parseProject(
    readFileSync(new URL('../../../examples/components.smartflow', import.meta.url), 'utf8'),
  );
test('collapsed snapshot executes named outputs/controls without catalog and remains isolated', async () => {
  const project = file();
  delete project.project.components;
  const instance = project.project.graphs[0]!.nodes[1]!;
  instance.parameters.minimum = 40;
  const snapshot = structuredClone(instance.component);
  const result = await runGraph(project, 'graph', registry, { clone: structuredClone });
  assert.equal(result.state, 'completed');
  const out = result.outputs['summary-tool']!.summary as { rows: { value: number }[] };
  assert.equal(out.rows[1]!.value, 48);
  assert.deepEqual(instance.component, snapshot);
  const editor = new EditorSession(project, registry);
  assert.equal(editor.definition(instance)?.label, 'Filtered summary');
  editor.parameter('graph', 'summary-tool', 'minimum', 20);
  editor.history.undo();
  assert.deepEqual(editor.history.snapshot, project);
});
test('bad body parameters and unsupported snapshots reject the whole graph and preserve retained data', async () => {
  for (const change of ['nested', 'version', 'parameter']) {
    const project = file(),
      instance = project.project.graphs[0]!.nodes[1]!,
      definition = validateComponent(instance.component);
    if (change === 'nested') {
      definition.graph.nodes[0]!.packageId = 'smartflow.components';
      definition.graph.nodes[0]!.typeId = 'instance';
    }
    if (change === 'version') definition.graph.nodes[0]!.version = 2;
    if (change === 'parameter') instance.parameters.minimum = -1;
    instance.component = definition;
    const previous = structuredClone(project);
    const result = await runGraph(project, 'graph', registry, { clone: structuredClone });
    assert.equal(result.state, 'failed');
    assert.deepEqual(result.outputs, {});
    assert.deepEqual(project, previous);
  }
});
