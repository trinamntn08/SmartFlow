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

## Run from VS Code with CMake Tools

Open this repository folder or `smartflow.code-workspace`. Install the recommended
**CMake Tools** and **C/C++** extensions if they are missing. Your local machine
has an ignored `CMakeUserPresets.json` already prepared with its Qt MSVC kit.
The shared `CMakePresets.json` contains portable templates, not machine paths.

1. Run **CMake: Select Configure Preset** and choose
   **Windows local (Qt / MSVC x64)**, then **CMake: Configure**.
2. Run **CMake: Select Build Preset** and choose **Windows local Release**.
   Release reuses the existing verified build. **Windows local Debug** is also
   available for source debugging; its first build compiles a separate configuration.
3. Run **CMake: Set Launch/Debug Target** and choose **smartflow** for the 3D app,
   or **smartflow-data** for the independent table app. The Launch entry in the
   CMake sidebar also lets you choose the executable.
4. Click the play button beside **Launch**, or run **CMake: Run Without Debugging**.
   CMake Tools builds the target first and supplies the Qt runtime through the
   preset environment. The working directory is the repository root.
5. For F5 or Ctrl+F5, use a **SmartFlow** configuration in the Run and Debug menu.
   `SmartFlow: selected CMake target` starts the chosen executable;
   `SmartFlow: data workflow` passes `--data`; `SmartFlow: component example`
   loads the shipped collapsed-component project. Choose `smartflow` or
   `smartflow-data` as the CMake launch target first, rather than a test executable.
   These configurations resolve the executable and Qt prefix from CMake Tools.

To run tests, choose **Windows local Release tests** with **CMake: Select Test
Preset**, then **CMake: Run Tests**. The presets also work from a terminal:

```powershell
cmake --preset windows-local
cmake --build --preset windows-local-release
ctest --preset windows-local-release-tests
```

On another machine, copy
[the user-preset example](examples/CMakeUserPresets.example.json) to the repository
root as `CMakeUserPresets.json`, and change `SMARTFLOW_QT_ROOT` to that machine's
Qt MSVC kit directory (the directory containing `bin` and `lib`). Keep this file
out of Git. The launch configurations expect the single Qt root used by these
presets as `CMAKE_PREFIX_PATH`. Qt version upgrades belong in the local preset.
VS2022 C++ tools, CMake 3.21 or newer, and Qt6 Widgets/OpenGLWidgets/Test are required.

These steps follow the [CMake Tools preset guide](https://github.com/microsoft/vscode-cmake-tools/blob/main/docs/cmake-presets.md)
and [target launch guide](https://github.com/microsoft/vscode-cmake-tools/blob/main/docs/debug-launch.md).
Using a build-tree executable is a development workflow. Packaged desktop
acceptance still uses `build/desktop-test`; the user is testing it manually and
that acceptance is pending until results are reported.

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

## N6a reusable graph components

N6b extends the component API with selection extraction and standalone catalog
commands. `native_components` checks boundary interfaces, unchanged source graphs,
catalog identity conflicts/no-ops, semantic undo/redo, workspace isolation and
reopened extracted definitions executing through the data extension. See the
[component guide](GRAPH_COMPONENTS.md) and [N6b handoff](agents/handoffs/2026-10-02-native-checkpoint-n6b.md).

`smartflow_components` is a project-codec/Qt Core library, without domain or
canvas imports. DocumentSession/DocumentHistory expose atomic component
instantiation with retained catalog insertion, typed binding validation and undo.
The N6c workspace UI invokes this model/command API. N6d extends it with
collapsed instances.
`native_components` checks independent data instances, save/reopen, opaque-field
retention, undo/redo and failed-command isolation. Report:
`build/native/native/component-results.xml`. Read the
[component guide](GRAPH_COMPONENTS.md) and
[decision 0012](architecture/decisions/0012-native-graph-component-foundation.md).

## N6c component authoring and library

The Components menu creates reusable definitions from canvas selections and
inserts saved definitions with compatible source bindings and numeric controls.
Creation and insertion use the retained document history; cancellation and
validation failures preserve project state. Inserted nodes are selected, placed
beside the existing graph and framed. IDs are generated automatically.

`native_component_ui` drives actual menu/modal-dialog actions in a data-only
workspace and verifies execution, undo/redo, save/reopen/library reuse, placement,
compatible-input filtering, cancellation, stale dialogs and unavailable catalogs.
Report: `build/native/native/component-ui-results.xml`. With optional
`SMARTFLOW_TEST_FONT` configured, it also writes `component-author-smoke.png`,
`component-library-smoke.png` and `component-workspace-smoke.png` in that directory.
Offscreen tests/render inspection do not establish real desktop interaction
acceptance. See [the guide](GRAPH_COMPONENTS.md) and
[N6c handoff](agents/handoffs/2026-10-02-native-checkpoint-n6c.md).

## N6d collapsed component instances

The library defaults to one retained snapshot node. Exposed ports/control edits
use ordinary document commands; execution expands private body steps and groups
results back to the instance. The toolbar selects and pins individual outputs.
`native_components`, `native_pipeline`, `native_component_ui` and `native_scene`
cover validation, dependency barriers, failure/cancellation, controls, viewers,
undo and save/reopen. `native_component_example_startup` checks the shipped data-only
example. UI tests write `component-collapsed-smoke.png`,
`component-example-smoke.png` and `component-collapsed-scene-smoke.png`.
Set `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf` for readable offscreen
component and scene screenshots. See the [component guide](GRAPH_COMPONENTS.md).

## N6e component file exchange

The library's Import/Export buttons use `project::readComponent`/`writeComponent`.
The existing project codec exposes schema-independent `jsonFile` transport with
strict preflight and atomic writes; its project entry points retain schema checks.
`native_components` checks lossless file exchange, unsupported/lossy input,
size/nesting limits, write-failure preservation and reuse in another project.
`native_component_ui` drives real Qt file dialogs, cancellation, inline errors,
unavailable-package exchange, import undo/redo, conflicts, snapshot isolation and
save/reopen. It writes `component-file-library-smoke.png` for render inspection.
This Qt test coverage does not establish real Windows desktop acceptance.

## N6f undoable library removal

`DocumentSession` and `DocumentHistory::removeCatalogComponent(index)` remove one
retained array entry without changing graphs, instance snapshots or workspace.
The library confirms removal with No as the default and refreshes its selection.
`native_components` tests duplicate/opaque entries, failed-command redo retention,
workspace isolation and execution/save/reopen after the entire catalog is removed.
`native_component_ui` drives No/Yes confirmation, unavailable/empty/stale selections,
undo/redo and viewer restoration. It writes `component-library-removal-smoke.png`.
Definition editing and live instance updates remain separate future work.
