import assert from 'node:assert/strict';
import test from 'node:test';
import { readFileSync } from 'node:fs';
import {
  ProjectHistory,
  addNode,
  removeNodes,
  connect,
  disconnect,
  setParameter,
  createProject,
  parseProject,
} from '../src/index.ts';

function fixture() {
  return parseProject(
    readFileSync(new URL('../../../tests/fixtures/project-v1.json', import.meta.url), 'utf8'),
  );
}
test('deletion and incident opaque edges undo exactly; inactive graphs and envelope survive', () => {
  const original = fixture();
  const graph = original.project.graphs[0]!;
  original.project.graphs.push({
    id: 'inactive',
    nodes: [],
    connections: [],
    future: { keep: true },
  });
  const history = new ProjectHistory(original);
  history.edit('Delete', (project) => removeNodes(project, graph.id, [graph.nodes[0]!.id]));
  assert.equal(history.snapshot.project.graphs[0]!.nodes.length, graph.nodes.length - 1);
  assert.ok(history.undo());
  assert.deepEqual(history.snapshot, original);
  assert.equal(history.dirty, false);
  assert.ok(history.redo());
  assert.equal(history.dirty, true);
  assert.deepEqual(history.snapshot.project.graphs[1], original.project.graphs[1]);
});
test('add/connect/parameter/disconnect restore stable identities and undo to saved state', () => {
  const file = createProject('empty');
  file.project.graphs.push({ id: 'g', nodes: [], connections: [] });
  const history = new ProjectHistory(file);
  for (const id of ['a', 'b'])
    history.edit('Add', (project) =>
      addNode(project, 'g', {
        id,
        packageId: 'unknown',
        typeId: 'future',
        version: 1,
        parameters: {},
        opaque: true,
      }),
    );
  history.edit('Connect', (project) =>
    connect(project, 'g', {
      id: 'edge',
      source: { nodeId: 'a', portId: 'out' },
      target: { nodeId: 'b', portId: 'in' },
      opaque: { keep: true },
    }),
  );
  history.edit('Parameter', (project) =>
    setParameter(project, 'g', 'a', '__proto__', { keep: true }),
  );
  const connected = history.snapshot;
  history.edit('Disconnect', (project) => disconnect(project, 'g', 'edge'));
  history.undo();
  assert.deepEqual(history.snapshot, connected);
  while (history.undo()) {
    /* undo to initial state */
  }
  assert.deepEqual(history.snapshot, file);
  assert.equal(history.dirty, false);
  while (history.redo()) {
    /* replay all semantic changes */
  }
  assert.equal(history.snapshot.project.graphs[0]!.connections.length, 0);
});
test('workspace changes survive undo/redo without changing semantic revision', () => {
  const history = new ProjectHistory(fixture());
  const graph = history.snapshot.project.graphs[0]!;
  history.edit('Parameter', (project) =>
    setParameter(project, graph.id, graph.nodes[0]!.id, 'value', 12),
  );
  const revision = history.revision;
  history.updateWorkspace((workspace) => {
    workspace.web = { navigation: [3, 4], selection: [] };
  });
  assert.equal(history.revision, revision);
  const workspace = history.snapshot.workspace;
  history.undo();
  history.redo();
  assert.deepEqual(history.snapshot.workspace, workspace);
  assert.equal(history.dirty, true);
  history.markSaved();
  assert.equal(history.dirty, false);
});
test('failed/no-op commands retain redo; successful branch clears it; snapshots and commands cannot alias state', () => {
  const history = new ProjectHistory(fixture());
  const graph = history.snapshot.project.graphs[0]!;
  const node = graph.nodes[0]!;
  history.edit('Edit', (project) => setParameter(project, graph.id, node.id, 'value', 10));
  history.undo();
  const before = history.snapshot;
  const revision = history.revision;
  assert.throws(() =>
    history.edit('Invalid', (project) =>
      setParameter(project, graph.id, node.id, 'value', Infinity),
    ),
  );
  assert.equal(
    history.edit('No-op', () => {}),
    false,
  );
  assert.deepEqual(history.snapshot, before);
  assert.equal(history.canRedo, true);
  assert.equal(history.revision, revision);
  let leaked: typeof before.project | undefined;
  history.edit('Branch', (project) => {
    leaked = project;
    setParameter(project, graph.id, node.id, 'value', 20);
  });
  leaked!.id = 'changed-after-command';
  assert.equal(history.canRedo, false);
  assert.equal(history.snapshot.project.id, before.project.id);
  const snapshot = history.snapshot;
  snapshot.project.id = 'changed';
  assert.equal(history.snapshot.project.id, before.project.id);
});
test('transactional replacement rejects bad envelopes and resets history only on success', () => {
  const history = new ProjectHistory(fixture());
  const graph = history.snapshot.project.graphs[0]!;
  history.edit('Delete', (project) => removeNodes(project, graph.id, [graph.nodes[0]!.id]));
  const before = history.snapshot;
  const invalid = createProject('bad');
  Object.assign(invalid, { schemaVersion: 2 });
  assert.throws(() => history.replace(invalid));
  assert.deepEqual(history.snapshot, before);
  assert.equal(history.canUndo, true);
  history.replace(createProject('new'));
  assert.equal(history.canUndo, false);
  assert.equal(history.dirty, false);
});
