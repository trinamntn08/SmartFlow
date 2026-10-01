# First desktop test version

The first version for user testing is the Windows native C++/Qt application.
The browser starter is deferred. This is a local development preview, not a
public installer or a completed N5 release.

## Build and prepare

Use Visual Studio 2022 C++ tools, CMake and a matching Qt6 MSVC kit. Adjust the
Qt path to your installation. No old repository checkout is required.

```powershell
cmake -S . -B build/native -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/msvc2022_64
cmake --build build/native --config Release --parallel 8
cmake --install build/native --config Release --prefix "$PWD/build/desktop-test"
```

Open `build/desktop-test/bin/smartflow.exe` in Explorer. Keep the whole
`desktop-test` folder together: it contains Qt DLLs/plugins, Qt configuration,
and the compiler runtime. Running this folder does not require Node.js, npm,
a browser server or a Qt PATH entry. Close the app before reinstalling the folder.

## Verify

```powershell
$env:PATH = "C:/Qt/6.11.1/msvc2022_64/bin;" + $env:PATH
ctest --test-dir build/native -C Release --output-on-failure
build/desktop-test/bin/smartflow.exe --smoke-test
```

The packaged smoke check briefly opens a native window and exits after successful
execution. CTest includes offscreen scene and numeric application startup, graph editing, execution,
undo/redo, file failures, unknown content, and scene edit/save/reopen with a visible
result. Offscreen checks do not replace desktop interaction checks.

For the desktop test:

1. Launch the installed executable. Confirm the Cube/Transform/Material/Scene
   graph executes and displays a cube in the 3D preview.
2. Select Cube, change size to 3, and click Apply. Confirm the preview updates;
   use Undo and Redo to check both parameter and result changes.
3. Drag in the preview to orbit and use the wheel to zoom.
4. Use File > Save As to save a `.smartflow` file. Confirm the title shows its
   filename and the unsaved marker clears.
5. Change a parameter, then Open the saved file. Check Cancel keeps the edit,
   Discard loads the saved version, and Save preserves the edit before opening.
6. Close and relaunch the app, then Open the saved file. Confirm the parameter,
   graph execution and scene result are restored.
7. Try opening an invalid file or saving to an unwritable location. Confirm an
   error is shown and the active project remains usable.

## Current limitations

- Canvas positions/navigation and viewer camera/pinning are not yet saved.
  Open resets the canvas and displays a terminal output by default.
- Open edits the first graph; other graphs and unknown extension data remain
  preserved. Unsupported nodes are displayed but cannot execute.
- The 3D view is a primitive software preview, not a production GPU renderer.
- The independent non-3D extension and remaining N5 acceptance are pending.
- Public installer/signing, clean-machine compatibility and distribution
  preparation remain future work.
