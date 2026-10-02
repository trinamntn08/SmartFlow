import type { JsonObject, JsonValue, NodeDocument, PackageDependency } from '@smartflow/core';
export { ExtensionRegistry, validateParameter } from './registry.ts';

export type ExecutionCapability = 'browser' | 'local' | 'remote';
export interface TypeDefinition {
  id: string;
  version: number;
  accepts(value: unknown): boolean;
}
export interface PortDefinition {
  id: string;
  typeId: string;
  required: boolean;
}
export interface ParameterDefinition {
  id: string;
  label: string;
  control: 'number' | 'text' | 'boolean' | 'select';
  defaultValue: JsonValue;
  minimum?: number;
  maximum?: number;
  options?: readonly JsonValue[];
}
export interface ExecutionContext {
  readonly nodeId: string;
  readonly cancelled: boolean;
  throwIfCancelled(): void;
  /** Yield to the host event loop and check cancellation before resuming work. */
  yield(): Promise<void>;
}
export interface NodeDefinition {
  id: string;
  version: number;
  label: string;
  inputs: readonly PortDefinition[];
  outputs: readonly PortDefinition[];
  parameters: readonly ParameterDefinition[];
  capabilities: readonly ExecutionCapability[];
  execute(
    parameters: Readonly<JsonObject>,
    inputs: Readonly<Record<string, unknown>>,
    context: ExecutionContext,
  ): Promise<Record<string, unknown>>;
}
/** UI adapters resolve this ID; no framework or DOM types enter the SDK. */
export interface ViewerDefinition {
  id: string;
  typeId: string;
  adapterId: string;
}
export interface NodeMigration {
  typeId: string;
  fromVersion: number;
  toVersion: number;
  migrate(node: Readonly<NodeDocument>): NodeDocument;
}
export interface ExtensionDefinition {
  id: string;
  version: string;
  apiVersion: 1;
  dependencies: readonly PackageDependency[];
  types: readonly TypeDefinition[];
  nodes: readonly NodeDefinition[];
  viewers: readonly ViewerDefinition[];
  migrations: readonly NodeMigration[];
}
