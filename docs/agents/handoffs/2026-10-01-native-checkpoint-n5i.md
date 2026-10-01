# Checkpoint N5i: shipped projects and packaged launch verification

## Objective and scope

Continue N5h with packaged saved-project verification and useful test examples.
Real desktop interaction could not proceed because the Windows computer-use
helper native pipe was unavailable; do not mark full N5 complete from smoke tests.

## Current state

- Added `examples/scene.smartflow` and `examples/data.smartflow`, installed into
  the desktop test folder. They include explicit package identities, deterministic
  node/edge IDs, parameters, positions, selection, pinning and viewer state.
- `--project <path>` opens a document after initial window layout. Missing paths
  or unreadable files report to stderr and exit 6. Smoke mode executes the loaded
  graph, with failure exit 5, rather than silently using the preset.
- The scene example restores Cube size 3, yaw 60/pitch 15 and selected object.
  The data example restores minimum 30, count 2/total 79/mean 39.5 and Total row.
- Fresh-process startup exposed an unwanted close prompt for untouched loaded
  files. Close/Open confirmation now flushes only pending UI events, leaving
  retained navigation precision untouched when Qt rounds scrollbar centers.
- A CMake launch-error check requires exact exit 6 and the expected stderr
  diagnostic for missing files and missing arguments, not merely any failure.

## Verification

- CMake configure, Release build and absolute-prefix install passed using the
  commands in [native development](../../NATIVE_DEVELOPMENT.md). Direct scene/data
  test autogen was run before the initial build to avoid known nested moc issues.
- All fourteen CTest checks passed, including both saved-example startup
  processes, exact launch-error codes, native scene/data example save/reopen,
  existing opaque-content/dirty-state/undo checks and independent data tests.
  Initial saved-example startup checks timed out on the close prompt; fixed
  and reran the entire suite successfully. Initial Qt logging was not captured
  in Windows GUI subprocess stderr; explicit stderr reporting fixed that check.
- Both freshly installed native executables opened their installed example files
  and returned 0 in smoke mode with PATH limited to Windows directories and all
  Qt plugin/platform overrides cleared. Inspected
  `build/native/n5i-packaged-scene.png` and `build/native/n5i-packaged-data.png`:
  restored parameters, all nodes, scene camera/object highlight and data results
  are visible. The task-created hung scene smoke process from the earlier run
  was identified by exact path/command line and closed before reinstalling.
- `npm.cmd run check` passed: formatting, typechecks, nine persistence tests and
  production build. `git diff --check` passed.
- The computer-use skill was read and initialized. `sky.list_apps()` failed with
  `Computer Use native pipe is unavailable: failed to connect native pipe: The
system cannot find the file specified. (os error 2)`. Retry and session reset
  failed unchanged. No app input or custom automation fallback was attempted.
  Manual desktop file-dialog, mouse and close-prompt acceptance is still pending.

## Decisions and open questions

These are local-machine native startup/rendering and Qt interaction regressions,
not clean-machine compatibility or complete desktop interaction acceptance.
`--project` does not infer extension configuration; choose the matching data
or scene executable/mode. No plugin loader, production renderer, public installer
or file association is added. 3D remains the primary included workflow.

## Next steps

1. When the native computer-use helper is restored, verify packaged scene and
   data editing, undo/redo, viewer interaction, native Save As, close/relaunch
   and native Open. Include Save/Discard/Cancel prompt branches and view restore.
2. Record full N5 only after those desktop acceptance checks pass. Preserve the
   distinction between automated startup and real desktop interaction evidence.
3. Design reusable graph components as a separate native checkpoint; do not
   infer a production plugin system or alter the old repository.
