# Task handoff: desktop and browser process Gantt

## Objective and scope

Review the old repository's time-process Gantt and integrate analysis in both
native desktop and browser/web mode. Keep old sources unchanged, platform state
independent of domains, profiling transient and desktop delivery primary.

## Current state

- Read-only legacy review and independent implementation recorded in
  [Gantt analysis](../../GANTT_ANALYSIS.md). No legacy Gantt source or dependencies
  were copied. Qt item painting and SVG use the current execution adapters.
- Native synchronized progress has monotonic ready/start/end timestamps and a
  frozen final extent. Selectable Process Gantt shows compiled body steps and
  selects their owning visible nodes. Existing workspace layout persistence
  includes its placement; unknown/unsupported graphs clear the chart safely.
- Browser worker/client messages carry per-node timing samples through existing
  run/revision gates. Timings survive cancellation with an observed endpoint;
  obsolete results are labelled. Whole components have one span. Timings never
  enter project JSON. UI uses 100-row pages and batched progress copies so the
  3000-node responsiveness/cancellation test remains usable.
- Refreshed `build/desktop-test` includes both native executables and documentation.
  User guide, execution/workspace docs, roadmap and test sheet describe access.

## Verification

- `cmake --build --preset windows-local-release`: passed. Nested Qt code generation
  hit the documented `libuv process spawn failed` restriction; direct
  `cmake -E cmake_autogen .../AutogenInfo.json Release` resolved it.
- `$env:SMARTFLOW_TEST_FONT = 'C:/Windows/Fonts/segoeui.ttf'; ctest --preset windows-local-release-tests --parallel 4`:
  all 24 entries passed, including offscreen startup, timing order, frozen final
  values, node selection, unsupported-content regression and rendered Gantt rows.
- `npm.cmd run check`: formatting, TypeScript, 15 web tests, 45 core tests,
  four SDK tests, five runtime tests and production build passed.
- Production Chromium suite with the native exchange adapter: 17 passed, one
  intentional development-only raw-worker probe skipped. All four fresh native
  file exchanges passed. An existing preview occupied port 4173; a temporary ignored
  `build/web-tests/gantt.config.ts` reused the repository config on strict port 4174,
  with absolute test/output directories. Command:
  `node tools/browser-tests.mjs --production test --config build/web-tests/gantt.config.ts`.
  Set `SMARTFLOW_NATIVE_EXCHANGE` to the built `file_exchange_check.exe` and
  `SMARTFLOW_QT_ROOT` to the Qt kit before reproduction.
- `cmake --install build/native --config Release --prefix D:/dev/projects/SmartFlow/build/desktop-test`
  and `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/Test-DesktopPackage.ps1`:
  both passed; ten packaged startup configurations and executable hashes matched.
- Visually inspected `build/native/native/execution-gantt-smoke.png` with an explicit
  Windows font and production `gantt-data.png`/scene/component render artifacts
  under `build/web-tests/results`. Row height was corrected after initial rendering.
- Initial browser large-run responsiveness and unsupported-content exchange failed;
  pagination/batched copies and the unsupported-projection guard fixed them.
  Final complete suites passed. `git diff --check` passed.

## Decisions and open questions

See [timing semantics and legacy audit](../../GANTT_ANALYSIS.md). Invocation timings
include adapter costs; component granularity and execution scheduling differ
between backends. There is no embedded Qt WebEngine mode, new plugin system,
calendar scheduling, dependency-arrow renderer, wheel zoom or trace export.
Native packaged manual interaction/clean-machine acceptance remains pending;
offscreen UI tests and packaged startup do not establish it.

## Next steps

Manual testing can select Process Gantt in a desktop region, inspect scene/data/
component runs, click rows, move/split the widget and verify placement after Save/
relaunch/Open. Timings require a fresh Run after reopening. No required
implementation work remains for this checkpoint.
