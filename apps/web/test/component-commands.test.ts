import assert from 'node:assert/strict';
import test from 'node:test';
import { readFileSync } from 'node:fs';
import { parseProject, parseComponent, cloneJson } from '@smartflow/core';
import { ExtensionRegistry } from '@smartflow/extension-sdk';
import { dataExtension } from '@smartflow/data';
import { EditorSession } from '../src/editor-session.ts';
import { catalog, instantiate, replaceInstance } from '../src/component-commands.ts';
const definition = () =>
  parseComponent(
    readFileSync(
      new URL('../../../examples/filtered-summary.smartflow-component', import.meta.url),
      'utf8',
    ),
  );
const editor = () =>
  new EditorSession(
    parseProject(
      readFileSync(new URL('../../../examples/data.smartflow', import.meta.url), 'utf8'),
    ),
    new ExtensionRegistry([dataExtension]),
  );
test('catalog import/noop/conflicts and insertion are atomic and undoable', () => {
  const session = editor(),
    before = session.history.snapshot,
    component = definition();
  session.history.edit('Import', (project) => catalog(project, component));
  session.history.undo();
  assert.deepEqual(session.history.snapshot, before);
  session.history.redo();
  const revision = session.history.revision;
  session.history.edit('Identical import', (project) => catalog(project, component));
  assert.equal(session.history.revision, revision);
  const conflict = cloneJson(component);
  conflict.title = 'Changed';
  assert.throws(
    () => session.history.edit('Conflict', (project) => catalog(project, conflict)),
    /conflicts/,
  );
  assert.equal(session.history.revision, revision);
  const cataloged = session.history.snapshot;
  assert.throws(() => instantiate(session, 'graph', component, 'tool', {}, {}), /Choose an input/);
  assert.deepEqual(session.history.snapshot, cataloged);
  instantiate(
    session,
    'graph',
    component,
    'tool',
    { table: { nodeId: 'sample', portId: 'out' } },
    { minimum: 40 },
  );
  assert.equal(session.history.snapshot.project.graphs[0]!.nodes.at(-1)!.parameters.minimum, 40);
  session.history.undo();
  assert.deepEqual(session.history.snapshot.project, cataloged.project);
  session.history.redo();
  session.history.edit('Remove catalog', (project) => {
    project.components = [];
  });
  assert.ok(session.definition(session.history.snapshot.project.graphs[0]!.nodes.at(-1)!));
});
test('explicit compatible replacement preserves controls and exact snapshot undo; incompatible interfaces reject', () => {
  const session = editor(),
    component = definition();
  instantiate(
    session,
    'graph',
    component,
    'tool',
    { table: { nodeId: 'sample', portId: 'out' } },
    { minimum: 40 },
  );
  const before = session.history.snapshot.project,
    copy = cloneJson(component);
  copy.id = 'copy';
  copy.title = 'Copy';
  copy.graph.nodes[0]!.parameters.minimum = 10;
  replaceInstance(session, 'graph', 'tool', copy);
  const node = session.history.snapshot.project.graphs[0]!.nodes.at(-1)!;
  assert.equal(node.parameters.minimum, 40);
  assert.deepEqual(node.component, copy);
  session.history.undo();
  assert.deepEqual(session.history.snapshot.project, before);
  const changed = cloneJson(copy);
  changed.outputs[0]!.id = 'renamed';
  const saved = session.history.snapshot;
  assert.throws(() => replaceInstance(session, 'graph', 'tool', changed), /incompatible/);
  assert.deepEqual(session.history.snapshot, saved);
  assert.equal(session.history.canRedo, true);
});
