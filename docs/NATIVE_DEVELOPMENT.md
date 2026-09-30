# Native development

SmartFlow now starts with C++17, Qt6 Widgets and CMake. The existing TypeScript
workspace is retained as a future extension prototype.

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
double-click to frame and click to select an object. There is no stable project
save/open command yet; clipboard import/duplication is disabled.

No source or build paths in CMake reference the old repository. Qt DLLs and
plugins must be discoverable locally; standalone installer/deployment is a
later checkpoint. Build outputs and personal CMake presets are ignored.

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
