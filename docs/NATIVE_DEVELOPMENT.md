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
The preview provides Number nodes on the copied QtNodes canvas. Right-click
to add nodes and drag matching ports to connect; edit the source value to
propagate it. There is no 3D viewer or stable project save/open command yet.

No source or build paths in CMake reference the old repository. Qt DLLs and
plugins must be discoverable locally; standalone installer/deployment is a
later checkpoint. Build outputs and personal CMake presets are ignored.

## N2 pipeline foundation

The build also produces `smartflow_legacy`, `smartflow_pipeline`, and
`pipeline_tests.exe`. CTest runs `native_pipeline` alongside the canvas test.
The pipeline suite writes `build/native/native/pipeline-results.xml`; for readable
Windows test output, run `pipeline_tests.exe -o pipeline-results.txt,txt` and read
that file. The native application still displays the N1 numeric canvas; N3 will
connect it to the background pipeline adapter.

The adapter takes a graph snapshot and immutable delegate/factory registrations,
then returns a cancellable future. It requires initialized named parameters and
explicit typed port mappings. In the legacy API, `setParamerter` creates the
parameter; `setParameterValue` alone does not initialize its name. Long-running
delegates must poll for cancellation and must not edit the graph or UI. See
[decision 0004](architecture/decisions/0004-native-pipeline-migration.md) for
behavior and limitations, and the [vendor audit](../native/vendor/legacy/README.md)
for the source/license inventory.
