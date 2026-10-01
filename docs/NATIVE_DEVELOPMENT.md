# Native development

SmartFlow now starts with C++17, Qt6 Widgets and CMake. The existing TypeScript
workspace is retained as a future extension prototype.

The first version for user testing is the Windows desktop application. Use
[desktop testing](DESKTOP_TESTING.md) to prepare and launch the app with its
runtime dependencies, and to exercise the real desktop acceptance workflow.

On Windows, use Visual Studio 2022 C++ tools and a matching Qt MSVC kit. From
the repository root (adjust the Qt installation path for your machine):

```powershell
cmake -S . -B build/native -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/msvc2022_64
cmake --build build/native --config Release --parallel 8
$env:PATH = "C:/Qt/6.11.1/msvc2022_64/bin;" + $env:PATH
ctest --test-dir build/native -C Release --output-on-failure
build/native/native/Release/smartflow.exe
```

`smartflow.exe --smoke-test -platform offscreen` starts the application and
exits automatically. It does not validate real display/GPU interaction.
The default preview starts with Cube → Transform → Material → Scene, with the
final Scene output pinned in the viewer. `smartflow.exe --numeric` instead starts
Number (41) connected to Add (1), yielding 42, without a scene registry or viewer.
Right-click or use Add node, then drag matching ports to connect. Select a node,
edit its inspector value, and click Apply. Undo/redo covers parameter edits,
creation, deletion, connections, and movement. Live updates recompute after edits;
disable them to use Run explicitly. Cancel discards the active result. Select
nodes or result rows to inspect outputs. Pin output keeps one result in the viewer
while inspecting another node. Drag the 3D preview to orbit, wheel to zoom,
double-click to frame and click to select an object. Use File > Open, Save or
Save As for project files. Canvas layout/navigation, selection, pinned output and viewer settings are saved;
clipboard import/duplication is disabled.

No source or build paths in CMake reference the old repository. Raw build outputs
need Qt on PATH; the Windows CMake install target copies runtime dependencies
into a local test folder. A public installer remains a later checkpoint.
Build outputs and personal CMake presets are ignored.

## N2 pipeline foundation

The build also produces `smartflow_legacy`, `smartflow_pipeline`, and
`pipeline_tests.exe`. CTest runs `native_pipeline` alongside the canvas test.
The pipeline suite writes `build/native/native/pipeline-results.xml`; for readable
Windows test output, run `pipeline_tests.exe -o pipeline-results.txt,txt` and read
that file. N3 connects the native workspace to the background pipeline adapter.

The adapter takes a graph snapshot and immutable delegate/factory registrations,
then returns a cancellable future. It requires initialized named parameters and
explicit typed port mappings. In the legacy API, `setParamerter` creates the
parameter; `setParameterValue` alone does not initialize its name. Long-running
delegates must poll for cancellation and must not edit the graph or UI. See
[decision 0004](architecture/decisions/0004-native-pipeline-migration.md) for
behavior and limitations, and the [vendor audit](../native/vendor/legacy/README.md)
for the source/license inventory.

## N3 workspace checks

CTest also runs `native_workspace`, covering inspector edits and recomputation,
undo/redo and identity restoration, connections, workspace-only movement,
parameter validation, cancellation, manual runs, and obsolete completion rejection.
Results are written to `build/native/native/workspace-results.xml`.

For visual verification, append `--screenshot build/native/n3-smoke.png` to the
offscreen smoke command. If system font discovery fails, also pass
`--smoke-font C:/Windows/Fonts/segoeui.ttf`. This optional font is loaded only in
smoke-test mode; no machine-specific font or path is bundled with the app.

In restricted process environments, nested Qt moc launches can fail. Run the
reported `cmake -E cmake_autogen .../AutogenInfo.json Release` directly with
process permission, then rebuild. This is an environment workaround, not a
different source configuration.

## N4 scene checks

`native_scene` checks primitive geometry, transformed bounds/normals, material and
snapshot independence, inspector-driven viewer recomputation with undo/redo,
camera/selection separation from project state, merge behavior and error/size
limits. Report: `build/native/native/scene-results.xml`. `native_workspace`
continues to link and run without the scene library.

Use `--screenshot build/native/n4-smoke.png` for the default scene smoke check.
The orthographic software preview uses fixed lighting and sorted opaque faces;
intersecting surfaces and production GPU rendering are not supported. Read the
[scene extension guide](../extensions/scene-3d/README.md) for its current scope.

## N5a project-file checks

