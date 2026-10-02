import type { JsonValue, NodeDocument } from '@smartflow/core';
import type {
  ExtensionDefinition,
  NodeDefinition,
  ParameterDefinition,
  TypeDefinition,
} from './index.ts';

export function validateParameter(parameter: ParameterDefinition, value: JsonValue): void {
  if (parameter.control === 'number') {
    if (
      typeof value !== 'number' ||
      !Number.isFinite(value) ||
      (parameter.minimum !== undefined && value < parameter.minimum) ||
      (parameter.maximum !== undefined && value > parameter.maximum)
    )
      throw new Error(`${parameter.label}: expected a finite number in the allowed range`);
  } else if (parameter.control === 'boolean' && typeof value !== 'boolean')
    throw new Error(`${parameter.label}: expected boolean`);
  else if (parameter.control === 'text' && typeof value !== 'string')
    throw new Error(`${parameter.label}: expected text`);
  else if (
    parameter.control === 'select' &&
    !parameter.options?.some((option) => JSON.stringify(option) === JSON.stringify(value))
  )
    throw new Error(`${parameter.label}: unsupported choice`);
}

function unique(values: readonly { id: string }[], label: string): void {
  const ids = new Set<string>();
  for (const value of values) {
    if (!value.id.trim() || ids.has(value.id)) throw new Error(`${label}: empty or duplicate ID`);
    ids.add(value.id);
  }
}

function immutableCopy<T>(value: T): T {
  if (Array.isArray(value)) return Object.freeze(value.map((item) => immutableCopy(item))) as T;
  if (value && typeof value === 'object')
    return Object.freeze(
      Object.fromEntries(Object.entries(value).map(([key, item]) => [key, immutableCopy(item)])),
    ) as T;
  return value;
}

/** Explicit composition of bundled source-level extensions. No dynamic code loading. */
export class ExtensionRegistry {
  #extensions = new Map<string, ExtensionDefinition>();
  constructor(extensions: readonly ExtensionDefinition[]) {
    extensions = immutableCopy(extensions);
    unique(extensions, 'extensions');
    for (const extension of extensions) {
      if (extension.apiVersion !== 1 || !extension.version.trim())
        throw new Error('Unsupported extension API/version');
      unique(extension.nodes, 'nodes');
      unique(extension.types, 'types');
      unique(extension.viewers, 'viewers');
      for (const node of extension.nodes) {
        if (!Number.isSafeInteger(node.version) || node.version < 1)
          throw new Error('Invalid node contract version');
        unique(node.inputs, 'inputs');
        unique(node.outputs, 'outputs');
        unique(node.parameters, 'parameters');
        for (const parameter of node.parameters)
          validateParameter(parameter, parameter.defaultValue);
      }
      this.#extensions.set(extension.id, extension);
    }
    const typeIds = new Set<string>();
    for (const extension of extensions) {
      for (const dependency of extension.dependencies)
        if (this.#extensions.get(dependency.id)?.version !== dependency.version)
          throw new Error(`Missing extension dependency: ${dependency.id}@${dependency.version}`);
      for (const type of extension.types) {
        if (typeIds.has(type.id)) throw new Error(`Duplicate type contract: ${type.id}`);
        typeIds.add(type.id);
      }
    }
    for (const extension of extensions)
      for (const node of extension.nodes)
        for (const port of [...node.inputs, ...node.outputs])
          if (!typeIds.has(port.typeId)) throw new Error(`Unknown port type: ${port.typeId}`);
  }
  get extensions(): readonly ExtensionDefinition[] {
    return [...this.#extensions.values()];
  }
  extension(id: string, version?: string): ExtensionDefinition | undefined {
    const extension = this.#extensions.get(id);
    return extension && (version === undefined || extension.version === version)
      ? extension
      : undefined;
  }
  node(
    document: Pick<NodeDocument, 'packageId' | 'typeId' | 'version'>,
  ): NodeDefinition | undefined {
    return this.#extensions
      .get(document.packageId)
      ?.nodes.find((node) => node.id === document.typeId && node.version === document.version);
  }
  type(id: string): TypeDefinition | undefined {
    for (const extension of this.#extensions.values()) {
      const type = extension.types.find((item) => item.id === id);
      if (type) return type;
    }
    return undefined;
  }
  compatible(source: NodeDocument, output: string, target: NodeDocument, input: string): boolean {
    const sourcePort = this.node(source)?.outputs.find((port) => port.id === output);
    const targetPort = this.node(target)?.inputs.find((port) => port.id === input);
    return !!sourcePort && !!targetPort && sourcePort.typeId === targetPort.typeId;
  }
}
