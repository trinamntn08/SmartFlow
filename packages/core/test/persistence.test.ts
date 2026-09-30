import assert from 'node:assert/strict';
import test from 'node:test';
import { readFileSync } from 'node:fs';
import { createProject, parseProject, serializeProject } from '../src/index.ts';

test('shared native/browser schema fixture preserves unknown extension content', () => {
  const text = readFileSync(
    new URL('../../../tests/fixtures/project-v1.json', import.meta.url),
    'utf8',
  );
  const document = parseProject(text);
  assert.deepEqual(parseProject(serializeProject(document)), JSON.parse(text));
  assert.equal(document.project.graphs[0]!.connections[0]!.source.nodeId, 'missing-node');
});

function fixture() {
  const file = createProject('project-1');
  file.project.packages.push({ id: 'unavailable', version: '9.2.0' });
  file.project.assets.push({ id: 'asset-1', uri: 'assets/example.bin' });
  file.project.graphs.push({
    id: 'graph-1',
    nodes: [
      {
        id: 'node-1',
        packageId: 'unavailable',
        typeId: 'custom',
        version: 4,
        parameters: { value: 3, nested: [null, true, 'é'] },
        opaque: { future: 'keep' },
      },
    ],
    connections: [
      {
        id: 'connection-1',
        source: { nodeId: 'missing-node', portId: 'output' },
        target: { nodeId: 'node-1', portId: 'input' },
        routingHint: 'keep',
      },
    ],
  });
  file.workspace = {
    positions: { 'node-1': { x: 10, y: 20 } },
    viewers: [{ id: 'viewer-1', camera: [1, 2, 3] }],
  };
  file.futureField = { preserved: true };
  return file;
}

test('unknown extension payloads, dangling connections, assets and workspace survive reopening', () => {
  const file = fixture();
  assert.deepEqual(parseProject(serializeProject(file)), file);
});
test('empty domain-independent project reopens without extensions or a scene', () => {
  const file = createProject('empty');
  assert.deepEqual(parseProject(serializeProject(file)), file);
});
test('loading neither aliases the input nor repairs graph content', () => {
  const file = fixture();
  const reopened = parseProject(serializeProject(file));
  reopened.project.graphs[0]!.nodes[0]!.parameters.value = 99;
  assert.equal(file.project.graphs[0]!.nodes[0]!.parameters.value, 3);
  assert.equal(reopened.project.graphs[0]!.connections.length, 1);
});
test('reject malformed JSON, unsupported versions and incomplete envelopes', () => {
  for (const text of [
    '{',
    'null',
    '[]',
    '{}',
    JSON.stringify({ ...fixture(), schemaVersion: 2 }),
    JSON.stringify({ ...fixture(), workspace: [] }),
  ]) {
    assert.throws(() => parseProject(text));
  }
});
test('reject duplicate identities and malformed known node/connection fields', () => {
  const mutations = [
    (file: ReturnType<typeof fixture>) => {
      file.project.graphs.push(file.project.graphs[0]!);
    },
    (file: ReturnType<typeof fixture>) => {
      file.project.graphs[0]!.nodes.push(file.project.graphs[0]!.nodes[0]!);
    },
    (file: ReturnType<typeof fixture>) => {
      file.project.graphs[0]!.nodes[0]!.version = 0;
    },
    (file: ReturnType<typeof fixture>) => {
      file.project.graphs[0]!.connections[0]!.source.portId = '';
    },
    (file: ReturnType<typeof fixture>) => {
      file.project.packages[0]!.version = '';
    },
  ];
  for (const mutate of mutations) {
    const file = fixture();
    mutate(file);
    assert.throws(() => parseProject(JSON.stringify(file)));
  }
});
test('serialization rejects lossy values and cycles instead of corrupting data', () => {
  for (const invalid of [undefined, NaN, Infinity, 1n, () => 1, new Date(), new Map(), [, ,]]) {
    const file = fixture();
    Object.assign(file.workspace, { invalid });
    assert.throws(() => serializeProject(file));
  }
  const file = fixture();
  file.workspace.cycle = file.workspace;
  assert.throws(() => serializeProject(file), /cycle/);
});
test('reserved-looking JSON keys survive without prototype mutation', () => {
  const file = fixture();
  file.workspace = JSON.parse('{"__proto__":{"polluted":true},"constructor":"opaque"}');
  assert.deepEqual(parseProject(serializeProject(file)).workspace, file.workspace);
  assert.equal(Object.hasOwn(Object.prototype, 'polluted'), false);
});
test('reject numeric overflow on load', () => {
  assert.throws(
    () => parseProject(serializeProject(fixture()).replace('"value": 3', '"value": 1e400')),
    /non-JSON/,
  );
});
