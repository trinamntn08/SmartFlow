import type { ExtensionDefinition, NodeDefinition } from '@smartflow/extension-sdk';
const typeId = 'smartflow.data.table@1';
const node = (id: string, label: string): NodeDefinition => ({
  id,
  label,
  version: 1,
  inputs: id === 'sample' ? [] : [{ id: 'in', typeId, required: true }],
  outputs: [{ id: 'out', typeId, required: true }],
  parameters:
    id === 'summary'
      ? []
      : [
          {
            id: id === 'sample' ? 'multiplier' : 'minimum',
            label: id === 'sample' ? 'Multiplier' : 'Minimum',
            control: 'number',
            defaultValue: id === 'sample' ? 1 : 20,
            minimum: 0,
            maximum: 1000,
          },
        ],
  capabilities: [],
  execute: async () => {
    throw new Error('Browser data execution is not implemented yet');
  },
});
export const dataExtension: ExtensionDefinition = {
  id: 'smartflow.data',
  version: '1',
  apiVersion: 1,
  dependencies: [],
  types: [{ id: typeId, version: 1, accepts: () => false }],
  nodes: [
    node('sample', 'Sample table'),
    node('filter', 'Filter rows'),
    node('summary', 'Summary'),
  ],
  viewers: [{ id: 'smartflow.data.table-viewer@1', typeId, adapterId: 'table' }],
  migrations: [],
};
