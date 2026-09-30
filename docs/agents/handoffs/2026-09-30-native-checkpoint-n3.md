# Checkpoint N3: native workspace, inspector and execution

## Objective and scope

Finish the existing N3 working tree, connect the canvas/inspector to the N2
executor, verify parameter edits and undo, and record a separate checkpoint.
Preserve the old repository. Required 3D remains the next migration checkpoint.

## Current state

- Composed `native/app/workspace`: semantic graph state, stable canvas bindings,
  explicit parameter commands, shared undo stack, inspector and result panel.
- Number (41) connected to Add (1) executes to 42 on startup. Users can create,
  connect, disconnect, delete, move, inspect, and edit nodes with undo/redo.
- Reused eight unchanged widget source/header/license files. Hashes verified
  against the source and manifest. The copied module notice is proprietary and
  confidential, not MIT; retained verbatim. See the
  [widget audit](../../../native/vendor/pipeline-widgets/README.md).
- Execution stays on the N2 worker. Live edits coalesce; manual runs and Cancel
  are available. Both graph revision and request generation gate publication.
  Edits clear obsolete results; canvas movement leaves semantic revision alone.
- Undo snapshots retain step/output identities, parameter metadata and unknown
  parameters. These are internal snapshots, not public project persistence.
- Fixed a missing collection include, rejected incompatible parameter value
  variants, and fixed a scene selection callback during workspace destruction.
- Roadmap, architecture and native development documentation updated.

## Verification

- Configured with `cmake -S . -B build/native -G "Visual Studio 17 2022" -A x64
-DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/msvc2022_64`: passed.
- `cmake --build build/native --config Release --parallel 8`: passed.
- Nested Qt moc launches initially failed with `operation not permitted`.
  Direct `cmake -E cmake_autogen .../AutogenInfo.json Release` with process
  permission for widget/workspace/test targets, then rebuilding, succeeded.
- `ctest --test-dir build/native -C Release --output-on-failure`: all three
  suites passed. Workspace XML reports seven entries including setup/cleanup,
  zero failures: parameter validation/unknown metadata, actual inspector Apply
  and downstream results with undo/redo, delete restoration, create/disconnect/
  movement commands, stale completion/cancellation/manual/live execution.
- Initial workspace tests found the shutdown crash; fixed and rerun successfully.
- Offscreen startup with `--smoke-test -platform offscreen --smoke-font
C:/Windows/Fonts/segoeui.ttf --screenshot build/native/n3-smoke.png`: exit 0,
  confirmed by waiting for the GUI process. Screenshot inspected: readable
  controls and canvas labels, selected Number inspector, results 41 and 42,
  Complete status. Font path is only a test argument, not a build/runtime dependency.
- `npm.cmd run check` with Node 24.19.0/npm 11.17.0: formatting, type checks,
  eight persistence tests, and production build passed with process permission.
  Initial restricted run reached tests but failed to spawn the Node test child.
- Old checkout HEAD remains `d6f457e8415445d00eaf7e9fbf9e5047768ea637` and its
  only status entry is the pre-existing untracked
  `documentation/VisualWorkspaceProposal.md`. No old checkout writes performed.
- No old checkout paths found in application, test, or root/native build sources.

## Decisions and open questions

See [decision 0005](../../architecture/decisions/0005-native-workspace.md).
The numeric workspace is a migration fixture, not a production plugin system.
No native project file format, 3D viewer, clipboard import, progress streaming,
cache or selective recomputation is provided. Cancellation remains cooperative;
shutdown can wait for uncooperative delegates. Distribution licensing remains
unresolved, including the copied proprietary widget notice.

Offscreen Qt interaction tests and rendering do not establish real desktop input,
GPU behavior, default desktop font discovery, installer behavior or portability.
Only Windows/MSVC was built. The explicit test font resolves offscreen readability;
normal desktop rendering still needs interactive verification.

## Next steps

N4: audit and copy required 3D modules and dependencies, preserving notices;
implement/test primitive, transform, material and viewer behavior through the
included scene-3d extension. Then N5: native project persistence with unknown
extension preservation and a separate non-3D workflow through platform contracts.
