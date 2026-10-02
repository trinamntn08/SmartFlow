# Task handoff: N6e standalone component import/export

## Objective and scope

Continue after N6d by checking packaged desktop interaction availability, then
implement the next reusable-component checkpoint while that verification is
unavailable. Keep native C++/Qt primary and domain-independent, with required
scene support and independent data validation.

## Current state

- The native computer-use skill was read and `sky.list_windows()` retried. It
  returned: native pipe unavailable, file not found (OS error 2). Full N5 desktop
  interaction acceptance remains open; no unsupported fallback automation used.
- Component library now has Import/Export file buttons. Import works in an empty
  library, adds only a catalog entry, and uses the existing undoable command.
  Identical imports are no-ops; same-identity/version conflicts reject before
  changing project state or redo history. Existing snapshots are untouched.
- Export writes one existing schema-v1 definition as `.smartflow-component` or
  `.json`. It exports definition defaults and opaque fields, not current instance
  edits, project bindings, workspace state or external assets.
- Structurally valid definitions can be exchanged without installed node packages.
  Unavailable components stay retained and cannot be inserted. Unsupported/future
  definition formats reject; malformed catalogs remain preserved in project files.
- ProjectFile shares schema-independent strict `jsonFile` transport with the new
  ComponentFile adapter. Project entry points still validate the project schema.
  Duplicate keys, integer overflow, excessive nesting and size reject. Writes are
  atomic with direct-write fallback disabled. No new dependencies or vendor edits.
- File-dialog cancellation changes nothing; errors appear inline. Closing the
  library preserves imports already completed; Undo reverses them. Stale dialogs
  reject before catalog/file mutation.
- `examples/filtered-summary.smartflow-component` ships with the desktop package
  and can be imported into another data project. Required scene functionality is
  unchanged and covered by the full suite.

## Verification

- `cmake --build build/native --config Release -j 4`: passed with Qt 6.11.1/MSVC.
- Targeted component/UI/project checks: 4/4 passed. New tests cover standalone
  round trips, opaque 64-bit values, unavailable packages, failed/oversized writes
  preserving existing files, malformed/oversized/deep JSON, cross-project reuse,
  real Qt file-dialog buttons, cancellation, inline errors, conflicts, undo/redo,
  snapshot isolation and save/reopen.
- With Qt `bin` on PATH and `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf`,
  `ctest --test-dir build/native -C Release --output-on-failure`: 17/17 passed.
  JUnit reports under `build/native/native/` confirm all four new file-exchange
  tests ran. The shared project-codec and required scene regressions also passed.
- `npm.cmd run check`: formatting, type checking, nine persistence tests and
  production build passed. `git diff --check` passed.
- `cmake --install build/native --config Release --prefix
D:/dev/projects/SmartFlow/build/desktop-test`: passed and included the standalone
  example. Qt emitted its existing optional DX12 compiler warning.
- Packaged scene, independent data and collapsed-component examples ran with
  the native Windows platform via `Start-Process -Wait`, all exiting 0.
  Screenshots/logs: `build/native/n6e-*-packaged.*`. They check startup/rendering,
  not desktop import/export interaction.
- `component-file-library-smoke.png` under `build/native/native/` was inspected:
  import/export buttons and exposed bindings are visible with no clipped layout.
- Qt tests and packaged startup do not substitute for mouse-driven Windows
  edit/undo/save/relaunch/Open acceptance. That verification remains unavailable
  due to the native pipe error above. No full N5 acceptance is claimed.

## Decisions and open questions

See [decision 0014](../../architecture/decisions/0014-native-component-files.md)
and [the component guide](../../GRAPH_COMPONENTS.md). Asset bundling/relocation,
nested components, library editing/deletion, migration and live updates remain
future work. The migration still has no production runtime/plugin loader.
Audience, market and public distribution remain open.

## Next steps

1. Complete packaged desktop interaction acceptance for scene, data, collapsed
   components and file exchange when native computer-use becomes available.
2. Consider explicit library rename/deletion with undo, preserving independent
   instance snapshots; scope it as a separate checkpoint before implementation.
3. Keep asset distribution, nested component semantics and automatic snapshot
   updates separate from basic library management.
