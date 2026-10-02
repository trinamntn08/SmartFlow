import { useRef, useState } from 'react';
import {
  isComponentInstance,
  parseComponent,
  serializeComponent,
  validateComponent,
  type ComponentDefinition,
  type Endpoint,
  type JsonValue,
  type ProjectFile,
} from '@smartflow/core';
import { EditorSession } from './editor-session.ts';
import { catalog, componentContract, instantiate, replaceInstance } from './component-commands.ts';
import { ComponentEditor } from './ComponentEditor.tsx';
import { ParameterEditor } from './ParameterEditor.tsx';
export function ComponentLibrary({
  editor,
  file,
  graphId,
  selection,
  action,
}: {
  editor: EditorSession;
  file: ProjectFile;
  graphId: string;
  selection: string[];
  action(work: () => void): void;
}) {
  const input = useRef<HTMLInputElement>(null),
    [chosen, setChosen] = useState(''),
    [draft, setDraft] = useState<{ source?: ComponentDefinition | undefined } | undefined>(),
    [sources, setSources] = useState<Record<string, Endpoint>>({}),
    [controls, setControls] = useState<Record<string, JsonValue>>({});
  const entries = Array.isArray(file.project.components) ? file.project.components : [];
  const index = Number(chosen),
    raw = chosen !== '' ? entries[index] : undefined;
  let definition: ComponentDefinition | undefined,
    problem = '',
    contract: ReturnType<typeof componentContract> | undefined;
  try {
    if (raw !== undefined) {
      definition = validateComponent(raw);
      contract = componentContract(editor, definition);
    }
  } catch (error) {
    problem = error instanceof Error ? error.message : String(error);
  }
  const graph = file.project.graphs.find((graph) => graph.id === graphId),
    selected = graph?.nodes.find((node) => selection.includes(node.id));
  const outputs =
    graph?.nodes.flatMap(
      (node) =>
        editor
          .definition(node)
          ?.outputs.map((port) => ({ nodeId: node.id, portId: port.id, typeId: port.typeId })) ??
        [],
    ) ?? [];
  const exportFile = () =>
    action(() => {
      if (!definition) throw new Error('Choose a supported definition');
      const url = URL.createObjectURL(
        new Blob([serializeComponent(definition)], { type: 'application/json' }),
      );
      const link = document.createElement('a');
      link.href = url;
      link.download = `${definition.title.replace(/[^a-zA-Z0-9._-]/g, '_')}.smartflow-component`;
      link.click();
      setTimeout(() => URL.revokeObjectURL(url), 1000);
    });
  return (
    <section className="component-library">
      <h3>Components</h3>
      <div className="component-actions">
        <button onClick={() => input.current?.click()}>Import component</button>
        <button disabled={!selection.length} onClick={() => setDraft({})}>
          Create component
        </button>
      </div>
      <input
        ref={input}
        className="file-input"
        type="file"
        accept=".smartflow-component,.json"
        aria-label="Component file"
        onChange={async (event) => {
          const element = event.currentTarget,
            imported = element.files?.[0];
          if (!imported) return;
          try {
            if (imported.size > 16 * 1024 * 1024) throw new Error('JSON exceeds 16 MiB file limit');
            const definition = parseComponent(
              new TextDecoder('utf-8', { fatal: true, ignoreBOM: true }).decode(
                await imported.arrayBuffer(),
              ),
            );
            action(() => {
              editor.history.edit('Import component', (project) => catalog(project, definition));
            });
          } catch (error) {
            action(() => {
              throw error;
            });
          } finally {
            element.value = '';
          }
        }}
      />
      <select
        aria-label="Component library"
        value={chosen}
        onChange={(event) => {
          setChosen(event.target.value);
          setSources({});
          setControls({});
        }}
      >
        <option value="">Choose a definition</option>
        {entries.map((entry, index) => (
          <option key={index} value={index}>
            {entry &&
            typeof entry === 'object' &&
            !Array.isArray(entry) &&
            typeof entry.title === 'string'
              ? entry.title
              : `Unsupported entry ${index + 1}`}
          </option>
        ))}
      </select>
      {raw !== undefined && (
        <>
          <div className="component-actions">
            <button disabled={!definition} onClick={exportFile}>
              Export component
            </button>
            <button
              onClick={() =>
                action(() => {
                  editor.history.edit('Remove component catalog entry', (project) => {
                    if (!Array.isArray(project.components)) throw new Error('Unsupported catalog');
                    project.components.splice(index, 1);
                  });
                  setChosen('');
                })
              }
            >
              Remove component
            </button>
            <button disabled={!contract} onClick={() => setDraft({ source: definition })}>
              Edit copy
            </button>
          </div>
          {problem && <p className="unsupported-note">{problem}</p>}
          {contract && definition && (
            <>
              <div className="component-bindings">
                {contract.inputs.map((port) => (
                  <label key={port.id}>
                    {port.id}
                    <select
                      aria-label={`Source ${port.id}`}
                      value={sources[port.id] ? JSON.stringify(sources[port.id]) : ''}
                      onChange={(event) =>
                        setSources({
                          ...sources,
                          [port.id]: JSON.parse(event.target.value) as Endpoint,
                        })
                      }
                    >
                      <option disabled value="">
                        Choose matching output
                      </option>
                      {outputs
                        .filter((output) => output.typeId === port.typeId)
                        .map((output) => (
                          <option
                            key={JSON.stringify(output)}
                            value={JSON.stringify({ nodeId: output.nodeId, portId: output.portId })}
                          >
                            {output.nodeId}:{output.portId}
                          </option>
                        ))}
                    </select>
                  </label>
                ))}
                {contract.parameters.map((parameter) => (
                  <ParameterEditor
                    key={`${definition.id}:${parameter.id}`}
                    parameter={parameter}
                    value={controls[parameter.id] ?? parameter.defaultValue}
                    commit={(value) => setControls({ ...controls, [parameter.id]: value })}
                  />
                ))}
              </div>
              <button
                onClick={() =>
                  action(() =>
                    instantiate(
                      editor,
                      graphId,
                      definition!,
                      crypto.randomUUID(),
                      sources,
                      controls,
                    ),
                  )
                }
              >
                Insert component
              </button>
              {selected && isComponentInstance(selected) && (
                <details>
                  <summary>Update selected instance</summary>
                  <p>
                    Source: {editor.definition(selected)?.label ?? selected.id}. Target:{' '}
                    {definition.title}. Existing control values will be carried by name.
                  </p>
                  <details>
                    <summary>Review target body and defaults</summary>
                    <pre>{JSON.stringify(definition.graph, null, 2)}</pre>
                  </details>
                  <button
                    onClick={() =>
                      action(() => replaceInstance(editor, graphId, selected.id, definition!))
                    }
                  >
                    Apply compatible update
                  </button>
                </details>
              )}
            </>
          )}
        </>
      )}
      {file.project.components !== undefined && !Array.isArray(file.project.components) && (
        <p className="unsupported-note">Unsupported catalog shape retained.</p>
      )}
      {draft && (
        <ComponentEditor
          editor={editor}
          graphId={graphId}
          selection={selection}
          source={draft.source}
          close={() => setDraft(undefined)}
          changed={() => action(() => {})}
        />
      )}
    </section>
  );
}
