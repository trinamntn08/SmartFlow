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
