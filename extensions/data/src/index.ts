import type { ExtensionDefinition, NodeDefinition } from '@smartflow/extension-sdk';
export interface TableRow {
  label: string;
  value: number;
}
export interface Table {
  rows: TableRow[];
}
export function isTable(value: unknown): value is Table {
  if (
    !value ||
    typeof value !== 'object' ||
    !('rows' in value) ||
    !Array.isArray(value.rows) ||
    value.rows.length > 1000
  )
    return false;
  return value.rows.every(
    (row: unknown) =>
      !!row &&
      typeof row === 'object' &&
      'label' in row &&
      typeof row.label === 'string' &&
      'value' in row &&
      typeof row.value === 'number' &&
      Number.isFinite(row.value),
  );
}
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
  capabilities: ['browser'],
  execute: async (parameters, inputs, context) => {
    context.throwIfCancelled();
    const rows: TableRow[] = [];
    if (id === 'sample') {
      const multiplier = parameters.multiplier;
      if (
        typeof multiplier !== 'number' ||
        !Number.isFinite(multiplier) ||
        multiplier < 0 ||
        multiplier > 1000
      )
        throw new Error('Invalid data parameter');
      const sample = [
        ['Alpha', 12],
        ['Beta', 25],
        ['Gamma', 7],
        ['Delta', 48],
        ['Epsilon', 31],
        ['Zeta', 19],
      ] as const;
      for (const [label, value] of sample) rows.push({ label, value: value * multiplier });
    } else {
      if (!isTable(inputs.in) || inputs.in.rows.some((row) => Math.abs(row.value) > 1000000))
        throw new Error('Expected bounded table input');
      const minimum = parameters.minimum;
      if (
        id === 'filter' &&
        (typeof minimum !== 'number' || !Number.isFinite(minimum) || minimum < 0 || minimum > 1000)
      )
        throw new Error('Invalid data parameter');
      let total = 0;
      for (let index = 0; index < inputs.in.rows.length; index++) {
        const row = inputs.in.rows[index]!;
        total += row.value;
        if (id === 'filter' && row.value >= (minimum as number)) rows.push({ ...row });
        if (index % 32 === 0) await context.yield();
      }
      if (id === 'summary')
        rows.push(
          { label: 'Count', value: inputs.in.rows.length },
          { label: 'Total', value: total },
          { label: 'Mean', value: inputs.in.rows.length ? total / inputs.in.rows.length : 0 },
        );
    }
    return { out: { rows } satisfies Table };
  },
});
export const dataExtension: ExtensionDefinition = {
  id: 'smartflow.data',
  version: '1',
  apiVersion: 1,
  dependencies: [],
  types: [{ id: typeId, version: 1, accepts: isTable }],
  nodes: [
    node('sample', 'Sample table'),
    node('filter', 'Filter rows'),
    node('summary', 'Summary'),
  ],
  viewers: [{ id: 'smartflow.data.table-viewer@1', typeId, adapterId: 'table' }],
  migrations: [],
};
