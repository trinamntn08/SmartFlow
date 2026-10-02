import {
  cloneJson,
  createProject,
  graphById,
  parseProject,
  serializeProject,
  type Endpoint,
  type GraphDocument,
  type JsonObject,
  type JsonValue,
  type NodeDocument,
  type ProjectFile,
} from './index.ts';
import { parseJson } from './json-transport.ts';
export interface ComponentInput extends JsonObject {
  id: string;
  target: Endpoint;
}
export interface ComponentOutput extends JsonObject {
  id: string;
  source: Endpoint;
}
export interface ComponentControl extends JsonObject {
  id: string;
  target: JsonObject & { nodeId: string; parameter: string };
}
export interface ComponentDefinition extends JsonObject {
  format: 'smartflow.graph-component';
  schemaVersion: 1;
  version: 1;
  id: string;
  title: string;
  graph: GraphDocument;
  inputs: ComponentInput[];
  outputs: ComponentOutput[];
  controls: ComponentControl[];
}
export const isComponentInstance = (node: NodeDocument) =>
  node.packageId === 'smartflow.components' && node.typeId === 'instance';
const name = (value: JsonValue | undefined): string => {
  if (typeof value !== 'string' || /^\uFEFF?[\p{Z}\u0009-\u000d\u0085]*$/u.test(value))
    throw new Error('Component identity must be nonempty');
  return value;
};
const object = (value: JsonValue | undefined): JsonObject => {
  if (!value || typeof value !== 'object' || Array.isArray(value))
    throw new Error('Component field must be an object');
  return value;
};
/** Explicit validation; project loading preserves unsupported catalogs and snapshots. */
export function validateComponent(value: JsonValue | undefined): ComponentDefinition {
  const definition = object(value);
  if (
    definition.format !== 'smartflow.graph-component' ||
    definition.schemaVersion !== 1 ||
    definition.version !== 1
  )
    throw new Error('Unsupported component format or version');
  name(definition.id);
  name(definition.title);
  const envelope = createProject('component-validation');
  envelope.project.graphs = [definition.graph as GraphDocument];
  envelope.component = definition;
  const graph = parseProject(serializeProject(envelope)).project.graphs[0]!;
  if (!graph.nodes.length) throw new Error('Component graph must contain nodes');
  if (graph.nodes.some(isComponentInstance)) throw new Error('Nested components are unsupported');
  const nodes = new Map(graph.nodes.map((node) => [node.id, node]));
  const endpoint = (value: JsonValue | undefined, port: string) => {
    const item = object(value),
      id = name(item.nodeId),
      key = name(item[port]);
    if (!nodes.has(id)) throw new Error('Component endpoint node is missing');
    return JSON.stringify([id, key]);
  };
  const occupied = new Set<string>(),
    dependencies = new Map(graph.nodes.map((node) => [node.id, new Set<string>()]));
  for (const edge of graph.connections) {
    endpoint(edge.source, 'portId');
    const target = endpoint(edge.target, 'portId');
    if (occupied.has(target)) throw new Error('Component input has multiple producers');
    occupied.add(target);
    dependencies.get(edge.target.nodeId)!.add(edge.source.nodeId);
  }
  const ready = graph.nodes
    .filter((node) => !dependencies.get(node.id)!.size)
    .map((node) => node.id);
  for (let cursor = 0; cursor < ready.length; cursor++)
    for (const [id, inputs] of dependencies)
      if (inputs.delete(ready[cursor]!) && !inputs.size) ready.push(id);
  if (ready.length !== graph.nodes.length) throw new Error('Component graph contains a cycle');
  for (const section of ['inputs', 'outputs', 'controls']) {
    const entries = definition[section];
    if (!Array.isArray(entries)) throw new Error('Component interface must be an array');
    const identities = new Set<string>(),
      targets = new Set<string>();
    for (const value of entries) {
      const entry = object(value),
        id = name(entry.id);
      if (identities.has(id)) throw new Error('Duplicate component interface ID');
      identities.add(id);
      const output = section === 'outputs',
        control = section === 'controls',
        binding = object(entry[output ? 'source' : 'target']);
      const target = endpoint(binding, control ? 'parameter' : 'portId');
      if (!output && targets.has(target)) throw new Error('Duplicate exposed component target');
      targets.add(target);
      if (section === 'inputs' && occupied.has(target))
        throw new Error('Exposed input already has an internal producer');
      if (
        control &&
        !Object.hasOwn(nodes.get(name(binding.nodeId))!.parameters, name(binding.parameter))
      )
        throw new Error('Exposed component parameter is missing');
    }
  }
  return cloneJson(definition) as ComponentDefinition;
}
export function parseComponent(text: string): ComponentDefinition {
  return validateComponent(parseJson(text) as JsonValue);
}
export function serializeComponent(definition: ComponentDefinition): string {
  validateComponent(definition);
  const file = createProject('component-transport');
  file.component = definition;
  serializeProject(file);
  return JSON.stringify(definition, null, 2) + '\n';
}
export function extractComponent(
  file: ProjectFile,
  graphId: string,
  ids: string[],
  definition: Pick<
    ComponentDefinition,
    'format' | 'schemaVersion' | 'version' | 'id' | 'title' | 'inputs' | 'outputs' | 'controls'
  >,
): ComponentDefinition {
  const source = graphById(file.project, graphId),
    selected = new Set(ids);
  if (!ids.length || selected.size !== ids.length)
    throw new Error('Component selection must be nonempty and unique');
  const graph = cloneJson(source);
  graph.id = 'body';
  graph.nodes = graph.nodes.filter((node) => selected.has(node.id));
  graph.connections = graph.connections.filter(
    (edge) => selected.has(edge.source.nodeId) && selected.has(edge.target.nodeId),
  );
  if (graph.nodes.length !== ids.length)
    throw new Error('Component selection contains missing nodes');
  const result = validateComponent({ ...definition, graph });
  for (const edge of source.connections) {
    const from = selected.has(edge.source.nodeId),
      to = selected.has(edge.target.nodeId);
    if (from === to) continue;
    const endpoint = to ? edge.target : edge.source;
    const exposed = to
      ? result.inputs.map((input) => input.target)
      : result.outputs.map((output) => output.source);
    if (!exposed.some((item) => item.nodeId === endpoint.nodeId && item.portId === endpoint.portId))
      throw new Error('Component boundary requires an exposed endpoint');
  }
  return result;
}
