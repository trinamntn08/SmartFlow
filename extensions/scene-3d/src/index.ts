import type {
  ExtensionDefinition,
  NodeDefinition,
  ParameterDefinition,
} from '@smartflow/extension-sdk';
import { cube, isScene, type SceneValue, type Vector } from './scene.ts';
export { isScene, bounds, type SceneValue, type SceneObject, type Vector } from './scene.ts';
const typeId = 'smartflow.scene-3d.scene@1';
const field = (
  id: string,
  defaultValue: number,
  minimum: number,
  maximum: number,
): ParameterDefinition => ({ id, label: id, control: 'number', defaultValue, minimum, maximum });
const parameters: Record<string, ParameterDefinition[]> = {
  cube: [field('size', 2, 0.01, 100)],
  transform: [
    field('x', 0, -100, 100),
    field('y', 0, -100, 100),
    field('z', 0, -100, 100),
    field('rotation Y', 25, -360, 360),
    field('scale X', 1, 0.01, 10),
    field('scale Y', 1, 0.01, 10),
    field('scale Z', 1, 0.01, 10),
  ],
  material: [field('red', 0.18, 0, 1), field('green', 0.58, 0, 1), field('blue', 0.88, 0, 1)],
};
const node = (id: string, label: string): NodeDefinition => ({
  id,
  label,
  version: 1,
  inputs: (id === 'cube' ? [] : id === 'merge' ? ['first', 'second'] : ['in']).map((id) => ({
    id,
    typeId,
    required: true,
  })),
  outputs: [{ id: 'out', typeId, required: true }],
  parameters: parameters[id] ?? [],
  capabilities: ['browser'],
  execute: async (values, inputs, context) => {
    for (const parameter of parameters[id] ?? []) {
      const value = values[parameter.id];
      if (
        typeof value !== 'number' ||
        !Number.isFinite(value) ||
        value < parameter.minimum! ||
        value > parameter.maximum!
      )
        throw new Error(`Invalid scene parameter: ${parameter.id}`);
    }
    context.throwIfCancelled();
    let scene: SceneValue;
    if (id === 'cube') scene = cube(context.nodeId, values.size as number);
    else {
      const input = inputs[id === 'merge' ? 'first' : 'in'];
      if (!isScene(input)) throw new Error('Expected bounded scene input');
      scene = {
        objects: input.objects.map((object) => ({
          ...object,
          color: [...object.color],
          vertices: object.vertices.map((vertex) => [...vertex]),
          triangles: object.triangles.map((face) => [...face]),
        })),
      };
      if (id === 'merge') {
        if (!isScene(inputs.second) || scene.objects.length + inputs.second.objects.length > 64)
          throw new Error('Scene preview supports at most 64 objects');
        scene.objects.push(
          ...inputs.second.objects.map((object) => ({
            ...object,
            color: [...object.color] as Vector,
            vertices: object.vertices.map((vertex) => [...vertex] as Vector),
            triangles: object.triangles.map((face) => [...face] as [number, number, number]),
          })),
        );
      }
      for (const object of scene.objects) {
        if (id === 'material') object.color = [values.red, values.green, values.blue] as Vector;
        if (id === 'transform') {
          const angle = ((values['rotation Y'] as number) * Math.PI) / 180;
          const c = Math.cos(angle);
          const s = Math.sin(angle);
          object.vertices = object.vertices.map(([x, y, z]) => {
            x *= values['scale X'] as number;
            y *= values['scale Y'] as number;
            z *= values['scale Z'] as number;
            return [
              c * x + s * z + (values.x as number),
              y + (values.y as number),
              -s * x + c * z + (values.z as number),
            ];
          });
        }
        await context.yield();
      }
    }
    if (!isScene(scene)) throw new Error('Scene preview coordinates exceed supported bounds');
    return { out: scene };
  },
});
export const sceneExtension: ExtensionDefinition = {
  id: 'smartflow.scene-3d',
  version: '1',
  apiVersion: 1,
  dependencies: [],
  types: [{ id: typeId, version: 1, accepts: isScene }],
  nodes: [
    node('cube', 'Cube'),
    node('transform', 'Transform'),
    node('material', 'Material'),
    node('scene', 'Scene'),
    node('merge', 'Merge'),
  ],
  viewers: [{ id: 'smartflow.scene-3d.viewer@1', typeId, adapterId: 'scene' }],
  migrations: [],
};