`smartflow_project` implements registry-independent schema-v1 file operations.
`native_project` checks shared native/browser fixture compatibility, unknown
content retention, real filesystem round trips, malformed files, numeric limits,
and failed-save preservation. Report: `build/native/native/project-results.xml`.
The library is not yet connected to application Save/Open actions. Read the
[N5a handoff](agents/handoffs/2026-09-30-native-checkpoint-n5a.md) before continuing
the editor integration.

## N5b retained-document adapter

`smartflow_document_session` compiles a selected graph from the complete retained
document without dropping unknown content. `native_document_session` checks
saved numeric graph execution, exact preservation during edits, rejected edits,
unknown node/parameter/connection handling and registration validation.
`native_scene` also executes a serialized scene graph through these contracts.
Report: `build/native/native/document-session-results.xml`.
This adapter is not yet wired to application file actions or missing-node visuals.
Continue from the [N5b handoff](agents/handoffs/2026-09-30-native-checkpoint-n5b.md).

N5c extends that suite with empty-project creation, structural node/connection
edits, save/reopen execution, opaque-edge preservation, failed-command isolation,
type compatibility and workspace-only edits. Continue editor integration from the
[N5c handoff](agents/handoffs/2026-09-30-native-checkpoint-n5c.md).

N5d adds `smartflow_document_history`, a Qt undo command layer over retained
documents. The same suite now checks exact restoration of opaque nodes and
incident edges, connection identity, redo branching and rejection, execution
after undo, workspace isolation and transactional document replacement. The demo
editor still uses its earlier canvas undo stack; no file actions are exposed yet.
Continue from the [N5d handoff](agents/handoffs/2026-10-01-native-checkpoint-n5d.md).

N5e wires that command layer into the running workspace. The inspector and canvas
now edit retained documents, with exact semantic undo and workspace-only movement
undo. Unknown node types display as labeled placeholders, and unsupported graph
diagnostics block worker submission. `native_workspace` checks editor-level
opaque-content round trips, deletion/disconnection undo, hidden-edge occupancy,
inspection-copy isolation and replacement during execution. File actions and
viewer/canvas persistence remain pending. See the
[N5e handoff](agents/handoffs/2026-10-01-native-checkpoint-n5e.md).

For readable workspace-test screenshots when offscreen font discovery fails, set
`SMARTFLOW_TEST_FONT` to a local font file before running `native_workspace`.
The test writes `build/native/native/n5e-unavailable-smoke.png` under CTest.

## N5f editor file actions

File actions use the atomic codec and track retained-content changes, including
undo back to the saved snapshot. Open and Close offer Save/Discard/Cancel for
unsaved content. Open selects the first graph and resets canvas layout and
selection; files with no graphs are rejected without replacing the editor.
Unknown content and other graphs remain retained. Workspace/viewer restoration
is the next checkpoint. See the [N5f handoff](agents/handoffs/2026-10-01-native-checkpoint-n5f.md).

## N5g workspace persistence

Save/Open restores stable-node canvas positions, selection, canvas navigation,
pinned output and extension-owned viewer JSON. Camera navigation changes dirty
workspace state without executing the graph or adding semantic undo commands.
Malformed view fields use safe defaults; unknown fields remain retained.
Scene tests cover a fresh-window round trip, camera/selection, layout, opaque
fields and semantic undo isolation. See [N5g](agents/handoffs/2026-10-01-native-checkpoint-n5g.md)
and [decision 0010](architecture/decisions/0010-native-workspace-persistence.md).

## N5h independent data extension

`smartflow.exe --data` selects Sample table ? Filter rows ? Summary. The separate
`smartflow-data.exe` builds the same workspace without scene-domain libraries.
Both support the generic numeric inspector, semantic undo, execution and project
Save/Open. The data viewer shows label/value rows and saves its selected row.

`native_data` checks filter/aggregation, empty input, snapshot isolation,
inspector/undo, saved parameters and viewer/layout restore, invalid parameters
and missing-package retention. Two additional startup checks cover `--data`
and the independent executable. Report: `build/native/native/data-results.xml`.
See [the data extension](../extensions/data/README.md) and
[decision 0011](architecture/decisions/0011-independent-native-data-extension.md).

## N5i shipped-project launch checks

`--project <path>` opens a retained document after window layout. Load failures
report a diagnostic and exit 6, without a modal dialog. Smoke mode then checks
the loaded graph, rather than the preset. Scene/data examples are installed
alongside the app and tested for parameters, viewer state and save/reopen.
CTest includes both example startup processes and exact exit-code/diagnostic
checks for missing projects and missing path arguments. See
[the examples](../examples/README.md) and the
[N5i handoff](agents/handoffs/2026-10-01-native-checkpoint-n5i.md).
