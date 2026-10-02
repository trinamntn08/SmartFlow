import {
  cloneJson,
  graphById,
  parseProject,
  serializeProject,
  type GraphDocument,
  type JsonObject,
  type ProjectFile,
  type Endpoint,
} from '@smartflow/core';
import { resolveDefinition } from './components.ts';
export { resolveDefinition } from './components.ts';
import {
  ExtensionRegistry,
  validateParameter,
  type NodeDefinition,
} from '@smartflow/extension-sdk';
export type NodeState = 'waiting' | 'running' | 'completed' | 'failed' | 'cancelled' | 'skipped';
export interface Diagnostic {
  message: string;
  nodeId?: string;
  connectionId?: string;
}
export interface CompiledGraph {
  graph: GraphDocument;
  order: string[];
  definitions: Map<string, NodeDefinition>;
  parameters: Map<string, JsonObject>;
  diagnostics: Diagnostic[];
}
export function compileGraph(
  file: ProjectFile,
  graphId: string,
  registry: ExtensionRegistry,
  options?: Partial<RuntimeOptions>,
): CompiledGraph {
  const graph = graphById(file.project, graphId);
  const diagnostics: Diagnostic[] = [];
  const definitions = new Map<string, NodeDefinition>();
  const parameters = new Map<string, JsonObject>();
  if (!graph.nodes.length) diagnostics.push({ message: 'Graph has no nodes' });
  if (graph.nodes.length > 10000)
    diagnostics.push({ message: 'Browser preview supports at most 10000 nodes' });
  const nodes = new Map(graph.nodes.map((node) => [node.id, node]));
  for (const node of graph.nodes) {
    const packageVersion = file.project.packages.find(
      (item) => item.id === node.packageId,
    )?.version;
    let definition: NodeDefinition | undefined;
    try {
      definition = resolveDefinition(file, node, registry, options as RuntimeOptions | undefined);
    } catch (error) {
      diagnostics.push({
        nodeId: node.id,
        message: error instanceof Error ? error.message : String(error),
      });
      continue;
    }
    if (
      !packageVersion ||
      (node.packageId !== 'smartflow.components' &&
        !registry.extension(node.packageId, packageVersion)) ||
      !definition
    ) {
      diagnostics.push({
        nodeId: node.id,
        message: `Unavailable contract: ${node.packageId}/${node.typeId}@${node.version}`,
      });
      continue;
    }
    if (!definition.capabilities.includes('browser'))
      diagnostics.push({ nodeId: node.id, message: 'Node has no browser execution capability' });
    definitions.set(node.id, definition);
    const values: JsonObject = Object.fromEntries(
      definition.parameters.map((parameter) => [parameter.id, cloneJson(parameter.defaultValue)]),
    );
    for (const name of Object.keys(node.parameters))
      if (!definition.parameters.some((parameter) => parameter.id === name))
        diagnostics.push({ nodeId: node.id, message: `Unsupported retained parameter: ${name}` });
    for (const parameter of definition.parameters) {
      const value = Object.hasOwn(node.parameters, parameter.id)
        ? node.parameters[parameter.id]!
        : parameter.defaultValue;
      try {
        validateParameter(parameter, value);
        Object.defineProperty(values, parameter.id, {
          value: cloneJson(value),
          enumerable: true,
          writable: true,
          configurable: true,
        });
      } catch (error) {
        diagnostics.push({
          nodeId: node.id,
          message: error instanceof Error ? error.message : String(error),
        });
      }
    }
    parameters.set(node.id, values);
  }
  const occupied = new Set<string>();
  for (const binding of options?.externalInputs ?? []) {
    const definition = definitions.get(binding.target.nodeId);
    if (!definition?.inputs.some((port) => port.id === binding.target.portId))
      diagnostics.push({
        nodeId: binding.target.nodeId,
        message: 'Unavailable external input binding',
      });
    const key = JSON.stringify([binding.target.nodeId, binding.target.portId]);
    if (occupied.has(key)) diagnostics.push({ message: 'Input has multiple producers' });
    occupied.add(key);
  }
  const dependencies = new Map(graph.nodes.map((node) => [node.id, new Set<string>()]));
  const consumers = new Map(graph.nodes.map((node) => [node.id, new Set<string>()]));
  const endpointKey = (id: string, port: string) => JSON.stringify([id, port]);
  for (const edge of graph.connections) {
    const source = nodes.get(edge.source.nodeId);
    const target = nodes.get(edge.target.nodeId);
    if (!source || !target) {
      diagnostics.push({ connectionId: edge.id, message: 'Connection endpoint node is missing' });
      continue;
    }
    const output = definitions
      .get(source.id)
      ?.outputs.find((port) => port.id === edge.source.portId);
    const input = definitions.get(target.id)?.inputs.find((port) => port.id === edge.target.portId);
    if (!output || !input || output.typeId !== input.typeId)
      diagnostics.push({
        connectionId: edge.id,
        message: 'Connection ports are unavailable or have incompatible types',
      });
    const key = endpointKey(target.id, edge.target.portId);
    if (occupied.has(key))
      diagnostics.push({ connectionId: edge.id, message: 'Input has multiple producers' });
    occupied.add(key);
    dependencies.get(target.id)!.add(source.id);
    consumers.get(source.id)!.add(target.id);
  }
  for (const [nodeId, definition] of definitions)
    for (const port of definition.inputs)
      if (port.required && !occupied.has(endpointKey(nodeId, port.id)))
        diagnostics.push({ nodeId, message: `Required input is not connected: ${port.id}` });
  const counts = new Map([...dependencies].map(([id, inputs]) => [id, inputs.size]));
  const ready = graph.nodes.filter((node) => counts.get(node.id) === 0).map((node) => node.id);
  const order: string[] = [];
  for (let cursor = 0; cursor < ready.length; cursor++) {
    const id = ready[cursor]!;
    order.push(id);
    for (const target of consumers.get(id)!) {
      const count = counts.get(target)! - 1;
      counts.set(target, count);
      if (!count) ready.push(target);
    }
  }
  if (order.length !== graph.nodes.length)
    diagnostics.push({ message: 'Graph contains a dependency cycle' });
  return { graph, definitions, parameters, order, diagnostics };
}
export interface RuntimeOptions {
  externalInputs?: { target: Endpoint; value: unknown }[];
  clone(value: unknown): unknown;
  cancelled?(): boolean;
  yield?(): Promise<void>;
  onStatus?(nodeId: string, state: NodeState): void;
}
export interface RunResult {
  state: 'completed' | 'failed' | 'cancelled';
  graphId: string;
  outputs: Record<string, Record<string, unknown>>;
  statuses: Record<string, NodeState>;
  diagnostics: Diagnostic[];
  error?: string;
}
class Cancelled extends Error {
  constructor() {
    super('Execution cancelled');
  }
}
export async function runGraph(
  file: ProjectFile,
  graphId: string,
  registry: ExtensionRegistry,
  options: RuntimeOptions,
): Promise<RunResult> {
  const outputs: Record<string, Record<string, unknown>> = Object.create(null) as Record<
    string,
    Record<string, unknown>
  >;
  const statuses: Record<string, NodeState> = Object.create(null) as Record<string, NodeState>;
  const status = (id: string, state: NodeState) => {
    statuses[id] = state;
    options.onStatus?.(id, state);
  };
  let current: string | undefined;
  const check = () => {
    if (options.cancelled?.()) throw new Cancelled();
  };
  try {
    check();
    const snapshot = parseProject(serializeProject(file));
    const plan = compileGraph(snapshot, graphId, registry, options);
    for (const node of plan.graph.nodes) status(node.id, 'waiting');
    if (plan.diagnostics.length)
      return {
        state: 'failed',
        graphId,
        outputs: {},
        statuses,
        diagnostics: plan.diagnostics,
        error: 'Graph cannot execute; resolve its diagnostics',
      };
    for (const id of plan.order) {
      check();
      current = id;
      status(id, 'running');
      const definition = plan.definitions.get(id)!;
      const inputs: Record<string, unknown> = Object.create(null) as Record<string, unknown>;
      for (const binding of options.externalInputs ?? [])
        if (binding.target.nodeId === id) {
          const port = definition.inputs.find((port) => port.id === binding.target.portId)!;
          if (!registry.type(port.typeId)!.accepts(binding.value))
            throw new Error(`Invalid component input: ${port.id}`);
          inputs[port.id] = options.clone(binding.value);
        }
      for (const edge of plan.graph.connections.filter((edge) => edge.target.nodeId === id)) {
        const value = outputs[edge.source.nodeId]![edge.source.portId];
        const port = definition.inputs.find((port) => port.id === edge.target.portId)!;
        if (!registry.type(port.typeId)!.accepts(value))
          throw new Error(`Invalid input value for ${port.id}`);
        inputs[port.id] = options.clone(value);
      }
      const context = {
        nodeId: id,
        get cancelled() {
          return options.cancelled?.() ?? false;
        },
        throwIfCancelled: check,
        yield: async () => {
          check();
          await options.yield?.();
          check();
        },
      };
      const result = await definition.execute(cloneJson(plan.parameters.get(id)!), inputs, context);
      check();
      if (!result || typeof result !== 'object' || Array.isArray(result))
        throw new Error('Node returned invalid outputs');
      for (const port of definition.outputs) {
        if (!Object.hasOwn(result, port.id)) {
          if (port.required) throw new Error(`Required output is missing: ${port.id}`);
          continue;
        }
        if (!registry.type(port.typeId)!.accepts(result[port.id]))
          throw new Error(`Invalid output value for ${port.id}`);
      }
      if (Object.keys(result).some((id) => !definition.outputs.some((port) => port.id === id)))
        throw new Error('Node returned an undeclared output');
      outputs[id] = options.clone(result) as Record<string, unknown>;
      status(id, 'completed');
      current = undefined;
      await context.yield();
    }
    check();
    return { state: 'completed', graphId, outputs, statuses, diagnostics: [] };
  } catch (error) {
    const cancelled = error instanceof Cancelled || options.cancelled?.();
    if (current) status(current, cancelled ? 'cancelled' : 'failed');
    for (const [id, state] of Object.entries(statuses))
      if (state === 'waiting') status(id, cancelled ? 'cancelled' : 'skipped');
    return {
      state: cancelled ? 'cancelled' : 'failed',
      graphId,
      outputs: {},
      statuses,
      diagnostics: [],
      error: error instanceof Error ? error.message : String(error),
    };
  }
}
export interface RunIdentity {
  runId: number;
  revision: number;
  graphId: string;
}
export type WorkerRequest =
  { kind: 'run'; identity: RunIdentity; file: ProjectFile } | { kind: 'cancel'; runId: number };
export type WorkerReply =
  | { kind: 'status'; identity: RunIdentity; nodeId: string; state: NodeState }
  | { kind: 'result'; identity: RunIdentity; result: RunResult };
/** Independent request/revision gate; stale progress and completion never become current state. */
export class RunGate {
  #sequence = 0;
  #current: RunIdentity | undefined;
  begin(revision: number, graphId: string): RunIdentity {
    this.#current = { runId: ++this.#sequence, revision, graphId };
    return { ...this.#current };
  }
  invalidate(): void {
    this.#current = undefined;
  }
  accepts(identity: RunIdentity, revision: number, graphId: string): boolean {
    return (
      !!this.#current &&
      identity.runId === this.#current.runId &&
      identity.revision === this.#current.revision &&
      identity.graphId === this.#current.graphId &&
      identity.revision === revision &&
      identity.graphId === graphId
    );
  }
}
