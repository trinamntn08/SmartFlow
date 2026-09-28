export type JsonValue = null | boolean | number | string | JsonValue[] | JsonObject;
export interface JsonObject {
  [key: string]: JsonValue;
}
export interface NodeDocument extends JsonObject {
  id: string;
  packageId: string;
  typeId: string;
  version: number;
  parameters: JsonObject;
}
export interface Endpoint extends JsonObject {
  nodeId: string;
  portId: string;
}
export interface ConnectionDocument extends JsonObject {
  id: string;
  source: Endpoint;
  target: Endpoint;
}
export interface GraphDocument extends JsonObject {
  id: string;
  nodes: NodeDocument[];
  connections: ConnectionDocument[];
}
export interface PackageDependency extends JsonObject {
  id: string;
  version: string;
}
export interface AssetReference extends JsonObject {
  id: string;
  uri: string;
}
export interface ProjectDocument extends JsonObject {
  id: string;
  graphs: GraphDocument[];
  packages: PackageDependency[];
  assets: AssetReference[];
}
export interface ProjectFile extends JsonObject {
  format: 'smartflow';
  schemaVersion: 1;
  project: ProjectDocument;
  workspace: JsonObject;
}

function object(value: unknown, path: string): asserts value is Record<string, unknown> {
  if (!value || typeof value !== 'object' || Array.isArray(value))
    throw new Error(`${path}: expected object`);
}
function name(value: unknown, path: string): asserts value is string {
  if (typeof value !== 'string' || !value.trim())
    throw new Error(`${path}: expected nonempty string`);
}
function list(value: unknown, path: string): asserts value is unknown[] {
  if (!Array.isArray(value)) throw new Error(`${path}: expected array`);
}
function identified(value: unknown, path: string): asserts value is Record<string, unknown>[] {
  list(value, path);
  const ids = new Set<string>();
  for (const item of value) {
    object(item, path);
    name(item.id, `${path}.id`);
    if (ids.has(item.id)) throw new Error(`${path}: duplicate id ${item.id}`);
    ids.add(item.id);
  }
}

/** Checks structure only. Missing extensions and dangling connections remain intact. */
export function parseProject(text: string): ProjectFile {
  const file: unknown = JSON.parse(text);
  assertJson(file);
  object(file, 'file');
  if (file.format !== 'smartflow' || file.schemaVersion !== 1)
    throw new Error('Unsupported project format or schema version');
  object(file.project, 'project');
  object(file.workspace, 'workspace');
  const project = file.project;
  name(project.id, 'project.id');
  identified(project.packages, 'packages');
  for (const dependency of project.packages) name(dependency.version, 'package.version');
  identified(project.assets, 'assets');
  for (const asset of project.assets) name(asset.uri, 'asset.uri');
  identified(project.graphs, 'graphs');
  for (const graph of project.graphs) {
    identified(graph.nodes, 'nodes');
    identified(graph.connections, 'connections');
    for (const node of graph.nodes) {
      name(node.packageId, 'node.packageId');
      name(node.typeId, 'node.typeId');
      if (!Number.isSafeInteger(node.version) || (node.version as number) < 1)
        throw new Error('node.version: expected positive integer');
      object(node.parameters, 'node.parameters');
    }
    for (const connection of graph.connections) {
      for (const end of ['source', 'target']) {
        const endpoint = connection[end];
        object(endpoint, `connection.${end}`);
        name(endpoint.nodeId, 'endpoint.nodeId');
        name(endpoint.portId, 'endpoint.portId');
      }
    }
  }
  return file as ProjectFile;
}

/** Refuse values JSON would silently lose, including cycles and nonfinite numbers. */
function assertJson(value: unknown, ancestors = new Set<object>()): void {
  if (value === null || typeof value === 'string' || typeof value === 'boolean') return;
  if (typeof value === 'number' && Number.isFinite(value)) return;
  if (typeof value !== 'object' || !value) throw new Error('Project contains a non-JSON value');
  if (ancestors.has(value)) throw new Error('Project contains a cycle');
  if (
    !Array.isArray(value) &&
    Object.getPrototypeOf(value) !== Object.prototype &&
    Object.getPrototypeOf(value) !== null
  )
    throw new Error('Project contains a non-JSON object');
  if (Object.getOwnPropertySymbols(value).length) throw new Error('Project contains symbol keys');
  ancestors.add(value);
  if (Array.isArray(value)) {
    for (let index = 0; index < value.length; index++) assertJson(value[index], ancestors);
  } else {
    for (const item of Object.values(value)) assertJson(item, ancestors);
  }
  ancestors.delete(value);
}
export function serializeProject(file: ProjectFile): string {
  assertJson(file);
  const text = JSON.stringify(file, null, 2);
  parseProject(text);
  return text;
}
export function createProject(id: string): ProjectFile {
  name(id, 'project.id');
  return {
    format: 'smartflow',
    schemaVersion: 1,
    project: { id, graphs: [], packages: [], assets: [] },
    workspace: {},
  };
}
