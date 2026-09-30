# Checkpoint N5a: native project-file foundation and session handoff

## Objective and scope

Continue N5 carefully within the remaining session context. The user explicitly
requested a durable record for the next session before tokens run low. Complete
the standalone native project-file layer as a bounded checkpoint; do not claim
N5 or editor save/open is complete.

## Current state

- Previous checkpoint: N4, commit `1678980`, bundled native 3D primitive preview.
- Cancellation prerequisite fix committed separately as `7e8388e`.
- `native/app/project/ProjectFile.h/.cpp` provides registry-independent schema-v1
  creation, validation, JSON parsing/serialization, read and atomic write.
- Uses the existing schema from `packages/core/src/index.ts` and decision 0002.
  Full JSON DOM retained; unknown fields and unresolved connections are preserved.
- Native writes validate/serialize before opening QSaveFile; direct-write fallback
  is disabled. No editor/application state is changed by this library.
- Explicit file/depth/numeric limits and duplicate-key rejection avoid silent
  truncation or integer conversion. Read decision 0007 for precise behavior.
- `smartflow_project` links Qt Core and the existing JSON headers only. It does
  not link the pipeline, canvas, widget, or scene libraries.
- Native tests exercise real filesystem round trips and failed saves. A shared
  fixture in `tests/fixtures/project-v1.json` is also tested by the TypeScript core.
- The current application still has no Save/Open commands. No UI changed in N5a.

## Verification

- `cmake -S . -B build/native -G "Visual Studio 17 2022" -A x64
-DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/msvc2022_64`: passed.
- Direct `cmake -E cmake_autogen` for the new project test target, followed by
  `cmake --build build/native --config Release --parallel 8`: passed.
- `ctest --test-dir build/native -C Release --output-on-failure`: all five native
  suites passed after the separate cancellation follow-up described below.
- Project XML reports eight entries including setup/cleanup, zero failures:
  shared fixture/filesystem round trip, empty project, malformed/unsupported
  documents, failed-save preservation, numeric precision and resource limits.
  Report: `build/native/native/project-results.xml`.
- `npm.cmd run check`: passed formatting, workspace type checks, all nine
  persistence tests (including the shared fixture) and production build.
- `git diff --check`: passed. No vendor source or old checkout edits.
- No native UI changed. Workspace/scene regression suites passed; no new
  interactive or rendering verification is claimed for this file-layer checkpoint.

The first full run exposed an existing cooperative-cancellation defect: the
copied parallel progress wrapper read a cached root stop flag without polling
its cancellation callback. Fixed independently in the execution adapter and
strengthened the regression to require delegate observation. See
[cancellation follow-up](2026-09-30-cancellation-followup.md). Do not dismiss the
initial timeout as a random test failure or reintroduce the old watchdog-only check.

## Decisions and open questions

See [decision 0007](../../architecture/decisions/0007-native-project-files.md).
Native JSON limits are 16 MiB and 128 containers. Native integer values can exceed
JavaScript's exact-number range; portable files should stay within that range.
No schema-version bump was introduced. Do not mistake unknown-content retention
in the codec for completed unknown-node support in the editor/runtime adapter.

## Next steps for the next session

1. Read this handoff, README, ROADMAP, decisions 0002/0005/0006/0007, current
   `native/sdk/WorkspaceExtension.h`, `GraphProject`, `PipelineCanvas`,
   `ExecutionController` and `WorkspaceWindow`. Inspect Git status first.
2. Add explicit persisted package ID, type ID and node contract version to native
   registration metadata. Current native delegates use combined IDs such as
   `smartflow.scene-3d.cube@1`; presentation labels are not persistence identities.
3. Build a native document/session adapter retaining the original DOM. Map graph
   IDs, stable node IDs, explicit connection IDs and stable port names. Keep
   unknown nodes/parameters/fields, disconnected endpoints and unsupported data
   intact. Overlay only fields actually changed; do not rebuild the file solely
   from currently supported canvas nodes. Preserve unedited graphs and assets.
4. Loading must validate and construct a candidate before replacing active state.
   `GraphProject::restore` currently accepts trusted undo snapshots only and rejects
   missing delegates; it is not the file loader. `PipelineCanvas::loadNode` assumes
   a registered delegate. These need deliberate adapters/placeholder handling,
   not direct calls on file-provided snapshots.
5. Missing extensions must be visible and block affected execution while preserving
   content. The N2 executor can report unknown delegates, but the editor cannot yet
   represent them. Keep version mismatch and unknown parameter semantics explicit.
6. Add Open/Save/Save As and dirty/undo integration. Cancel/invalidate active work
   when swapping documents; revision plus generation must still gate results.
   Handle failed loads/saves without losing the existing graph or dirty state.
7. Save workspace canvas positions, pin/selection and scene camera separately from
   project semantics, via generic viewer save/restore hooks. Never persist runtime
   outputs. Current OutputViewer only has present/describe; SceneViewer camera is
   internal. Test graph/parameter/view state through real save/reopen.
8. Add a separate non-3D data/text extension using the same native contracts.
   Existing `--numeric` is an application fixture and does not complete this proof.
9. Run native build/CTest, npm checks, offscreen startup and rendering inspection
   for UI changes. Record and commit each completed verified step separately.
   Update ROADMAP and report commit hashes. Keep N5 pending until all required
   persistence, unknown-extension and non-3D acceptance behavior is verified.

## Environment reminders

- Workspace: `D:/dev/projects/SmartFlow`; PowerShell; Qt 6.11.1 MSVC2022 kit at
  `C:/Qt/6.11.1/msvc2022_64`; existing CMake build directory `build/native`.
- Native process launch often needs tool process permission. If nested Qt moc
  reports `operation not permitted`, run its reported `cmake -E cmake_autogen
.../AutogenInfo.json Release` directly, then rebuild. Do not alter sources to
  work around this environment restriction.
- Set Qt bin on process PATH for CTest/application launches. Offscreen smoke can
  use `--smoke-font C:/Windows/Fonts/segoeui.ttf`; wait for the GUI process to
  obtain its actual exit code. Previous screenshots are under `build/native`.
- Node 24.19.0/npm 11.17.0; use `npm.cmd`. Test/build subprocesses need permission.
- Never modify the old checkout. Its last recorded HEAD is
  `d6f457e8415445d00eaf7e9fbf9e5047768ea637`, with only the pre-existing untracked
  `documentation/VisualWorkspaceProposal.md`. N5a requires no access to it.
- No agent delegation is authorized by current instructions. Do not spawn agents.
