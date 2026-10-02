import { useCallback, useEffect, useRef, useState } from 'react';
import {
  parseProject,
  serializeProject,
  type NodeDocument,
  type ProjectFile,
} from '@smartflow/core';
import { ExtensionRegistry } from '@smartflow/extension-sdk';
import { dataExtension } from '@smartflow/data';
import { sceneExtension } from '@smartflow/scene-3d';
import type { Connection } from '@xyflow/react';
import dataText from '../../../examples/data.smartflow?raw';
import sceneText from '../../../examples/scene.smartflow?raw';
import componentText from '../../../examples/components.smartflow?raw';
import { parseProjectBytes } from './project-files.ts';
import { EditorSession, emptyProject } from './editor-session.ts';
import { patchWorkspace, readWorkspace, record, WEB_WORKSPACE } from './workspace.ts';
import { GraphCanvas } from './GraphCanvas.tsx';
import { ParameterEditor } from './ParameterEditor.tsx';
import { ExecutionClient, createExecutionWorker, type ExecutionState } from './execution-client.ts';
import { OutputPanel } from './OutputPanel.tsx';
import { ComponentLibrary } from './ComponentLibrary.tsx';

const registry = new ExtensionRegistry([dataExtension, sceneExtension]);
const examples: Record<string, string> = {
  data: dataText,
  scene: sceneText,
  components: componentText,
};
export function App() {
  const [session] = useState(() => new EditorSession(parseProject(dataText), registry));
  const [execution, setExecution] = useState<ExecutionState>({ phase: 'idle', statuses: {} });
  const [executor] = useState(() => new ExecutionClient(createExecutionWorker, setExecution));
  const [file, setFile] = useState(session.history.snapshot);
  const [graphId, setGraphId] = useState(file.project.graphs[0]?.id ?? '');
  const [selectedEdges, setSelectedEdges] = useState<string[]>([]);
  const [loadNumber, setLoadNumber] = useState(0);
  const [error, setError] = useState('');
  const [notice, setNotice] = useState('');
  const [search, setSearch] = useState('');
  const [filename, setFilename] = useState('data.smartflow');
  const importInput = useRef<HTMLInputElement>(null);
  const refresh = useCallback(() => {
    setFile(session.history.snapshot);
  }, [session]);
  const action = useCallback(
    (work: () => void) => {
      try {
        work();
        setError('');
        refresh();
      } catch (error) {
        setError(error instanceof Error ? error.message : String(error));
      }
    },
    [refresh],
  );
  const graph = file.project.graphs.find((graph) => graph.id === graphId);
  const workspace = readWorkspace(file, graphId);
  const rawLayout = record(record(file.workspace[WEB_WORKSPACE])[graphId]).layout;
  const layout = record(rawLayout);
  const width = (key: string, fallback: number) =>
    typeof layout[key] === 'number' && layout[key] >= 180 && layout[key] <= 440
      ? layout[key]
      : fallback;
  const showLibrary = layout.library !== false,
    showInspector = layout.inspector !== false;
  const layoutPatch = (patch: import('@smartflow/core').JsonObject, reset = false) =>
    action(() => {
      if (
        !reset &&
        rawLayout !== undefined &&
        (rawLayout === null || typeof rawLayout !== 'object' || Array.isArray(rawLayout))
      )
        throw new Error('Unsupported saved layout; use Reset layout to replace it explicitly');
      patchWorkspace(session.history, graphId, { layout: { ...layout, ...patch } });
    });
  useEffect(() => {
    executor.invalidate(session.history.revision, graphId);
  }, [executor, session, file, graphId]);
  useEffect(() => () => executor.dispose(), [executor]);
  const selected = graph?.nodes.find((node) => workspace.selection.includes(node.id));
  const definition = selected && session.definition(selected);
  const resolve = useCallback((node: NodeDocument) => session.definition(node), [session]);
  const remove = useCallback(() => {
    if (graph)
      action(() =>
        session.remove(
          graphId,
          workspace.selection.filter((id) => graph.nodes.some((node) => node.id === id)),
          selectedEdges,
        ),
      );
  }, [action, graph, graphId, session, workspace.selection, selectedEdges]);
  useEffect(() => {
    const onKey = (event: KeyboardEvent) => {
      if (document.querySelector('[role="dialog"][aria-modal="true"]')) return;
      if (
        event.target instanceof HTMLElement &&
        (event.target.matches('input,textarea,select') || event.target.isContentEditable)
      )
        return;
      if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'z') {
        event.preventDefault();
        action(() => {
          if (event.shiftKey) session.history.redo();
          else session.history.undo();
        });
      } else if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'y') {
        event.preventDefault();
        action(() => {
          session.history.redo();
        });
      } else if (event.key === 'Delete' || event.key === 'Backspace') {
        event.preventDefault();
        remove();
      }
    };
    window.addEventListener('keydown', onKey);
    return () => window.removeEventListener('keydown', onKey);
  }, [action, remove, session]);
  useEffect(() => {
    const handler = (event: BeforeUnloadEvent) => {
      if (session.history.dirty) {
        event.preventDefault();
        event.returnValue = '';
      }
    };
    window.addEventListener('beforeunload', handler);
    return () => window.removeEventListener('beforeunload', handler);
  }, [session]);
  const replace = (replacement: ProjectFile, name: string) => {
    if (
      session.history.dirty &&
      !window.confirm(
        'Discard changes to the current project? Download it first to keep your edits.',
      )
    )
      return;
    action(() => {
      session.history.replace(replacement);
      setGraphId(replacement.project.graphs[0]?.id ?? '');
      setSelectedEdges([]);
      setLoadNumber((value) => value + 1);
      setFilename(name);
      setNotice('Project opened. Opening does not execute it.');
    });
  };
  const download = () =>
    action(() => {
      const url = URL.createObjectURL(
        new Blob([serializeProject(session.history.snapshot)], { type: 'application/json' }),
      );
      const link = document.createElement('a');
      link.href = url;
      link.download = filename.replace(/[^a-zA-Z0-9._-]/g, '_');
      link.click();
      setTimeout(() => URL.revokeObjectURL(url), 1000);
      session.history.markSaved();
      setNotice('Project download prepared. Keep the downloaded file to save your work.');
    });
  const onConnect = (connection: Connection) =>
    action(() => {
      if (!connection.sourceHandle || !connection.targetHandle)
        throw new Error('Choose named ports');
      session.connection(graphId, {
        id: crypto.randomUUID(),
        source: { nodeId: connection.source, portId: connection.sourceHandle },
        target: { nodeId: connection.target, portId: connection.targetHandle },
      });
    });
  return (
    <main className="app-shell">
      <header className="app-header">
        <div className="brand">
          <span className="mark">SF</span>
          <div>
            <strong>SmartFlow</strong>
            <small>Visual workspace</small>
          </div>
        </div>
        <div className="project-name">
          {filename}
          <span className={session.history.dirty ? 'dirty' : 'saved'}>
            {session.history.dirty ? 'Unsaved changes' : 'Saved'}
          </span>
        </div>
        <span className="preview-badge">Web preview</span>
      </header>
      <div className="toolbar" aria-label="Project actions">
        <button onClick={() => replace(emptyProject(crypto.randomUUID()), 'untitled.smartflow')}>
          New project
        </button>
        <button onClick={() => importInput.current?.click()}>Open project</button>
        <button onClick={download}>Download project</button>
        <span className="toolbar-divider" />
        <button
          disabled={!graph || !graph.nodes.length}
          onClick={() => executor.run(session.history.snapshot, session.history.revision, graphId)}
        >
          Run graph
        </button>
        <button disabled={execution.phase !== 'running'} onClick={() => executor.cancel()}>
          Cancel run
        </button>
        <button
          disabled={!session.history.canUndo}
          title={session.history.undoLabel}
          onClick={() =>
            action(() => {
              session.history.undo();
            })
          }
        >
          Undo
        </button>
        <button
          disabled={!session.history.canRedo}
          title={session.history.redoLabel}
          onClick={() =>
            action(() => {
              session.history.redo();
            })
          }
        >
          Redo
        </button>
        <button disabled={!workspace.selection.length && !selectedEdges.length} onClick={remove}>
          Delete selection
        </button>
        <div className="toolbar-spacer" />
        <details className="layout-controls">
          <summary>Workspace layout</summary>
          <div>
            <label>
              <input
                type="checkbox"
                checked={showLibrary}
                onChange={(event) => layoutPatch({ library: event.target.checked })}
              />
              Node library
            </label>
            <label>
              <input
                type="checkbox"
                checked={showInspector}
                onChange={(event) => layoutPatch({ inspector: event.target.checked })}
              />
              Inspector
            </label>
            <label>
              Library width{' '}
              <input
                aria-label="Library width"
                type="range"
                min="180"
                max="440"
                value={width('libraryWidth', 225)}
                onChange={(event) => layoutPatch({ libraryWidth: Number(event.target.value) })}
              />
            </label>
            <label>
              Inspector width{' '}
              <input
                aria-label="Inspector width"
                type="range"
                min="180"
                max="440"
                value={width('inspectorWidth', 280)}
                onChange={(event) => layoutPatch({ inspectorWidth: Number(event.target.value) })}
              />
            </label>
            <button
              onClick={() =>
                layoutPatch(
                  {
                    library: true,
                    inspector: true,
                    libraryWidth: 225,
                    inspectorWidth: 280,
                  },
                  true,
                )
              }
            >
              Reset layout
            </button>
          </div>
        </details>
        <label className="compact-label">
          Examples
          <select
            aria-label="Open example"
            defaultValue=""
            onChange={(event) => {
              const value = examples[event.target.value];
              if (value) replace(parseProject(value), `${event.target.value}.smartflow`);
              event.target.value = '';
            }}
          >
            <option value="" disabled>
              Choose a workflow
            </option>
            <option value="data">Data · filtered summary</option>
            <option value="scene">3D · colored cube</option>
            <option value="components">Reusable component</option>
          </select>
        </label>
      </div>
      <input
        ref={importInput}
        type="file"
        accept=".smartflow,application/json"
        aria-label="Project file"
        className="file-input"
        onChange={async (event) => {
          const input = event.currentTarget;
          const imported = input.files?.[0];
          if (!imported) return;
          try {
            if (imported.size > 16 * 1024 * 1024) throw new Error('JSON exceeds 16 MiB file limit');
            replace(parseProjectBytes(new Uint8Array(await imported.arrayBuffer())), imported.name);
          } catch (error) {
            setError(error instanceof Error ? error.message : String(error));
          } finally {
            input.value = '';
          }
        }}
      />
      {error && (
        <div role="alert" className="error-banner">
          <strong>Action could not be completed.</strong> {error}
          <button aria-label="Dismiss error" onClick={() => setError('')}>
            ×
          </button>
        </div>
      )}
      <div
        className="workspace-grid"
        style={{
          gridTemplateColumns: `${showLibrary ? `${width('libraryWidth', 225)}px ` : ''}minmax(300px,1fr)${showInspector ? ` ${width('inspectorWidth', 280)}px` : ''}`,
        }}
      >
        <aside className="library panel" hidden={!showLibrary}>
          <div className="panel-title">
            <h2>Node library</h2>
            <span>
              {registry.extensions.reduce((sum, extension) => sum + extension.nodes.length, 0)}
            </span>
          </div>
          <input
            className="search"
            aria-label="Search nodes"
            placeholder="Find an operation…"
            value={search}
            onChange={(event) => setSearch(event.target.value)}
          />
          {registry.extensions.map((extension) => (
            <section className="library-group" key={extension.id}>
              <h3>{extension.id === 'smartflow.data' ? 'Data' : 'Scene 3D'}</h3>
              {extension.nodes
                .filter((node) => node.label.toLowerCase().includes(search.toLowerCase()))
                .map((node) => (
                  <button
                    className="library-node"
                    key={node.id}
                    disabled={!graph}
                    onClick={() =>
                      action(() => session.add(graphId, extension.id, node.id, crypto.randomUUID()))
                    }
                  >
                    <span
                      className={`node-dot ${extension.id.includes('scene') ? 'scene' : 'data'}`}
                    />
                    <span>{node.label}</span>
                    <span className="add-symbol">+</span>
                  </button>
                ))}
            </section>
          ))}
          <div className="library-note">
            Connect an output to an input. Select a node to edit its parameters.
          </div>
          <ComponentLibrary
            editor={session}
            file={file}
            graphId={graphId}
            selection={workspace.selection}
            action={action}
          />
        </aside>
        <section className="graph-panel">
          <div className="panel-title graph-title">
            <h2>Graph</h2>
            <select
              aria-label="Active graph"
              value={graphId}
              onChange={(event) => {
                setGraphId(event.target.value);
                setSelectedEdges([]);
              }}
            >
              {file.project.graphs.map((graph) => (
                <option key={graph.id} value={graph.id}>
                  {graph.id}
                </option>
              ))}
            </select>
            <span>
              {graph?.nodes.length ?? 0} nodes · {graph?.connections.length ?? 0} connections
            </span>
          </div>
          {graph ? (
            <GraphCanvas
              key={`${loadNumber}:${graphId}`}
              graph={graph}
              workspace={workspace}
              definition={resolve}
              statuses={execution.phase === 'obsolete' ? {} : execution.statuses}
              onConnect={onConnect}
              onSelection={(nodes, edges) => {
                setSelectedEdges((current) =>
                  JSON.stringify(current) === JSON.stringify(edges) ? current : edges,
                );
                if (
                  JSON.stringify(nodes) !==
                  JSON.stringify(
                    workspace.selection.filter((id) => graph.nodes.some((node) => node.id === id)),
                  )
                )
                  action(() => patchWorkspace(session.history, graphId, { selection: nodes }));
              }}
              onPositions={(positions) =>
                action(() =>
                  patchWorkspace(session.history, graphId, {
                    positions: { ...workspace.positions, ...positions },
                  }),
                )
              }
              onViewport={(viewport) =>
                action(() => patchWorkspace(session.history, graphId, { viewport }))
              }
            />
          ) : (
            <div className="canvas-empty">
              <strong>This project has no graphs</strong>
              <button
                onClick={() =>
                  action(() => {
                    const id = crypto.randomUUID();
                    session.history.edit('Add graph', (project) => {
                      project.graphs.push({ id, nodes: [], connections: [] });
                    });
                    setGraphId(id);
                  })
                }
              >
                Add graph
              </button>
            </div>
          )}
        </section>
        <aside className="inspector panel" hidden={!showInspector}>
          <div className="panel-title">
            <h2>Inspector</h2>
            <span>
              {workspace.selection.length
                ? `${workspace.selection.length} selected`
                : 'No selection'}
            </span>
          </div>
          {selected ? (
            <>
              <h3 className="inspector-heading">{definition?.label ?? selected.typeId}</h3>
              <div className="muted small">
                {selected.packageId} · v{selected.version}
              </div>
              {definition ? (
                <div className="parameters">
                  {definition.parameters.map((parameter) => (
                    <ParameterEditor
                      key={`${selected.id}:${parameter.id}`}
                      parameter={parameter}
                      value={selected.parameters[parameter.id] ?? parameter.defaultValue}
                      commit={(value) =>
                        action(() => session.parameter(graphId, selected.id, parameter.id, value))
                      }
                    />
                  ))}
                  {!definition.parameters.length && (
                    <p className="muted">This node has no parameters.</p>
                  )}
                  <p className="contract-note">
                    Run explicitly to inspect outputs. Parameter edits make earlier results
                    obsolete.
                  </p>
                </div>
              ) : (
                <div className="unsupported-note">
                  This node contract is unavailable. Its parameters and connections remain in the
                  project.
                </div>
              )}
              <details>
                <summary>Retained node content</summary>
                <pre>{JSON.stringify(selected, null, 2)}</pre>
              </details>
            </>
          ) : (
            <div className="inspector-empty">
              <span>↗</span>
              <strong>Select a node</strong>
              <p>Inspect its parameters and ports here.</p>
            </div>
          )}
          <div className="connection-list">
            <h3>Connections</h3>
            {graph?.connections.map((edge) => (
              <div key={edge.id}>
                <span>
                  {edge.source.nodeId.slice(0, 12)}:{edge.source.portId} →{' '}
                  {edge.target.nodeId.slice(0, 12)}:{edge.target.portId}
                </span>
                <button
                  aria-label={`Remove connection ${edge.id}`}
                  onClick={() => action(() => session.remove(graphId, [], [edge.id]))}
                >
                  ×
                </button>
              </div>
            ))}
          </div>
        </aside>
      </div>
      <OutputPanel
        editSource={(id) =>
          action(() => {
            const node = graph?.nodes.find((node) => node.id === id);
            if (!node || node.packageId !== 'smartflow.scene-3d' || node.typeId !== 'cube')
              throw new Error('The source cube is not editable in this graph');
            const size = node.parameters.size ?? 2;
            if (typeof size !== 'number') throw new Error('Invalid cube size');
            session.parameter(graphId, id, 'size', Math.min(100, size + 0.5));
          })
        }
        execution={execution}
        workspace={workspace}
        patch={(value) => action(() => patchWorkspace(session.history, graphId, value))}
      />
      <footer className="statusbar">
        <span>{notice || 'Local workspace · project files stay on your device'}</span>
        <span>Schema 1 · {file.project.assets.length} asset references</span>
      </footer>
    </main>
  );
}
