# Checkpoint N5g: native workspace persistence

## Objective and scope

Continue the desktop checkpoint with saved canvas/viewer workspace state.
The independent non-3D extension remains the next separate N5 step.

## Current state

- Save/Open restores positions by retained node ID, canvas scale/center, multiple
  node selection, pinned output and opaque extension viewer state per graph.
- OutputViewer exposes state key, capture/restore and a change callback. The host
  remains domain-independent; the included scene extension validates/restores
  yaw, pitch, zoom span, target and object selection.
- Workspace changes update dirty tracking without incrementing semantic revision,
  running the graph or adding semantic undo commands. Movement retains its
  existing canvas undo. Save and close/Open prompts capture pending UI state.
- Unknown workspace keys, other graph/viewer entries, missing-node positions and
  opaque fields within owned objects survive merging. Malformed state uses safe
  defaults; unrecognized future object shapes are retained.
- Open suppresses capture and cancels earlier queued captures. First display
  establishes a clean view baseline, preventing a spurious startup save prompt.
- Files lacking editor state keep terminal-output pinning and canvas fitting.
  See [decision 0010](../../architecture/decisions/0010-native-workspace-persistence.md).

## Verification

- Release CMake build passed using `cmake --build build/native --config Release
--parallel 8`. Applied the documented direct `cmake -E cmake_autogen` workaround
  for document history, workspace, workspace tests and scene tests after nested
  moc process-spawn restrictions. Build and tests used process permission.
- All eight CTest checks passed with Qt on PATH and optional workspace-test font:
  `ctest --test-dir build/native -C Release --output-on-failure`. Includes both
  offscreen startup checks, fresh-window scene save/reopen, camera/selection,
  stable-ID positions, pinned-output restore, malformed viewer defaults, opaque
  workspace retention and semantic undo isolation. Initial startup timeouts
  exposed the first-display baseline issue; fixed and reran successfully.
- `cmake --install build/native --config Release --prefix "$PWD/build/desktop-test"`
  passed. Qt deployment retained the existing optional DX12 compiler warning.
- Packaged Windows startup returned 0 with PATH restricted to Windows directories
  and Qt plugin/platform overrides cleared. Inspected the native rendered image
  `build/native/n5g-packaged-smoke.png`: all four nodes, inspector, output list and
  blue scene preview are visible with the updated save hint.
- `npm.cmd run check` passed: formatting, TypeScript, nine persistence tests and
  production build. `git diff --check` passed.
- Real desktop mouse/dialog save/reopen was not repeated in this session. The
  new persistence interactions are covered through Qt UI/behavior tests; the
  packaged check validates startup/rendering on this machine, not a clean machine.

## Decisions and open questions

No schema-envelope or package-boundary changes, downloads, delegation or vendor
edits. No production plugin system is claimed. Canvas center restoration has Qt
scrollbar pixel rounding and depends on viewport size. Splitters/window geometry
are not saved. Scene selection uses a bounded output object index; durable
selection across output reorder is future work. Capture currently favors
correctness over large-document performance.

## Next steps

1. Implement the separate non-3D text/table extension through native contracts,
   with an app mode that does not initialize or link scene-domain workflow code.
2. Validate both demonstration workflows in the packaged desktop app, including
   workspace restore and save/reopen interaction; record limitations accurately.
3. Record full N5 only after remaining acceptance checks. Commit each verified
   checkpoint separately. Reusable graph components and production rendering
   remain later work.
