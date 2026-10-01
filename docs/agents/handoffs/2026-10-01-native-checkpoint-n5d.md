# Checkpoint N5d: retained-document undo commands

## Objective and scope

Continue N5 toward editor Save/Open from N5c (`b63a24b`). Inspection found that
the vendor canvas undo commands store only connection endpoint IDs. Wiring file
documents into those commands would lose connection metadata and opaque incident
edges. This checkpoint implements the document undo prerequisite independently;
the editor model and canvas have not yet been switched. Full N5 is incomplete.

## Current state

- `native/app/project/DocumentHistory.h/.cpp` owns a retained session and Qt undo
  stack, with node creation/deletion, parameter edits and connection commands.
- Undo retains complete semantic DOM snapshots, preserving unknown node/edge
  fields, incident edges, other graphs, assets and arbitrary 64-bit integers.
  No Qt JSON conversion or legacy binary snapshot loader is used.
- Edits validate a candidate before pushing a command. Invalid operations and
  no-ops preserve redo history, clean state, active projection and revision.
- Undo/redo preserves the current workspace, recompiles the semantic projection,
  increments a monotonic revision and emits `changed`. Workspace edits emit
  `workspaceChanged` without recompiling or invalidating semantic execution.
- Document replacement prepares a candidate before switching selected graph and
  clearing history. Failed replacement leaves the current document/history intact.
- Four new behavior cases in `DocumentSessionTests.cpp` cover exact opaque-content
  restoration, stable connections, execution after undo, file round trips, redo
  branching, rejected commands, workspace separation and document replacement.
- New target `smartflow_document_history` links the existing session and Qt Gui.
  The application still uses `GraphProject` and vendor canvas undo commands.
  No vendor/old-checkout edits, new downloads or UI changes.
- Fixed existing Prettier wrapping in the N5c handoff, found by the required check.

## Verification

- `cmake --build build/native --config Release --parallel 8`: passed with process
  permission after the documented direct `cmake -E cmake_autogen` workaround for
  `smartflow_document_history` and `document_session_tests`. Nested moc initially
  reported `libuv process spawn failed: operation not permitted`.
- `ctest --test-dir build/native -C Release --output-on-failure`: all six suites
  passed with Qt bin on PATH and permission for temporary-file operations.
  Document-session report: 16 entries (14 behavior cases plus setup/cleanup),
  zero failures/errors/skips.
- `npm.cmd run check`: formatting, typechecks, nine persistence tests and build
  passed. `git diff --check`: passed.
- No UI changes or new interactive verification. Existing offscreen workspace
  and scene suites passed. Editor Save/Open is not implemented or verified.

## Decisions and open questions

See [decision 0007](../../architecture/decisions/0007-native-project-files.md).
Registrations must remain immutable. Complete snapshots favor exact restoration;
history memory limits/compaction remain future work. Undo-stack clean state covers
semantic changes only, so future file dirty tracking must include workspace edits.
Known-node display access while execution is blocked still needs a deliberately
non-executable projection API. Do not weaken `executableGraph()` diagnostics.

## Next steps

1. Make GraphProject/session composition use DocumentHistory as the authoritative
   model. Preserve the semantic revision/generation gates in ExecutionController.
2. Replace/adapt vendor canvas commands so graph edits use the document history.
   Rebuild/synchronize the canvas after semantic undo without recursively issuing
   document edits. Keep workspace movement on an appropriate separate command path.
   Do not combine endpoint-only vendor undo with persisted edge metadata.
3. Display unsupported node placeholders, retaining unresolved edges in the source
   even when they cannot be drawn. Gate execution on session diagnostics.
4. Add Open/Save/Save As and dirty state, then viewer/canvas workspace persistence.
   Test actual editor edit/save/reopen and undo with unavailable extension data.
5. Add the separate non-3D extension; the numeric fixture is not that acceptance.
6. Run native build/CTest/npm checks and offscreen visual checks for UI changes,
   record and commit each verified step. Mark full N5 only after acceptance passes.

Environment: PowerShell, Qt 6.11.1 MSVC2022, `build/native`, Node 24/npm 11.
Use `npm.cmd`; Qt bin is `C:/Qt/6.11.1/msvc2022_64/bin`. No agent delegation is
authorized. See N5b/N5c for the retained session and structural command details.
