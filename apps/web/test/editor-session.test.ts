import assert from 'node:assert/strict';
import test from 'node:test';
import { readFileSync } from 'node:fs';
import { parseProject, type ConnectionDocument } from '@smartflow/core';
import { ExtensionRegistry } from '@smartflow/extension-sdk';
import { dataExtension } from '@smartflow/data';
import { sceneExtension } from '@smartflow/scene-3d';
import { EditorSession, emptyProject } from '../src/editor-session.ts';
import { patchWorkspace, readWorkspace, WEB_WORKSPACE } from '../src/workspace.ts';
const registry = new ExtensionRegistry([dataExtension, sceneExtension]);
const edge = (source: string, target: string, id: string): ConnectionDocument => ({
  id,
  source: { nodeId: source, portId: 'out' },
  target: { nodeId: target, portId: 'in' },
});
test('editor uses registered defaults, rejects incompatible/cyclic/occupied inputs and preserves history', () => {
  const editor = new EditorSession(emptyProject('edit'), registry);
  editor.add('graph', 'smartflow.data', 'sample', 'sample');
  editor.add('graph', 'smartflow.data', 'filter', 'filter');
  editor.add('graph', 'smartflow.data', 'filter', 'other');
  editor.add('graph', 'smartflow.scene-3d', 'scene', 'scene');
  editor.connection('graph', edge('sample', 'filter', 'edge'));
  assert.throws(() => editor.connection('graph', edge('sample', 'scene', 'wrong')), /incompatible/);
  assert.throws(
    () => editor.connection('graph', edge('other', 'filter', 'occupied')),
    /already has/,
  );
  editor.connection('graph', edge('filter', 'other', 'next'));
  assert.throws(() => editor.connection('graph', edge('other', 'filter', 'cycle')), /cycle/);
  editor.parameter('graph', 'filter', 'minimum', 40);
  assert.throws(() => editor.parameter('graph', 'filter', 'minimum', -1));
  assert.throws(() => editor.parameter('graph', 'filter', 'future', 1));
  editor.history.undo();
  assert.equal(
    editor.history.snapshot.project.graphs[0]!.nodes.find((node) => node.id === 'filter')!
      .parameters.minimum,
    20,
  );
});
test('native positions and selection map into separate browser workspace without changing native JSON', () => {
  const file = parseProject(
    readFileSync(new URL('../../../examples/data.smartflow', import.meta.url), 'utf8'),
  );
  const editor = new EditorSession(file, registry);
  const native = file.workspace['smartflow.native-editor@1'];
  assert.deepEqual(readWorkspace(file, 'graph').positions.filter, { x: 200, y: 0 });
  patchWorkspace(editor.history, 'graph', {
    selection: ['summary'],
    viewport: { x: 30, y: 60, zoom: 1 },
  });
  editor.add('graph', 'smartflow.data', 'filter', 'new');
  assert.deepEqual(readWorkspace(editor.history.snapshot, 'graph').positions.filter, {
    x: 200,
    y: 0,
  });
  assert.deepEqual(editor.history.snapshot.workspace['smartflow.native-editor@1'], native);
  assert.ok(editor.history.snapshot.workspace[WEB_WORKSPACE]);
});
test('opaque browser namespace is preserved and rejects edits instead of replacing content', () => {
  const file = emptyProject('opaque');
  file.workspace[WEB_WORKSPACE] = ['future'];
  const editor = new EditorSession(file, registry);
  assert.throws(() => patchWorkspace(editor.history, 'graph', { selection: [] }), /Unsupported/);
  assert.deepEqual(editor.history.snapshot, file);
  assert.throws(() => editor.add('graph', 'smartflow.data', 'sample', 'new'), /Unsupported/);
  assert.deepEqual(editor.history.snapshot, file);
  assert.equal(editor.history.canUndo, false);
});
