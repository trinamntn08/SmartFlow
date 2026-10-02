import {
  cloneJson,
  isComponentInstance,
  validateComponent,
  type NodeDocument,
  type ProjectFile,
} from '@smartflow/core';
import { type ExtensionRegistry, type NodeDefinition } from '@smartflow/extension-sdk';
import { compileGraph, runGraph, type RuntimeOptions } from './index.ts';
/** Resolve a retained platform component without importing a domain implementation. */
export function resolveDefinition(
  file: ProjectFile,
  node: NodeDocument,
  registry: ExtensionRegistry,
  options?: RuntimeOptions,
): NodeDefinition | undefined {
  if (!isComponentInstance(node)) return registry.node(node);
  if (
    node.version !== 1 ||
    !file.project.packages.some(
      (item) => item.id === 'smartflow.components' && item.version === '1',
    )
  )
    throw new Error('Unavailable component instance version');
  const component = validateComponent(node.component);
  const bodyFile = cloneJson(file);
  bodyFile.project.graphs = [cloneJson(component.graph)];
  const bodyNodes = new Map(component.graph.nodes.map((item) => [item.id, item]));
  const bodyDefinition = (id: string) => {
    const item = bodyNodes.get(id)!;
    const version = file.project.packages.find(
      (dependency) => dependency.id === item.packageId,
    )?.version;
    if (!version || !registry.extension(item.packageId, version))
      throw new Error('Unavailable component body package');
    const definition = registry.node(item);
    if (!definition) throw new Error('Unavailable component body node');
    return definition;
  };
  const inputs = component.inputs.map((input) => {
    const port = bodyDefinition(input.target.nodeId).inputs.find(
      (port) => port.id === input.target.portId,
    );
    if (!port) throw new Error('Unavailable component input');
    return { ...port, id: input.id, required: true };
  });
  const outputs = component.outputs.map((output) => {
    const port = bodyDefinition(output.source.nodeId).outputs.find(
      (port) => port.id === output.source.portId,
    );
    if (!port) throw new Error('Unavailable component output');
    return { ...port, id: output.id };
  });
  const parameters = component.controls.map((control) => {
    const parameter = bodyDefinition(control.target.nodeId).parameters.find(
      (parameter) => parameter.id === control.target.parameter,
    );
    if (!parameter) throw new Error('Unavailable component control');
    return {
      ...parameter,
      id: control.id,
      label: control.id,
      defaultValue: cloneJson(
        bodyNodes.get(control.target.nodeId)!.parameters[control.target.parameter]!,
      ),
    };
  });
  const plan = compileGraph(bodyFile, component.graph.id, registry, {
    ...options,
    externalInputs: component.inputs.map((input) => ({ target: input.target, value: undefined })),
  });
  if (plan.diagnostics.length)
    throw new Error(
      `Component body cannot execute: ${plan.diagnostics.map((item) => item.message).join('; ')}`,
    );
  return {
    id: 'instance',
    version: 1,
    label: component.title,
    inputs,
    outputs,
    parameters,
    capabilities: ['browser'],
    execute: async (values, inputs, context) => {
      if (!options) throw new Error('Component requires an execution host');
      const inner = cloneJson(bodyFile);
      for (const control of component.controls) {
        const target = inner.project.graphs[0]!.nodes.find(
          (node) => node.id === control.target.nodeId,
        )!;
        Object.defineProperty(target.parameters, control.target.parameter, {
          value: cloneJson(values[control.id]!),
          enumerable: true,
          writable: true,
          configurable: true,
        });
      }
      const result = await runGraph(inner, component.graph.id, registry, {
        clone: options.clone,
        cancelled: () => context.cancelled,
        yield: context.yield,
        externalInputs: component.inputs.map((input) => ({
          target: input.target,
          value: inputs[input.id],
        })),
      });
      context.throwIfCancelled();
      if (result.state !== 'completed')
        throw new Error(
          `Component failed: ${result.error ?? result.diagnostics.map((item) => item.message).join('; ')}`,
        );
      return Object.fromEntries(
        component.outputs.map((output) => [
          output.id,
          result.outputs[output.source.nodeId]![output.source.portId],
        ]),
      );
    },
  };
}
