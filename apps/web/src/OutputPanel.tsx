import type { JsonObject } from '@smartflow/core';
import { isTable } from '@smartflow/data';
import type { ExecutionState } from './execution-client.ts';
import { record, type GraphWorkspace } from './workspace.ts';

export function OutputPanel({
  execution,
  workspace,
  patch,
}: {
  execution: ExecutionState;
  workspace: GraphWorkspace;
  patch(value: JsonObject): void;
}) {
  const outputs = execution.result?.outputs ?? {};
  const choices = Object.entries(outputs).flatMap(([nodeId, ports]) =>
    Object.keys(ports).map((portId) => ({ nodeId, portId })),
  );
  const selected =
    choices.find(
      (choice) =>
        choice.nodeId === workspace.pinned && choice.portId === (workspace.pinnedPort ?? 'out'),
    ) ?? choices.at(-1);
  const value = selected && outputs[selected.nodeId]![selected.portId];
  const viewer = record(workspace.viewers['smartflow.data.table-viewer@1']);
  return (
    <section className="output-panel panel" aria-label="Execution output">
      <div className="panel-title">
        <h2>Output</h2>
        <strong role="status" aria-label="Execution state">
          {execution.phase}
        </strong>
        <select
          aria-label="Viewed output"
          value={selected ? JSON.stringify([selected.nodeId, selected.portId]) : ''}
          onChange={(event) => {
            const [pinned, pinnedPort] = JSON.parse(event.target.value) as [string, string];
            patch({ pinned, pinnedPort });
          }}
        >
          <option value="" disabled>
            Choose an output
          </option>
          {choices.map((choice) => (
            <option
              key={JSON.stringify(choice)}
              value={JSON.stringify([choice.nodeId, choice.portId])}
            >
              {choice.nodeId}:{choice.portId}
            </option>
          ))}
        </select>
      </div>
      {execution.phase === 'obsolete' && (
        <p className="obsolete-note">
          These results belong to an older graph revision. Run again to refresh them.
        </p>
      )}
      {(execution.error || execution.result?.error) && (
        <p role="alert">{execution.error ?? execution.result?.error}</p>
      )}
      {!!execution.result?.diagnostics.length && (
        <ul>
          {execution.result.diagnostics.map((item, index) => (
            <li key={index}>
              {item.nodeId ?? item.connectionId}: {item.message}
            </li>
          ))}
        </ul>
      )}
      {isTable(value) ? (
        <table aria-label="Table output">
          <thead>
            <tr>
              <th>Label</th>
              <th>Value</th>
            </tr>
          </thead>
          <tbody>
            {value.rows.map((row, index) => (
              <tr
                key={index}
                className={viewer.selectedRow === index ? 'selected-row' : ''}
                tabIndex={0}
                onClick={() =>
                  patch({
                    viewers: {
                      ...workspace.viewers,
                      'smartflow.data.table-viewer@1': { ...viewer, selectedRow: index },
                    },
                  })
                }
                onKeyDown={(event) => {
                  if (event.key === 'Enter')
                    patch({
                      viewers: {
                        ...workspace.viewers,
                        'smartflow.data.table-viewer@1': { ...viewer, selectedRow: index },
                      },
                    });
                }}
              >
                <td>{row.label}</td>
                <td>{row.value.toLocaleString(undefined, { maximumFractionDigits: 8 })}</td>
              </tr>
            ))}
          </tbody>
        </table>
      ) : (
        <p className="muted">
          {choices.length
            ? 'No viewer is available for this output.'
            : 'Run the graph to inspect its outputs.'}
        </p>
      )}
      <div className="node-statuses" aria-label="Node execution statuses">
        {Object.entries(execution.statuses)
          .slice(0, 100)
          .map(([id, state]) => (
            <span key={id}>
              {id}: {state}
            </span>
          ))}
        {Object.keys(execution.statuses).length > 100 && (
          <span>{Object.keys(execution.statuses).length} node states; first 100 shown here.</span>
        )}
      </div>
    </section>
  );
}
