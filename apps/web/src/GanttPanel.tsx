import { useState } from 'react';
import type { ExecutionState } from './execution-client.ts';
export function GanttPanel({
  execution,
  select,
}: {
  execution: ExecutionState;
  select(id: string): void;
}) {
  const [page, setPage] = useState(0);
  const timings = execution.timings ?? {};
  const entries = Object.entries(timings);
  const lastPage = Math.max(0, Math.ceil(entries.length / 100) - 1);
  const currentPage = Math.min(page, lastPage);
  const elapsed = execution.elapsedMs ?? 0;
  const extent = Math.max(elapsed, 0.001);
  return (
    <section className="panel gantt-panel" aria-label="Process Gantt">
      <h2>Process Gantt</h2>
      <p>0 - {elapsed.toFixed(3)} ms | Amber: ready queue | Blue: execution</p>
      {execution.phase === 'obsolete' && (
        <p>Timings belong to an older graph revision. Run again to refresh.</p>
      )}
      {entries.length > 100 && (
        <div>
          <button disabled={currentPage === 0} onClick={() => setPage(currentPage - 1)}>
            Previous timings
          </button>
          <span>
            {' '}
            Page {currentPage + 1} / {lastPage + 1} | {entries.length} nodes{' '}
          </span>
          <button disabled={currentPage === lastPage} onClick={() => setPage(currentPage + 1)}>
            Next timings
          </button>
        </div>
      )}
      {!entries.length ? (
        <p>Run the graph to inspect process timings.</p>
      ) : (
        <table aria-label="Process timings">
          <thead>
            <tr>
              <th>Node</th>
              <th>State</th>
              <th>Queue ms</th>
              <th>Run ms</th>
              <th>Timeline (ms)</th>
            </tr>
          </thead>
          <tbody>
            {entries.slice(currentPage * 100, (currentPage + 1) * 100).map(([id, timing]) => {
              const end = timing.endedMs ?? elapsed;
              const queueEnd = timing.startedMs ?? end;
              const bar = (from: number, to: number, color: string) => (
                <rect
                  x={(1000 * from) / extent}
                  y="5"
                  width={Math.max(1, (1000 * (to - from)) / extent)}
                  height="18"
                  fill={color}
                />
              );
              return (
                <tr key={id}>
                  <td>
                    <button onClick={() => select(id)}>{id}</button>
                  </td>
                  <td>{execution.statuses[id] ?? 'waiting'}</td>
                  <td>
                    {timing.readyMs === undefined ? '-' : (queueEnd - timing.readyMs).toFixed(3)}
                  </td>
                  <td>
                    {timing.startedMs === undefined ? '-' : (end - timing.startedMs).toFixed(3)}
                  </td>
                  <td>
                    <svg
                      viewBox="0 0 1000 28"
                      preserveAspectRatio="none"
                      role="img"
                      aria-label={`${id} process timeline`}
                    >
                      {[0, 250, 500, 750, 1000].map((x) => (
                        <line key={x} x1={x} x2={x} y1="0" y2="28" stroke="#8894a4" opacity="0.3" />
                      ))}
                      {timing.readyMs !== undefined && bar(timing.readyMs, queueEnd, '#d79b35')}
                      {timing.startedMs !== undefined && bar(timing.startedMs, end, '#3694d6')}
                    </svg>
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
      )}
    </section>
  );
}
