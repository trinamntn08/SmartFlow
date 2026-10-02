import {
  assertFileSize,
  assertNumber,
  assertUnicode,
  MAXIMUM_NESTING,
  parseJson,
} from './json-transport.ts';
export { MAXIMUM_FILE_BYTES, MAXIMUM_NESTING } from './json-transport.ts';
export {
  validateComponent,
  parseComponent,
  serializeComponent,
  extractComponent,
  isComponentInstance,
  type ComponentDefinition,
} from './components.ts';
export {
  ProjectHistory,
  addNode,
  removeNodes,
  connect,
  disconnect,
  setParameter,
  graphById,
  cloneJson,
} from './commands.ts';

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
  // Match QString::fromUtf8 (one leading BOM) followed by QString::trimmed.
  if (typeof value !== 'string' || /^\uFEFF?[\p{Z}\u0009-\u000d\u0085]*$/u.test(value))
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
  const file: unknown = parseJson(text);
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
function assertJson(value: unknown, ancestors = new Set<object>(), depth = 0): void {
  if (typeof value === 'string') {
    assertUnicode(value);
    return;
  }
  if (value === null || typeof value === 'boolean') return;
  if (typeof value === 'number') {
    assertNumber(value);
    return;
  }
  if (typeof value !== 'object' || !value) throw new Error('Project contains a non-JSON value');
  if (ancestors.has(value)) throw new Error('Project contains a cycle');
  if (depth >= MAXIMUM_NESTING) throw new Error('JSON exceeds nesting limit');
  if (
    !Array.isArray(value) &&
    Object.getPrototypeOf(value) !== Object.prototype &&
    Object.getPrototypeOf(value) !== null
  )
    throw new Error('Project contains a non-JSON object');
  if (Object.getOwnPropertySymbols(value).length) throw new Error('Project contains symbol keys');
  ancestors.add(value);
  if (Array.isArray(value)) {
    for (let index = 0; index < value.length; index++)
      assertJson(value[index], ancestors, depth + 1);
  } else {
    for (const [key, item] of Object.entries(value)) {
      assertUnicode(key);
      assertJson(item, ancestors, depth + 1);
    }
  }
  ancestors.delete(value);
}
export function serializeProject(file: ProjectFile): string {
  assertJson(file);
  const text = JSON.stringify(file, null, 2);
  assertFileSize(text);
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
