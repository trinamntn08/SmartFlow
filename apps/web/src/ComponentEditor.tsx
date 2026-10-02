import { useState } from 'react';
import {
  cloneJson,
  createProject,
  extractComponent,
  validateComponent,
  type ComponentDefinition,
} from '@smartflow/core';
import { EditorSession } from './editor-session.ts';
import { readWorkspace, patchWorkspace, record } from './workspace.ts';
import { GraphCanvas } from './GraphCanvas.tsx';
import { ParameterEditor } from './ParameterEditor.tsx';
import { catalog, componentContract } from './component-commands.ts';
interface Choice {
  key: string;
  section: 'inputs' | 'outputs' | 'controls';
  nodeId: string;
  endpoint: string;
  required: boolean;
  enabled: boolean;
  name: string;
  previousId?: string;
}
export function ComponentEditor({
  editor,
  graphId,
  selection,
  source,
  close,
  changed,
}: {
  editor: EditorSession;
  graphId: string;
  selection: string[];
  source?: ComponentDefinition | undefined;
  close(): void;
  changed(): void;
}) {
  const [revision] = useState(editor.history.revision);
  const [draft] = useState(() => {
    const file = createProject('component-draft');
    file.project.packages = cloneJson(editor.history.snapshot.project.packages);
    const original =
      source?.graph ??
      editor.history.snapshot.project.graphs.find((graph) => graph.id === graphId)!;
    const body = cloneJson(original);
    body.id = 'body';
    if (!source) {
      body.nodes = body.nodes.filter((node) => selection.includes(node.id));
      body.connections = body.connections.filter(
        (edge) => selection.includes(edge.source.nodeId) && selection.includes(edge.target.nodeId),
      );
    }
    file.project.graphs = [body];
    return new EditorSession(file, editor.registry);
  });
  const [file, setFile] = useState(draft.history.snapshot),
    [title, setTitle] = useState(source ? `${source.title} copy` : 'New component'),
    [error, setError] = useState('');
  const [settings, setSettings] = useState<Record<string, { enabled: boolean; name: string }>>({});
  const graph = file.project.graphs[0]!,
    workspace = readWorkspace(file, 'body'),
    selected = graph.nodes.find((node) => workspace.selection.includes(node.id));
  const action = (work: () => void) => {
    try {
      work();
      setFile(draft.history.snapshot);
      setError('');
    } catch (error) {
      setError(error instanceof Error ? error.message : String(error));
    }
  };
  const parent = editor.history.snapshot.project.graphs.find((graph) => graph.id === graphId)!;
  const choices: Choice[] = [];
  for (const node of graph.nodes) {
    const definition = draft.definition(node);
    if (!definition) continue;
    for (const section of ['inputs', 'outputs', 'controls'] as const) {
      const endpoints =
        section === 'inputs'
          ? definition.inputs
          : section === 'outputs'
            ? definition.outputs
            : definition.parameters;
      for (const endpoint of endpoints) {
        if (
          section === 'inputs' &&
          graph.connections.some(
            (edge) => edge.target.nodeId === node.id && edge.target.portId === endpoint.id,
          )
        )
          continue;
        if (section === 'controls' && !Object.hasOwn(node.parameters, endpoint.id)) continue;
        const required =
          !source &&
          section !== 'controls' &&
          parent.connections.some((edge) =>
            section === 'inputs'
              ? edge.target.nodeId === node.id &&
                edge.target.portId === endpoint.id &&
                !selection.includes(edge.source.nodeId)
              : edge.source.nodeId === node.id &&
                edge.source.portId === endpoint.id &&
                !selection.includes(edge.target.nodeId),
          );
        const previousEntries = source?.[section].filter((entry) => {
          const binding = section === 'outputs' ? entry.source : entry.target;
          return (
            binding &&
            typeof binding === 'object' &&
            !Array.isArray(binding) &&
            binding.nodeId === node.id &&
            binding[section === 'controls' ? 'parameter' : 'portId'] === endpoint.id
          );
        });
        for (const previous of previousEntries?.length ? previousEntries : [undefined]) {
          const key = JSON.stringify([section, node.id, endpoint.id, previous?.id ?? null]),
            defaultEnabled =
              !!previous ||
              (!source &&
                (required ||
                  section === 'inputs' ||
                  section === 'controls' ||
                  !graph.connections.some(
                    (edge) => edge.source.nodeId === node.id && edge.source.portId === endpoint.id,
                  )));
          const setting = settings[key];
          choices.push({
            key,
            section,
            nodeId: node.id,
            endpoint: endpoint.id,
            required,
            enabled: required || (setting?.enabled ?? defaultEnabled),
            name: setting?.name ?? previous?.id ?? `${node.id}_${endpoint.id}`,
            ...(previous ? { previousId: previous.id } : {}),
          });
        }
      }
    }
  }
  const save = () =>
    action(() => {
      if (editor.history.revision !== revision)
        throw new Error('Destination graph changed while this draft was open');
      const sections = { inputs: [], outputs: [], controls: [] } as Pick<
        ComponentDefinition,
        'inputs' | 'outputs' | 'controls'
      >;
      for (const choice of choices.filter((choice) => choice.enabled)) {
        const previous = source?.[choice.section].find((entry) => {
          const binding = choice.section === 'outputs' ? entry.source : entry.target;
          return (
            binding &&
            typeof binding === 'object' &&
            !Array.isArray(binding) &&
            entry.id === choice.previousId &&
            binding.nodeId === choice.nodeId &&
            binding[choice.section === 'controls' ? 'parameter' : 'portId'] === choice.endpoint
          );
        });
        if (choice.section === 'outputs')
          sections.outputs.push({
            ...previous,
            id: choice.name,
            source: { ...record(previous?.source), nodeId: choice.nodeId, portId: choice.endpoint },
          });
        else if (choice.section === 'inputs')
          sections.inputs.push({
            ...previous,
            id: choice.name,
            target: { ...record(previous?.target), nodeId: choice.nodeId, portId: choice.endpoint },
          });
        else
          sections.controls.push({
            ...previous,
            id: choice.name,
            target: {
              ...record(previous?.target),
              nodeId: choice.nodeId,
              parameter: choice.endpoint,
            },
          });
      }
      const definition = validateComponent({
        ...source,
        format: 'smartflow.graph-component',
        schemaVersion: 1,
        version: 1,
        id: crypto.randomUUID(),
        title,
        graph: cloneJson(graph),
        ...sections,
      });
      componentContract(editor, definition);
      if (!source) extractComponent(editor.history.snapshot, graphId, selection, definition);
      editor.history.edit(source ? 'Save component copy' : 'Extract component', (project) =>
        catalog(project, definition),
      );
      changed();
      close();
    });
  return (
    <div className="modal-backdrop">
      <section
        className="component-dialog"
        role="dialog"
        aria-label="Component editor"
        aria-modal="true"
        onKeyDown={(event) => {
          if (event.key === 'Escape') {
            event.preventDefault();
            close();
            return;
          }
          if (event.key === 'Tab') {
            const controls = Array.from(
              event.currentTarget.querySelectorAll<HTMLElement>(
                'button:not(:disabled),input:not(:disabled),select:not(:disabled),[tabindex="0"]',
              ),
            ).filter((element) => element.getClientRects().length);
            const first = controls[0],
              last = controls.at(-1);
            if (event.shiftKey && document.activeElement === first) {
              event.preventDefault();
              last?.focus();
            } else if (!event.shiftKey && document.activeElement === last) {
              event.preventDefault();
              first?.focus();
            }
          }
          if (event.target instanceof HTMLElement && event.target.matches('input,textarea,select'))
            return;
          if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'z') {
            event.preventDefault();
            action(() => {
              if (event.shiftKey) draft.history.redo();
              else draft.history.undo();
            });
          } else if (event.key === 'Delete') {
            event.preventDefault();
            action(() => draft.remove('body', workspace.selection, []));
          }
        }}
      >
        <div className="panel-title">
          <h2>{source ? 'Edit component copy' : 'Create component from selection'}</h2>
          <button onClick={close}>Cancel draft</button>
          <button onClick={save}>Save component</button>
        </div>
        <label>
          Title{' '}
          <input
            aria-label="Component title"
            autoFocus
            value={title}
            onChange={(event) => setTitle(event.target.value)}
          />
        </label>
        {error && <p role="alert">{error}</p>}
        <div className="draft-tools">
          <button
            disabled={!draft.history.canUndo}
            onClick={() =>
              action(() => {
                draft.history.undo();
              })
            }
          >
            Undo draft
          </button>
          <button
            disabled={!draft.history.canRedo}
            onClick={() =>
              action(() => {
                draft.history.redo();
              })
            }
          >
            Redo draft
          </button>
          <button onClick={() => action(() => draft.remove('body', workspace.selection, []))}>
            Delete draft selection
          </button>
          <select
            aria-label="Add draft node"
            defaultValue=""
            onChange={(event) => {
              const [packageId, typeId] = JSON.parse(event.target.value) as [string, string];
              action(() => draft.add('body', packageId, typeId, crypto.randomUUID()));
              event.target.value = '';
            }}
          >
            <option disabled value="">
              Add node to body
            </option>
            {editor.registry.extensions.flatMap((extension) =>
              extension.nodes.map((node) => (
                <option
                  key={`${extension.id}/${node.id}`}
                  value={JSON.stringify([extension.id, node.id])}
                >
                  {node.label}
                </option>
              )),
            )}
          </select>
        </div>
        <div className="draft-grid">
          <GraphCanvas
            graph={graph}
            workspace={workspace}
            definition={(node) => draft.definition(node)}
            onConnect={(edge) =>
              action(() =>
                draft.connection('body', {
                  id: crypto.randomUUID(),
                  source: { nodeId: edge.source, portId: edge.sourceHandle! },
                  target: { nodeId: edge.target, portId: edge.targetHandle! },
                }),
              )
            }
            onSelection={(nodes) =>
              action(() => patchWorkspace(draft.history, 'body', { selection: nodes }))
            }
            onPositions={(positions) =>
              action(() =>
                patchWorkspace(draft.history, 'body', {
                  positions: { ...workspace.positions, ...positions },
                }),
              )
            }
            onViewport={(viewport) =>
              action(() => patchWorkspace(draft.history, 'body', { viewport }))
            }
          />
          <aside>
            {selected &&
              draft
                .definition(selected)
                ?.parameters.map((parameter) => (
                  <ParameterEditor
                    key={`${selected.id}:${parameter.id}`}
                    parameter={parameter}
                    value={selected.parameters[parameter.id] ?? parameter.defaultValue}
                    commit={(value) =>
                      action(() => draft.parameter('body', selected.id, parameter.id, value))
                    }
                  />
                ))}
          </aside>
        </div>
        <div className="interface-grid">
          {(['inputs', 'outputs', 'controls'] as const).map((section) => (
            <fieldset key={section}>
              <legend>{section}</legend>
              {choices
                .filter((choice) => choice.section === section)
                .map((choice) => (
                  <div className="interface-choice" key={choice.key}>
                    <label>
                      <input
                        type="checkbox"
                        checked={choice.enabled}
                        disabled={choice.required}
                        onChange={(event) =>
                          setSettings({
                            ...settings,
                            [choice.key]: { enabled: event.target.checked, name: choice.name },
                          })
                        }
                      />
                      {choice.nodeId}:{choice.endpoint}
                    </label>
                    <input
                      aria-label={`Name ${section} ${choice.nodeId}:${choice.endpoint}`}
                      value={choice.name}
                      disabled={!choice.enabled}
                      onChange={(event) =>
                        setSettings({
                          ...settings,
                          [choice.key]: { enabled: choice.enabled, name: event.target.value },
                        })
                      }
                    />
                  </div>
                ))}
            </fieldset>
          ))}
        </div>
      </section>
    </div>
  );
}
