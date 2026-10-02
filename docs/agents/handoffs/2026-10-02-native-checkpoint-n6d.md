# Task handoff: N6d collapsed native component instances

## Objective and scope

Continue the native migration from N6c with one visible component node, exposed
controls/ports and coherent execution/viewer results. Keep the platform independent
of domains, retain the required scene extension and prove the independent data app.

## Current state

- Instances retain an immutable definition snapshot and separate exposed values.
  Compilation expands a disposable graph; canvas/inspector facades show one node.
- Typed connections, chained instances, numeric control edits, delete, undo/redo
  and save/reopen use retained commands. Unknown or malformed instances remain
  retained and block execution. Nested definitions are explicitly unsupported.
- Worker result groups share exposed members without renaming them. Dependency
  barriers gate every body branch on external inputs and every external consumer
  on the complete body. Failed private branches cannot leak successful outputs;
  cancelled runs publish no partial results. Visible and runtime DAGs are checked.
- Library insertion defaults to **Insert as one node**; unchecking preserves N6c
  expanded insertion. The API keeps expanded insertion as its default argument.
- The toolbar selects individual output aliases for pinning. Workspace saves
  selected/pinned aliases separately from project controls.
- `examples/components.smartflow` demonstrates rows/summary outputs and minimum
  control in the separate data-only executable. Scene tests verify the same
  collapsed mechanism with cube size, viewer output and file round trips.
- No vendor edits, new dependencies, old-repository edits or runtime references.

## Verification

- `cmake --build build/native --config Release -j 4`: passed, Qt 6.11.1/MSVC.
- With Qt `bin` on PATH and `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf`,
  `ctest --test-dir build/native -C Release --output-on-failure`: 17/17 passed.
  Reports include component, UI, scene and pipeline JUnit results under
  `build/native/native/`. The pipeline report confirms the failed-unexposed-branch
  regression test ran. The scene test was rebuilt after its font hook was added.
- `npm.cmd run check`: formatting, type checking, nine persistence tests and build
  passed. Documentation uses the repository formatter.
- `cmake --install build/native --config Release --prefix
D:/dev/projects/SmartFlow/build/desktop-test`: passed. Qt reports its existing
  optional DX12 compiler warning; current primitive preview does not use DX12.
- Packaged `smartflow.exe` scene and `smartflow-data.exe` data/component examples
  ran via `Start-Process -Wait` with the Windows platform and exited 0. Screenshots
  under `build/native/n6d-*-packaged.png`; the component screenshot was inspected
  for visible ports/control, summary pinning, total 79 and unclipped layout.
- Offscreen component/scene rendering was inspected. Offscreen checks and packaged
  startup do not establish full desktop interaction acceptance. The preceding
  session reported the native computer-use pipe unavailable (OS error 2); no
  mouse-driven Windows interaction acceptance is claimed here. Full N5 remains
  pending, including actual packaged edit/undo/save/relaunch/Open interaction.
- An initial CTest invocation without Qt on PATH failed to start executables;
  corrected environment passed. Qt deployment requires an absolute install prefix.

## Decisions and open questions

See [decision 0013](../../architecture/decisions/0013-collapsed-native-components.md)
and [the component guide](../../GRAPH_COMPONENTS.md). This is a native migration
preview, not a production runtime or dynamically loaded plugin system. Snapshot
updates, nested components, definition editing/migration and library deletion/export
remain unimplemented. Market, audience and distribution remain open.

## Next steps

1. Perform packaged desktop interaction acceptance for scene, data and component
   workflows when the native computer-use surface is available; use the desktop
   guide and record results without marking N5 complete beforehand.
2. Define the next component-library scope (editing/export/update semantics) in
   a separate checkpoint; retain snapshot reproducibility and unknown content.
3. Keep broader scene rendering and public installer/distribution work separate.
