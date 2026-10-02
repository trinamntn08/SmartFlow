import assert from 'node:assert/strict';
import test from 'node:test';
import { readFileSync } from 'node:fs';
import { parseProject } from '@smartflow/core';
import { ExtensionRegistry } from '@smartflow/extension-sdk';
import { runGraph } from '@smartflow/runtime';
import { sceneExtension, isScene, bounds } from '@smartflow/scene-3d';
test('browser transforms agree with shared native coordinate fixtures', async () => {
  const fixture = JSON.parse(
    readFileSync(new URL('../../../tests/fixtures/scene-results-v1.json', import.meta.url), 'utf8'),
  );
  for (const expected of fixture.cases) {
    const file = parseProject(
      readFileSync(new URL('../../../examples/scene.smartflow', import.meta.url), 'utf8'),
    );
    const nodes = file.project.graphs[0]!.nodes;
    nodes[0]!.parameters.size = expected.size;
    nodes[1]!.parameters = {
      'rotation Y': expected.rotation,
      x: expected.position[0],
      y: expected.position[1],
      z: expected.position[2],
      'scale X': expected.scale[0],
      'scale Y': expected.scale[1],
      'scale Z': expected.scale[2],
    };
    const result = await runGraph(file, 'graph', new ExtensionRegistry([sceneExtension]), {
      clone: structuredClone,
    });
    assert.equal(result.state, 'completed');
    const scene = result.outputs.scene!.out;
    assert.ok(isScene(scene));
    scene.objects[0]!.vertices[0]!.forEach((value, axis) =>
      assert.ok(Math.abs(value - expected.firstVertex[axis]) < 1e-5),
    );
  }
});
test('scene example transforms geometry, applies material and owns upstream values', async () => {
  const file = parseProject(
    readFileSync(new URL('../../../examples/scene.smartflow', import.meta.url), 'utf8'),
  );
  const result = await runGraph(file, 'graph', new ExtensionRegistry([sceneExtension]), {
    clone: structuredClone,
  });
  assert.equal(result.state, 'completed');
  const source = result.outputs.cube!.out,
    final = result.outputs.scene!.out;
  assert.ok(isScene(source));
  assert.ok(isScene(final));
  assert.deepEqual(bounds(source), { minimum: [-1.5, -1.5, -1.5], maximum: [1.5, 1.5, 1.5] });
  assert.deepEqual(final.objects[0]!.color, [0.18, 0.58, 0.88]);
  const box = bounds(final),
    extent = 1.5 * (Math.cos((25 * Math.PI) / 180) + Math.sin((25 * Math.PI) / 180));
  assert.ok(Math.abs(box.maximum[0] - extent) < 1e-10);
  assert.equal(box.maximum[1], 1.5);
  assert.notDeepEqual(final.objects[0]!.vertices, source.objects[0]!.vertices);
});
test('scene merge preserves two inputs and bounds object/coordinate counts', async () => {
  const extension = new ExtensionRegistry([sceneExtension]);
  const context = { nodeId: 'box', cancelled: false, throwIfCancelled() {}, async yield() {} };
  const cube = await extension
    .extension('smartflow.scene-3d')!
    .nodes.find((node) => node.id === 'cube')!
    .execute({ size: 2 }, {}, context);
  const merge = extension
    .extension('smartflow.scene-3d')!
    .nodes.find((node) => node.id === 'merge')!;
  const merged = await merge.execute({}, { first: cube.out, second: cube.out }, context);
  assert.ok(isScene(merged.out));
  assert.equal(merged.out.objects.length, 2);
  const many = { objects: Array.from({ length: 64 }, () => merged.out.objects[0]!) };
  await assert.rejects(merge.execute({}, { first: many, second: cube.out }, context), /64 objects/);
  assert.equal(
    isScene({ objects: [{ ...merged.out.objects[0], vertices: [[100001, 0, 0]] }] }),
    false,
  );
});
