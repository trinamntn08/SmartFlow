# Checkpoint N5b: retained documents and executable projections

## Objective and scope

Continue N5a with explicit native persistence identities and the model-side
document adapter needed before editor Save/Open can safely expose project files.
Keep a durable session handoff as requested by the user. N5 is not complete.

## Current state

- N5a commit: `0c0db83`; cancellation prerequisite fix: `7e8388e`.
- `NodePresentation` now includes explicit `packageId`, `typeId`, and
  `contractVersion`. Numeric and scene registrations supply these fields.
- `native/app/project/DocumentSession.h/.cpp` retains the complete JSON document,
  selects one graph by ID, and compiles an independent runtime snapshot using
  registered node identities and stable port names. No display-label parsing.
- Unknown nodes/versions, unsupported or missing parameters, unresolved endpoints,
  missing ports, duplicate input producers and incompatible connection types are
  retained in the source and reported as diagnostics. `executableGraph()` throws
  if these diagnostics exist, preventing partial execution that silently omits data.
- Explicit `setParameter` edits replace one JSON value in a candidate session,
  then swap on success. Unedited fields, other graphs, assets and workspace state
  survive. Invalid edits and invalid candidate construction leave the original
  session intact. A known parameter can be edited while other unknown parameters
  remain retained and continue blocking execution.
- Initial parameter codecs handle bounded double, string and bool definitions.
  Definitions come from a disposable fresh prototype, never by fixing up loaded
  node content. Loaded missing parameters are diagnosed, not defaulted.
- The legacy loader receives only a generated node ID/delegate string envelope
  because StepDetails has no ID setter. Raw file parameters/mappings/binary data
  are never passed through that loader. Transient output collection IDs can change
  between snapshots; persisted connections retain node IDs and port names.
- Application file actions, unknown-node visuals, structural editing commands,
  viewer-state persistence and the separate non-3D extension are still pending.
  The existing demo UI still owns GraphProject and is not yet backed by this session.

## Verification

- CMake configured with VS2022 x64 and Qt 6.11.1 MSVC2022 kit; build passed:
  `cmake --build build/native --config Release --parallel 8`.
- Ran direct `cmake -E cmake_autogen .../AutogenInfo.json Release` for changed
  workspace/test targets before building, as documented for this environment.
- Initial compilation exposed non-const legacy parameter accessors and the
  `tp_pipeline::randomId` namespace; corrected the adapter and rebuilt.
- `ctest --test-dir build/native -C Release --output-on-failure`: all six suites
  passed. `native_document_session` reports seven entries including setup/cleanup,
  zero failures. Five cases exercise real numeric save/reopen/execution (42 then
  10 after edit), unknown-content blocking/preservation, failed-edit isolation,
  known edits beside opaque parameters, and registration/graph selection checks.
- `native_scene` additionally serializes and reparses a complete scene graph,
  compiles it using explicit node identities, edits Cube size and verifies the
  resulting scene geometry through the existing background executor.
- `npm.cmd run check`: formatting, workspace typechecks, nine persistence tests
  and production build passed. `git diff --check`: passed.
- No visual behavior changed; existing offscreen workspace/scene tests passed.
  No new real desktop/GPU or interactive file-dialog verification is claimed.
- No vendor source or old-checkout edits. No new dependency downloads.

## Decisions and limitations

Read [decision 0007](../../architecture/decisions/0007-native-project-files.md)
and the [N5a handoff](2026-09-30-native-checkpoint-n5a.md) for the file format,
numeric/resource limits and environment setup.

Package dependency versions are preserved but not resolved. Exact node contract
versions gate registration matching. Unknown parameters conservatively block the
selected graph; no partial-branch execution is offered. Cycles and remaining
runtime rules are still validated by PipelineExecution. Delegate registrations
must remain immutable for the session lifetime. The compiled graph is a runtime
projection: editor operations must edit the retained document, not mutate its
legacy StepDetails pointers and later regenerate a lossy file.

## Next steps / exact resume point

1. Inspect Git status, then read this handoff, README/ROADMAP, decisions 0005–0007,
   DocumentSession and the current workspace classes. N5a's identity/retained-DOM
   prerequisites are now implemented; do not recreate a competing document model.
2. Add explicit document commands for node creation/deletion and connection edits,
   retaining unknown fields and using stable connection IDs. Add workspace edits
   separately. Supply an empty/new-project path without inventing a scene dependency.
3. Make the editor session authoritative. Adapt GraphProject/PipelineCanvas to
   display loaded nodes and unsupported placeholders; keep dangling/unsupported
   connections in the retained document even if they cannot be drawn normally.
   Existing trusted undo snapshot loading is not a project-file loader.
4. Wire execution to DocumentSession diagnostics and snapshots. Show unsupported
   content clearly, prevent execution while diagnostics exist, and retain N2's
   runtime validation. Document replacement must cancel/invalidate pending work
   and preserve generation/revision gating. Do not publish obsolete results.
5. Add Open/Save/Save As and dirty/undo integration. Construct and validate a
   candidate session before replacing active UI state; failed operations must
   preserve the active project and dirty state. Use ProjectFile::write for atomic
   replacement. No current application file actions exist yet.
6. Add generic viewer state save/restore hooks; preserve canvas positions, pinned
   output, selection and camera under workspace, never runtime outputs. Test actual
   edit/save/reopen and undo through the editor, including unavailable extensions.
7. Add the independent data/text extension through the same contracts. `--numeric`
   remains an application fixture, not this independent extensibility acceptance.
8. Run build/CTest/npm checks and inspect offscreen UI rendering when it changes.
   Record and commit each verified step separately; update the handoff before
   context gets tight. Mark full N5 complete only after all acceptance behavior.

Environment: PowerShell, `D:/dev/projects/SmartFlow`, `build/native`, Qt bin
`C:/Qt/6.11.1/msvc2022_64/bin`, Node 24/npm 11 (`npm.cmd`). Tools often need process
permission. Follow the documented direct-moc workaround; do not modify the old
repository or spawn agents without new authorization.
