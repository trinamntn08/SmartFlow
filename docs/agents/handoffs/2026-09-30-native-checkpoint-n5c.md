# Checkpoint N5c: retained-document structural commands

## Objective and scope

Continue toward editor Save/Open from N5b (`f8d968f`). Implement the structural
command prerequisite without rebuilding files from lossy legacy graph snapshots.
Record a durable handoff as requested by the user. Full N5 remains incomplete.

## Current state

- `DocumentSession::empty` creates a domain-independent project and selected graph.
- `createNode` uses explicit registered identities and validates encoded defaults
  on a fresh node. IDs are supplied by callers for future stable command replay.
- `removeNode` removes that node and all incident edges, including unknown ones.
  Other graphs, assets, opaque nodes/edges and metadata remain unchanged.
- `connect` uses stable named ports, requires registered compatible endpoints and
  refuses occupied inputs, including inputs occupied by unresolved loaded edges.
  `disconnect` removes a stable edge ID even when its endpoints are unavailable.
- Semantic edits share candidate construction and swap, preserving the active
  document/projection if validation fails. Duplicate/blank IDs reject atomically.
- Runtime registration IDs must be unique as well as persisted identity tuples.
- `setWorkspaceField` validates and replaces a single owned workspace field without
  compiling or replacing transient execution state. Other workspace keys survive.
- Added behavioral tests in `native/tests/DocumentSessionTests.cpp` for construction
  from empty, execution yielding 42, disk save/reopen, disconnect/reconnect,
  opaque-content retention, incident-edge removal, failed-command atomicity,
  unresolved-edge occupancy, incompatible ports, and workspace separation.
- No vendor files, old checkout, build boundaries or UI code changed. No new
  dependencies. The demo editor still uses the existing GraphProject; Save/Open,
  undo integration, unknown-node display, viewer persistence and the separate
  data/text extension remain unimplemented.

## Verification

- `cmake --build build/native --config Release --parallel 8`: passed with process
  permission and the documented direct `cmake -E cmake_autogen` workaround for
  scene/document-session tests. Restricted nested moc launches failed initially.
- `ctest --test-dir build/native -C Release --output-on-failure`: all six suites
  passed with Qt bin on PATH and permission for temporary file writes. Initial
  restricted runs failed both file-writing suites; the same build passed when
  permitted. No source changes were made to bypass file permissions.
- After adding the final opaque-connection/type-mismatch case, rebuilt successfully
  and reran `ctest --test-dir build/native -C Release --output-on-failure -R
  native_document_session`: passed. The suite now has ten behavior cases plus
  setup/cleanup.
- `npm.cmd run check`: formatting, typechecks, nine persistence tests and build
  passed. `git diff --check`: passed.
- No UI changes or new interactive verification; existing offscreen scene and
  workspace suites passed. No application Save/Open verification is claimed.

## Decisions and open questions

See [decision 0007](../../architecture/decisions/0007-native-project-files.md).
Node deletion deliberately removes incident unknown edges; a future UI undo
command must retain the complete before/after document, including these edges.
Do not use Qt JSON snapshots to store retained arbitrary 64-bit integers.
Workspace-field callers must merge nested unknown values inside their owned field.
Package versions remain unresolved and are not inferred from node contract versions.
Cycles remain the responsibility of PipelineExecution. The compiled graph remains
a disposable projection; do not mutate it and reconstruct the file from it.

## Next steps

1. Read this handoff and N5b plus README/ROADMAP; inspect Git status.
2. Make GraphProject session-backed using the new structural commands. Design undo
   around retained documents, preserving unknown values and incident edges. Keep
   workspace edits distinct from semantic revisions and execution invalidation.
3. Adapt PipelineCanvas to load known nodes and unsupported placeholders without
   treating trusted legacy undo snapshots as project-file content. Keep unresolved
   edges in the source even when they cannot be drawn.
4. Gate execution on session diagnostics; cancel/invalidate pending work during
   document replacement and retain generation/revision checks against stale output.
5. Add Open/Save/Save As and dirty state. Prepare candidate documents before changing
   the active window; failed reads/writes must preserve active document and state.
6. Add viewer workspace hooks and persist positions/pin/selection/camera, never
   transient results. Test actual editor edit/save/reopen and undo with unknown data.
7. Implement the separate non-3D extension; numeric remains an application fixture.
8. Build/test/check, inspect offscreen UI when changed, record and commit each
   verified step. Mark N5 complete only after its full acceptance criteria pass.

Environment remains PowerShell, Qt 6.11.1 MSVC2022, `build/native`, Node 24/npm 11.
Use `npm.cmd`; Qt bin is `C:/Qt/6.11.1/msvc2022_64/bin`. No agent delegation is
authorized. Read the N5b handoff for further adapter and execution details.
