import {
  parseProject,
  serializeProject,
  type ConnectionDocument,
  type GraphDocument,
  type JsonObject,
  type JsonValue,
  type NodeDocument,
  type ProjectDocument,
  type ProjectFile,
} from './index.ts';

/** Copy retained JSON without interpreting extension fields or normalizing numbers. */
export function cloneJson<T extends JsonValue>(value: T): T {
  if (Array.isArray(value)) return value.map((item) => cloneJson(item)) as T;
  if (value && typeof value === 'object') {
    const copy: JsonObject = {};
    for (const [key, item] of Object.entries(value))
      Object.defineProperty(copy, key, {
        value: cloneJson(item),
        enumerable: true,
        writable: true,
        configurable: true,
      });
    return copy as T;
  }
  return value;
}

export function graphById(project: ProjectDocument, id: string): GraphDocument {
  const graph = project.graphs.find((item) => item.id === id);
  if (!graph) throw new Error(`Graph ${id} is unavailable`);
  return graph;
}

interface Entry {
  label: string;
  before: ProjectDocument;
  after: ProjectDocument;
}

/** Transactional semantic history. Workspace and disposable results never enter undo entries. */
export class ProjectHistory {
  #file: ProjectFile;
  #entries: Entry[] = [];
  #cursor = 0;
  #saved: string;
  #revision = 0;
  constructor(file: ProjectFile) {
    this.#saved = serializeProject(file);
    this.#file = parseProject(this.#saved);
  }
  get snapshot(): ProjectFile {
    return cloneJson(this.#file);
  }
  get revision(): number {
    return this.#revision;
  }
  get dirty(): boolean {
    return serializeProject(this.#file) !== this.#saved;
  }
  get canUndo(): boolean {
    return this.#cursor > 0;
  }
  get canRedo(): boolean {
    return this.#cursor < this.#entries.length;
  }
  get undoLabel(): string | undefined {
    return this.#entries[this.#cursor - 1]?.label;
  }
  get redoLabel(): string | undefined {
    return this.#entries[this.#cursor]?.label;
  }
  markSaved(): void {
    this.#saved = serializeProject(this.#file);
  }
  replace(file: ProjectFile): void {
    const text = serializeProject(file);
    const replacement = parseProject(text);
    this.#file = replacement;
    this.#saved = text;
    this.#entries = [];
    this.#cursor = 0;
    this.#revision++;
  }
  edit(
    label: string,
    command: (project: ProjectDocument) => void,
    workspaceCommand?: (workspace: JsonObject) => void,
  ): boolean {
    const candidate = cloneJson(this.#file);
    command(candidate.project);
    workspaceCommand?.(candidate.workspace);
    serializeProject(candidate);
    if (JSON.stringify(candidate.project) === JSON.stringify(this.#file.project)) {
      this.#file = cloneJson(candidate);
      return false;
    }
    const entry = {
      label,
      before: cloneJson(this.#file.project),
      after: cloneJson(candidate.project),
    };
    this.#entries.splice(this.#cursor);
    this.#entries.push(entry);
    this.#cursor++;
    this.#file = cloneJson(candidate);
    this.#revision++;
    return true;
  }
  updateWorkspace(command: (workspace: JsonObject) => void): void {
    const candidate = cloneJson(this.#file);
    command(candidate.workspace);
    serializeProject(candidate);
    this.#file = cloneJson(candidate);
  }
  undo(): boolean {
    const entry = this.#entries[this.#cursor - 1];
    if (!entry) return false;
    const candidate = { ...this.#file, project: cloneJson(entry.before) };
    serializeProject(candidate);
    this.#file = candidate;
    this.#cursor--;
    this.#revision++;
    return true;
  }
  redo(): boolean {
    const entry = this.#entries[this.#cursor];
    if (!entry) return false;
    const candidate = { ...this.#file, project: cloneJson(entry.after) };
    serializeProject(candidate);
    this.#file = candidate;
    this.#cursor++;
    this.#revision++;
    return true;
  }
}

export function addNode(project: ProjectDocument, graphId: string, node: NodeDocument): void {
  const graph = graphById(project, graphId);
  if (graph.nodes.some((item) => item.id === node.id))
    throw new Error(`Node ${node.id} already exists`);
  graph.nodes.push(cloneJson(node));
}
export function removeNodes(
  project: ProjectDocument,
  graphId: string,
  ids: readonly string[],
): void {
  const graph = graphById(project, graphId);
  if (ids.some((id) => !graph.nodes.some((node) => node.id === id)))
    throw new Error('Selected node is unavailable');
  graph.nodes = graph.nodes.filter((node) => !ids.includes(node.id));
  graph.connections = graph.connections.filter(
    (edge) => !ids.includes(edge.source.nodeId) && !ids.includes(edge.target.nodeId),
  );
}
export function connect(
  project: ProjectDocument,
  graphId: string,
  connection: ConnectionDocument,
): void {
  const graph = graphById(project, graphId);
  if (
    !graph.nodes.some((node) => node.id === connection.source.nodeId) ||
    !graph.nodes.some((node) => node.id === connection.target.nodeId)
  )
    throw new Error('Connection endpoint is unavailable');
  if (graph.connections.some((edge) => edge.id === connection.id))
    throw new Error('Connection ID already exists');
  if (
    graph.connections.some(
      (edge) =>
        edge.target.nodeId === connection.target.nodeId &&
        edge.target.portId === connection.target.portId,
    )
  )
    throw new Error('Input already has a connection');
  graph.connections.push(cloneJson(connection));
}
export function disconnect(project: ProjectDocument, graphId: string, id: string): void {
  const graph = graphById(project, graphId);
  if (!graph.connections.some((edge) => edge.id === id))
    throw new Error('Connection is unavailable');
  graph.connections = graph.connections.filter((edge) => edge.id !== id);
}
export function setParameter(
  project: ProjectDocument,
  graphId: string,
  nodeId: string,
  parameter: string,
  value: JsonValue,
): void {
  const node = graphById(project, graphId).nodes.find((item) => item.id === nodeId);
  if (!node) throw new Error('Node is unavailable');
  Object.defineProperty(node.parameters, parameter, {
    value: cloneJson(value),
    enumerable: true,
    writable: true,
    configurable: true,
  });
}
