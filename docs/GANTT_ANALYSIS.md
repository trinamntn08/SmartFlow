# Process Gantt analysis

Updated: 2026-10-02. Desktop and browser preview include execution timing analysis
for scene, data and component workflows. This is process profiling, not a calendar
or project scheduling editor.

## Use

Desktop: choose **Process Gantt** in any workspace region selector. Split a region
with H/V to show it beside the graph. Its placement saves/reopens with the normal
widget layout. Press Run; click a timing row to select its visible graph node.
The independent data executable has the same widget.

Browser/web mode: **Process Gantt** appears below the graph. Click a node name to
select it. Large runs have Previous/Next timings controls with 100 rows per page.
The application remains a browser preview; this change does not introduce an
embedded Qt WebEngine shell.

After Run, each native and browser graph node also displays **Run: ... ms**.
Native Execution results includes a **Run ms** column. No Gantt panel needs to be
open to see per-node durations. Nodes that have not executed show no duration;
graph edits clear the canvas timings. Collapsed component labels show the wall
span from their first invoked body step to the last terminal body step, including
gaps between steps rather than the sum of parallel step durations.

Both views show a common relative millisecond timeline, amber ready-queue bars,
blue invocation bars, status and numeric queue/run durations. Desktop parallel
branches can overlap; browser execution remains sequential. Native component
bodies have individual compiled-step rows selecting the owning component; browser
components show one whole-component span, including their body and worker yields.
These timings therefore support analysis within each mode rather than strict
cross-backend benchmarks.

## Meaning and lifecycle

Timings use a monotonic run origin. Queue time starts when dependencies are
satisfied and ends when invocation starts; dependency waiting is excluded.
Invocation spans include input preparation, node execution, output validation and
publication/cloning. They are wall-clock intervals, not CPU time or measurements
of individual managed internal threads. Total elapsed includes preparation and
scheduler overhead, so summed invocation times are not total run time, especially
with parallel branches.

Rows without a start have no execution duration. Failed/cancelled invocations keep
their observed spans; skipped nodes are not displayed as successfully executed.
Cancellation in the browser terminates the worker; the endpoint of an unfinished
span is the client-observed cancellation time based on the latest worker clock
sample. Desktop cooperative cancellation ends when work returns. Final timelines
freeze. A new run replaces them; late replies cannot replace current timings.
Native semantic edits clear timings, while browser edits label retained timings
obsolete. Project files contain no profiling data. Saving preserves workspace
placement/selection only; reopen requires another Run to obtain timings.

## Legacy review and reuse

Read-only review of the old studio-engine checkout at commit
`d6f457e8415445d00eaf7e9fbf9e5047768ea637` examined:

- `tp_qt_pipeline_widgets/src/displays/GanttDisplay.cpp` and its header: display
  registration, selection and chart interactions.
- `tp_qt_pipeline_widgets/src/profiling/PipelineGanttModel.cpp`: ready/start/end
  profiling timestamps, queue/run durations and normalized millisecond coordinates.
- `tp_qt_pipeline_widgets/inc/tp_qt_pipeline_widgets/CustomGanttDelegate.h`: two
  bar phases and timing tooltips.
- `tp_qt_pipeline_widgets/src/profiling/ProfilingStepPainter.cpp` and
  `StepDelegatePainter.cpp`: canvas timing statistics and queue/run/total tooltips.
  The former draws legacy total-time/percentage statistics below nodes; SmartFlow
  shows invocation time explicitly, with queue timing available in Gantt.
- The `NumericTimelineGrid` header and module references: KDGantt numeric chart
  integration rather than calendar dates.

The old display relies on KDGantt and the legacy application/display framework.
SmartFlow independently implements the reviewed behavior with a Qt item delegate
and browser SVG, using its synchronized execution snapshots and worker protocol.
No legacy Gantt source was copied, no additional dependency/license was imported,
and the old checkout was not changed. There is no old-checkout build/runtime path.
Legacy wheel zoom/panning, dependency arrows, per-task fraction-of-total and trace
export are not part of this checkpoint; charts fit the observed run extent.
