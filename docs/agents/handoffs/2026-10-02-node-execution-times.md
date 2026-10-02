# Task handoff: per-node execution times

## Objective and scope

Follow up the user's request to show each node's execution duration when a chain
runs. Read-only legacy review confirmed `ProfilingStepPainter.cpp` paints timing
statistics on nodes and `StepDelegatePainter.cpp` supplies queue/run/total tooltips.
Keep the old checkout unchanged and use existing transient run timestamps.

## Current state

- Native supported graph nodes embed a compact **Run: ... ms** label; **Execution
  results** has a **Run ms** column. Timings update during execution and freeze
  after completion/cancellation. They clear when the graph changes or runs again.
- Native collapsed-component progress aggregates earliest invocation start and
  latest terminal end; labels report body wall spans, including gaps, rather than
  adding overlapping step durations. Individual steps remain in Process Gantt.
- Browser graph nodes display matching invocation milliseconds from worker timing
  messages. Obsolete results are hidden on the graph while Gantt remains labelled.
- No profiling fields are persisted, and there are no new dependencies or copied
  legacy profiling sources. [Timing guide](../../GANTT_ANALYSIS.md) describes the
  meaning and difference from legacy queue-inclusive total-time statistics.

## Verification

- Release CMake build passed with the documented direct `cmake -E cmake_autogen`
  workaround after nested Qt moc process restrictions.
- Native timing UI tests assert labels on every node, the results duration column,
  clear-on-edit behavior and collapsed-component wall-span aggregation. They
  produce `build/native/native/execution-node-times-smoke.png` for visual inspection.
- `npm.cmd run check` passed: formatting, TypeScript, 15 web, 45 core, four SDK and
  five runtime tests, and the production build.
- Production browser suite: 17 passed, one intentional development-only worker
  probe skipped; includes all four native file exchanges and matching graph/Gantt
  duration assertions for scene/data/components. Used the prior ignored
  `build/web-tests/gantt.config.ts` on port 4174, with native exchange and Qt paths
  set, because port 4173 was occupied. Initial graph-label assertions missed nodes
  outside restored viewports; tests now explicitly use Fit graph before inspecting.
- The native component layout regression caught overlap from the initial wide
  labels. Compact 80-pixel labels with wrapping preserve the shipped graph spacing.
- Visually inspected native node labels/results and the browser data-chain render.
  Native offscreen rendering used `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf`.
- Final `$env:SMARTFLOW_TEST_FONT = 'C:/Windows/Fonts/segoeui.ttf'; ctest --preset windows-local-release-tests --parallel 4`:
  all 24 entries passed, including the component non-overlap check.
- CMake install refreshed `build/desktop-test`; `scripts/Test-DesktopPackage.ps1`
  passed all ten packaged startup/render configurations and matching executable hashes.
- `git diff --check` and the final formatting check passed.

## Decisions and open questions

Durations are invocation wall time excluding queue waiting. Component labels show
their overall body span. They are not CPU timings, critical-path percentages or
persisted graph parameters. Full packaged manual desktop/clean-machine acceptance
remains pending; automatic Qt checks and startup do not replace it.

## Next steps

Press Run to see durations directly on each node; no Gantt widget needs to be open.
The updated native package remains `build/desktop-test`. No further implementation
is required for this checkpoint.
