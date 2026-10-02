import {
  addNode,
  cloneJson,
  connect,
  createProject,
  disconnect,
  graphById,
  ProjectHistory,
  removeNodes,
  setParameter,
  type ConnectionDocument,
  type JsonValue,
  type NodeDocument,
  type ProjectFile,
  isComponentInstance,
} from '@smartflow/core';
import { ExtensionRegistry, validateParameter } from '@smartflow/extension-sdk';
import { writeWorkspace, readWorkspace } from './workspace.ts';
import { resolveDefinition } from '@smartflow/runtime';

export function emptyProject(id: string): ProjectFile {
  const file = createProject(id);
  file.project.graphs.push({ id: 'graph', nodes: [], connections: [] });
  return file;
}
export class EditorSession {
  readonly history: ProjectHistory;
  readonly registry: ExtensionRegistry;
  constructor(file: ProjectFile, registry: ExtensionRegistry) {
    this.history = new ProjectHistory(file);
    this.registry = registry;
  }
  definition(node: NodeDocument) {
    if (!isComponentInstance(node)) return this.registry.node(node);
    try {
      return resolveDefinition(this.history.snapshot, node, this.registry);
    } catch {
      return undefined;
    }
  }
  add(graphId: string, packageId: string, typeId: string, id: string): void {
    const extension = this.registry.extension(packageId);
    const definition = extension?.nodes.find((node) => node.id === typeId);
    if (!extension || !definition) throw new Error('Node type is unavailable');
    const file = this.history.snapshot;
    const count = graphById(file.project, graphId).nodes.length;
    const position = { x: (count % 4) * 260, y: Math.floor(count / 4) * 180 };
    this.history.edit(
      `Add ${definition.label}`,
      (project) => {
        const dependency = project.packages.find((item) => item.id === packageId);
        if (dependency && dependency.version !== extension.version)
          throw new Error('Project requires an unsupported package version');
        if (!dependency) project.packages.push({ id: packageId, version: extension.version });
        const parameters = Object.fromEntries(
          definition.parameters.map((parameter) => [
            parameter.id,
            cloneJson(parameter.defaultValue),
          ]),
        );
        addNode(project, graphId, {
          id,
          packageId,
          typeId,
          version: definition.version,
          parameters,
        });
      },
      (workspace) =>
        writeWorkspace({ ...file, workspace }, graphId, {
          selection: [id],
          positions: { ...readWorkspace(file, graphId).positions, [id]: position },
        }),
    );
    // Placement is workspace state and never enters semantic history.
  }
  parameter(graphId: string, nodeId: string, name: string, value: JsonValue): void {
    const node = graphById(this.history.snapshot.project, graphId).nodes.find(
      (item) => item.id === nodeId,
    );
    const parameter = node && this.definition(node)?.parameters.find((item) => item.id === name);
    if (!parameter) throw new Error('Parameter contract is unavailable');
    validateParameter(parameter, value);
    this.history.edit(`Set ${parameter.label}`, (project) =>
      setParameter(project, graphId, nodeId, name, value),
    );
  }
  connection(graphId: string, edge: ConnectionDocument): void {
    const graph = graphById(this.history.snapshot.project, graphId);
    const source = graph.nodes.find((node) => node.id === edge.source.nodeId);
    const target = graph.nodes.find((node) => node.id === edge.target.nodeId);
    const output =
      source && this.definition(source)?.outputs.find((port) => port.id === edge.source.portId);
    const input =
      target && this.definition(target)?.inputs.find((port) => port.id === edge.target.portId);
    if (!source || !target || !output || !input || output.typeId !== input.typeId)
      throw new Error('Ports are unavailable or have incompatible types');
    const pending = [target.id];
    const seen = new Set<string>();
    while (pending.length) {
      const id = pending.pop()!;
      if (id === source.id) throw new Error('Connection would create a cycle');
      if (seen.has(id)) continue;
      seen.add(id);
      pending.push(
        ...graph.connections
          .filter((item) => item.source.nodeId === id)
          .map((item) => item.target.nodeId),
      );
    }
    this.history.edit('Connect ports', (project) => connect(project, graphId, edge));
  }
  remove(graphId: string, nodes: readonly string[], edges: readonly string[]): void {
    this.history.edit('Delete selection', (project) => {
      for (const id of edges) disconnect(project, graphId, id);
      removeNodes(project, graphId, nodes);
    });
  }
}
