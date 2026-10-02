import {
  cloneJson,
  graphById,
  validateComponent,
  isComponentInstance,
  type ComponentDefinition,
  type Endpoint,
  type JsonValue,
  type ProjectDocument,
  type ProjectFile,
} from '@smartflow/core';
import { validateParameter, type NodeDefinition } from '@smartflow/extension-sdk';
import { resolveDefinition } from '@smartflow/runtime';
import { EditorSession } from './editor-session.ts';
import { readWorkspace, writeWorkspace } from './workspace.ts';
const canonical = (value: JsonValue): string =>
  JSON.stringify(
    value && typeof value === 'object'
      ? Array.isArray(value)
        ? value.map((item) => JSON.parse(canonical(item)) as JsonValue)
        : Object.fromEntries(
            Object.entries(value)
              .sort(([a], [b]) => (a < b ? -1 : a > b ? 1 : 0))
              .map(([key, item]) => [key, JSON.parse(canonical(item)) as JsonValue]),
          )
      : value,
  );
export function catalog(project: ProjectDocument, definition: ComponentDefinition): void {
  definition = validateComponent(definition);
  if (project.components === undefined) project.components = [];
  if (!Array.isArray(project.components)) throw new Error('Unsupported component catalog shape');
  const matches = project.components.filter(
    (item) =>
      item &&
      typeof item === 'object' &&
      !Array.isArray(item) &&
      item.id === definition.id &&
      item.version === definition.version,
  );
  if (matches.some((item) => canonical(item) !== canonical(definition)))
    throw new Error('Component identity/version conflicts with saved content');
  if (!matches.length) project.components.push(cloneJson(definition));
}
function packages(file: ProjectFile, definition: ComponentDefinition, editor: EditorSession): void {
  for (const node of definition.graph.nodes) {
    const extension = editor.registry.extension(node.packageId);
    if (!extension || !editor.registry.node(node))
      throw new Error('Component body contract is unavailable');
    const dependency = file.project.packages.find((item) => item.id === extension.id);
    if (dependency && dependency.version !== extension.version)
      throw new Error('Unsupported component body package version');
    if (!dependency) file.project.packages.push({ id: extension.id, version: extension.version });
  }
  const dependency = file.project.packages.find((item) => item.id === 'smartflow.components');
  if (dependency && dependency.version !== '1')
    throw new Error('Unsupported component package version');
  if (!dependency) file.project.packages.push({ id: 'smartflow.components', version: '1' });
}
export function componentContract(
  editor: EditorSession,
  definition: ComponentDefinition,
): NodeDefinition {
  const file = editor.history.snapshot;
  packages(file, definition, editor);
  const resolved = resolveDefinition(
    file,
    {
      id: 'preview',
      packageId: 'smartflow.components',
      typeId: 'instance',
      version: 1,
      parameters: {},
      component: definition,
    },
    editor.registry,
  );
  if (!resolved) throw new Error('Component is unavailable');
  return resolved;
}
export function instantiate(
  editor: EditorSession,
  graphId: string,
  definition: ComponentDefinition,
  id: string,
  sources: Record<string, Endpoint>,
  controls: Record<string, JsonValue>,
): void {
  definition = validateComponent(definition);
  const original = editor.history.snapshot;
  editor.history.edit(
    'Insert component',
    (project) => {
      const file = { ...original, project };
      packages(file, definition, editor);
      catalog(project, definition);
      const contract = componentContract(editor, definition),
        parameters = Object.fromEntries(
          contract.parameters.map((parameter) => [
            parameter.id,
            cloneJson(
              Object.hasOwn(controls, parameter.id)
                ? controls[parameter.id]!
                : parameter.defaultValue,
            ),
          ]),
        );
      for (const parameter of contract.parameters)
        validateParameter(parameter, parameters[parameter.id]!);
      if (
        Object.keys(controls).some(
          (id) => !contract.parameters.some((parameter) => parameter.id === id),
        )
      )
        throw new Error('Unknown component control');
      const graph = graphById(project, graphId);
      if (graph.nodes.some((node) => node.id === id))
        throw new Error('Component instance ID collision');
      for (const input of contract.inputs) {
        const source = sources[input.id];
        if (!source) throw new Error(`Choose an input source for ${input.id}`);
        const node = graph.nodes.find((node) => node.id === source.nodeId),
          port =
            node &&
            resolveDefinition(file, node, editor.registry)?.outputs.find(
              (port) => port.id === source.portId,
            );
        if (!port || port.typeId !== input.typeId)
          throw new Error('Component input has incompatible type');
        const edgeId = `component/input/${new TextEncoder().encode(id).length}/${id}/${input.id}`;
        if (graph.connections.some((edge) => edge.id === edgeId))
          throw new Error('Component edge ID collision');
        graph.connections.push({
          id: edgeId,
          source: cloneJson(source),
          target: { nodeId: id, portId: input.id },
        });
      }
      graph.nodes.push({
        id,
        packageId: 'smartflow.components',
        typeId: 'instance',
        version: 1,
        parameters,
        component: cloneJson(definition),
      });
    },
    (workspace) => {
      const count = graphById(original.project, graphId).nodes.length;
      writeWorkspace({ ...original, workspace }, graphId, {
        selection: [id],
        positions: {
          ...readWorkspace(original, graphId).positions,
          [id]: { x: (count % 4) * 260, y: Math.floor(count / 4) * 180 },
        },
      });
    },
  );
}
export function replaceInstance(
  editor: EditorSession,
  graphId: string,
  nodeId: string,
  replacement: ComponentDefinition,
): void {
  replacement = validateComponent(replacement);
  const original = editor.history.snapshot;
  editor.history.edit('Update component snapshot', (project) => {
    const file = { ...original, project },
      node = graphById(project, graphId).nodes.find((node) => node.id === nodeId);
    if (!node || !isComponentInstance(node))
      throw new Error('Choose one collapsed component instance');
    const previous = validateComponent(node.component);
    if (previous.id === replacement.id && canonical(previous) !== canonical(replacement))
      throw new Error('Changed definitions require a fresh ID');
    packages(file, replacement, editor);
    const before = resolveDefinition(file, node, editor.registry)!,
      after = resolveDefinition(file, { ...node, component: replacement }, editor.registry)!;
    const signature = (contract: NodeDefinition) =>
      JSON.stringify({
        inputs: contract.inputs.map((port) => [port.id, port.typeId, port.required]).sort(),
        outputs: contract.outputs.map((port) => [port.id, port.typeId, port.required]).sort(),
        controls: contract.parameters
          .map((parameter) => [
            parameter.id,
            parameter.control,
            parameter.minimum ?? null,
            parameter.maximum ?? null,
            parameter.options ?? null,
          ])
          .sort(),
      });
    if (signature(before) !== signature(after))
      throw new Error('Replacement public interface is incompatible');
    for (const parameter of after.parameters)
      validateParameter(
        parameter,
        Object.hasOwn(node.parameters, parameter.id)
          ? node.parameters[parameter.id]!
          : parameter.defaultValue,
      );
    if (
      Object.keys(node.parameters).some(
        (id) => !after.parameters.some((parameter) => parameter.id === id),
      )
    )
      throw new Error('Unknown retained component control');
    const graph = graphById(project, graphId),
      occupied = new Set<string>();
    for (const edge of graph.connections.filter(
      (edge) => edge.source.nodeId === nodeId || edge.target.nodeId === nodeId,
    )) {
      const source = graph.nodes.find((item) => item.id === edge.source.nodeId),
        target = graph.nodes.find((item) => item.id === edge.target.nodeId);
      const sourceContract =
          source &&
          (source.id === nodeId ? after : resolveDefinition(file, source, editor.registry)),
        targetContract =
          target &&
          (target.id === nodeId ? after : resolveDefinition(file, target, editor.registry));
      const output = sourceContract?.outputs.find((port) => port.id === edge.source.portId),
        input = targetContract?.inputs.find((port) => port.id === edge.target.portId);
      if (!output || !input || output.typeId !== input.typeId)
        throw new Error('Connected component ports are incompatible');
      if (edge.target.nodeId === nodeId) {
        if (occupied.has(input.id)) throw new Error('Component input has multiple producers');
        occupied.add(input.id);
      }
    }
    if (after.inputs.some((port) => port.required && !occupied.has(port.id)))
      throw new Error('Component required input is not connected');
    catalog(project, replacement);
    node.component = cloneJson(replacement);
  });
}
