# Checkpoint N2: copied pipeline foundation and background execution

## Objective and scope

Continue the approved native migration by auditing and copying the pipeline,
data, and task dependencies, then testing node/port and execution behavior.
Preserve the old checkout and all prior SmartFlow work.

## Current state

- Copied 186 selected source/header/license files from nine legacy modules to
  `native/vendor/legacy`. SHA-256 provenance and all module/file notices retained.
- New explicit CMake source list builds an independent `smartflow_legacy` static
  library. No copied machine settings, binaries, assets, or original build scripts.
- `native/app/pipeline/PipelineExecution` snapshots the graph on the owner thread
  and executes on a single task-queue worker. Registries/factories are held by
  shared ownership; callers must leave them immutable throughout execution.
- Validates identities, delegate availability, port mappings, input producers,
  connection types and cycles. Rejects unsupported legacy execution overrides.
- Handles delegate exceptions/failures, validates outputs, skips failed consumers
  and permits independent branches to finish. Cancellation discards run outputs.
- Recomputes each submitted snapshot; prior results remain separate. No cache,
  selective invalidation, application result publication, or project mutation.
- Patched only the copied PipelineManager to retain disabled fixup across
  restarts/callbacks and avoid pruning dangling inputs in that mode.
- `native/tests/PipelineTests.cpp` exercises execution/recomputation, ten invalid
  graph cases, four failure modes, worker/snapshot isolation, running and queued
  cancellation, independent branches, and the no-fixup regression.
- N1 numeric canvas remains the application UI. N3 will connect the pipeline and
  migrate inspector/workspace editing and undo. Required 3D remains N4.

## Verification

- CMake configured with VS2022 x64, CMake 3.23, Qt 6.11.1 MSVC kit.
- `cmake --build build/native --config Release --parallel 8`: passed.
- `ctest --test-dir build/native -C Release --output-on-failure`: both suites
  passed. Pipeline XML reports 22 passed entries, including setup/cleanup, with
  zero failures. Report: `build/native/native/pipeline-results.xml`.
- Compiler child-process permissions were required. Nested Qt moc invocation hit
  `operation not permitted`; running the generated `cmake -E cmake_autogen`
  command directly with process permission, then rebuilding, succeeded.
- Initial execution fixtures failed because `setParameterValue` on a nonexistent
  legacy parameter leaves its name unset. Fixtures now create named parameters
  through `setParamerter`; no vendor workaround was introduced for this API.
- All 186 imported source hashes verified against the source checkout. Only the
  documented PipelineManager adaptation differs in SmartFlow.
- `npm.cmd run check`: passed formatting, all workspace typechecks, eight
  persistence tests, and the Vite production build.
- `smartflow.exe --smoke-test -platform offscreen --screenshot ...`: exit 0.
  Inspected `build/native/n2-smoke.png` and `n2-smoke-fonts.png`: the canvas,
  nodes, and widgets render, but text is missing-font boxes in this environment,
  including a retry with process permission. Readable text rendering is not
  verified; check it on a real desktop during N3.
- Old checkout HEAD remains `d6f457e8415445d00eaf7e9fbf9e5047768ea637`; its only
  status entry remains the pre-existing untracked
  `documentation/VisualWorkspaceProposal.md`. No writes targeted it.
- Verified current imported hashes and checked build/app/test sources for old
  checkout paths: none are referenced. Provenance documentation intentionally
  records the historical source location.

## Decisions and open questions

See [decision 0004](../../architecture/decisions/0004-native-pipeline-migration.md)
and the [vendor audit](../../../native/vendor/legacy/README.md).

The selected legacy code is a migration foundation, not a production runtime,
extension loader or stable project format. N5 must implement native persisted
unknown-extension preservation. Long-running delegates must poll cancellation;
queue teardown can wait for uncooperative delegates. Future UI publication must
reject obsolete project revisions. All inputs are currently required.

No native UI changed. Offscreen verification does not establish real desktop
input, GPU interaction, or standalone distribution behavior. Only Windows/MSVC
was built here; cross-platform portability of this source subset is unverified.

## Next steps

N3: audit/copy the pipeline inspector/workspace, introduce explicit project
commands and undo, connect canvas edits to worker execution, and reject stale
results by project revision. Verify edits, undo, execution feedback and rendering.
Then N4: required 3D package; N5: native persistence and a separate non-3D workflow.
