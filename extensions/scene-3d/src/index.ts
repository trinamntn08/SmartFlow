import type {
  ExtensionDefinition,
  NodeDefinition,
  ParameterDefinition,
} from '@smartflow/extension-sdk';
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
  capabilities: [],
  execute: async () => {
    throw new Error('Browser scene execution is not implemented yet');
  },
});
export const sceneExtension: ExtensionDefinition = {
  id: 'smartflow.scene-3d',
  version: '1',
  apiVersion: 1,
  dependencies: [],
  types: [{ id: typeId, version: 1, accepts: () => false }],
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
