# First desktop test version

The first version for user testing is the Windows native C++/Qt application.
The browser starter is deferred. This is a local development preview, not a
public installer or a completed N5 release. Implementation is complete through
N7 and VS Code setup; full N5 packaged interaction acceptance is pending the
user's manual test results. Automated native computer-use is deferred for now.

**Ready for first manual testing, 2026-10-02:** the installed folder at
`build/desktop-test` contains the N7 app, examples, runtime dependencies,
README and testing guides. Seven Windows startup/render checks passed with
developer Qt paths removed. Start `bin/smartflow.exe` or `bin/smartflow-data.exe`.
Use [the test-result sheet](TEST_VERSION.md) to record your interaction results.

For development runs, use [VS Code CMake Tools](NATIVE_DEVELOPMENT.md#run-from-vs-code-with-cmake-tools).
Acceptance runs below use the installed test folder, so they check bundled runtime
dependencies as well as app behavior.

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

## Open the shipped examples

CMake install copies scene, data and component projects plus the standalone
component definition into the desktop test folder. Launch either directly from a PowerShell prompt:

```powershell
build/desktop-test/bin/smartflow.exe --project "$PWD/build/desktop-test/examples/scene.smartflow"
build/desktop-test/bin/smartflow-data.exe --project "$PWD/build/desktop-test/examples/data.smartflow"
build/desktop-test/bin/smartflow-data.exe --project "$PWD/build/desktop-test/examples/components.smartflow"
```

The scene example restores size 3 and an orbited camera with the object selected.
The data example restores minimum 30, total 79, mean 39.5 and the selected Total
row. See [example details](../examples/README.md). Missing project paths report
an error and exit 6; choose the matching scene/data app configuration.

## Verify

```powershell
$env:PATH = "C:/Qt/6.11.1/msvc2022_64/bin;" + $env:PATH
ctest --test-dir build/native -C Release --output-on-failure
build/desktop-test/bin/smartflow.exe --smoke-test
```

For repeatable packaged verification, run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/Test-DesktopPackage.ps1
```

The script checks installed/build executable SHA256 equality, required runtime
files and seven Windows native startup configurations. It clears developer Qt
variables, reduces PATH to Windows system folders, launches each test process
hidden and records rendering screenshots plus `verification/startup-results.json`
inside the test folder. The environment changes apply only to that script process.
This verifies startup/rendering; manual interaction still uses the steps below.

The packaged smoke check briefly opens a native window and exits after successful
execution. CTest includes offscreen scene and numeric application startup, graph editing, execution,
undo/redo, file failures, unknown content, and scene edit/save/reopen with a visible
result. Offscreen checks do not replace desktop interaction checks.

For the desktop test:

First try the [widget layout controls](WORKSPACE_WIDGETS.md): choose a widget in
any region, use H/V to split, X to close and drag dividers to resize. Selecting an
already visible widget swaps its placement. Save/relaunch/Open should restore
the arrangement. **Layout > Reset widget layout** recovers the default layout.

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

## Independent table workflow

Launch `build/desktop-test/bin/smartflow-data.exe` or `smartflow.exe --data`.
The separate executable does not link scene libraries. Sample table -> Filter
rows -> Summary initially displays count 3, total 104 and mean 34.66666667.
Select Filter rows, change minimum from 20 to 30 and Apply; expect count 2,
total 79 and mean 39.5. Undo/Redo restores those results. Select a viewer row,
move a canvas node, Save As, relaunch and Open to check parameters, selection
and layout restoration. This is a bundled sample, without CSV import.

## Reusable components

Select Filter rows and Summary, then use Components > Create from selection.
Name the component, expose its minimum control and name its table input and
summary output. Open Components > Component library, bind the input to Sample
table / out and set minimum to 30. Leave **Insert as one node** checked, insert it, select the component and pin
its summary output: expect total 79. Undo/Redo should remove/restore the instance
as one command. Save, relaunch and Open; confirm the saved component is available
in the library and its saved instances still execute. Try the shipped `components.smartflow` example
and use the toolbar Output selector to switch between rows and summary. See
[the component guide](GRAPH_COMPONENTS.md) for the full workflow. In the library,
export a definition to `.smartflow-component`, open another project and import it.
Check the original nodes remain unchanged, the catalog import supports Undo/Redo,
and the imported definition can be inserted with compatible bindings. Importing
identical content should add no history entry; conflicting identities report an
inline error. The shipped `filtered-summary.smartflow-component` is ready to import.
Select an entry and use **Remove from library...**: No preserves it; Yes removes
only the library entry. Verify existing collapsed instances still execute,
Undo/Redo restores/removes the entry, and saved instances reopen without it.
Choose **Edit a copy...**, change a body parameter in its draft inspector, and
use **Save copy...** to choose its title/interface. Confirm the new entry has the
edited defaults, existing instances retain their original values, and Cancel
discards draft edits. Undo/Redo should remove/restore the saved catalog copy.
Select one collapsed instance and use **Components > Update selected instance...**
to choose the edited copy. Review the body/defaults and Apply. Confirm existing
control values and pinned output remain, other instances are unchanged, and Undo
restores the old snapshot. Try an incompatible interface: Apply must be disabled
with an explanatory error. Save/relaunch/Open must retain the chosen snapshot.

## Current limitations

- Canvas positions/navigation, selection and viewer camera/pinning are saved.
  Selectable widget regions and divider sizes are saved; outer window geometry is not.
  Files without saved workspace state use a fitted canvas and terminal output by default.
- Open edits the first graph; other graphs and unknown extension data remain
  preserved. Unsupported nodes are displayed but cannot execute.
- The 3D view is a primitive software preview, not a production GPU renderer.
- The independent data extension is available; packaged interaction acceptance
  for scene, data and components is pending manual results.
- Components support collapsed snapshots and optional ordinary node copies. Nested
  components, definition migration, automatic updates
  of existing instances are pending.
- Public installer/signing, clean-machine compatibility and distribution
  preparation remain future work.

## Record manual results

Report the tested executable/configuration and whether scene, data, component,
import/export/removal, Undo/Redo and save/relaunch/Open checks passed. Include
reproduction steps and expected/actual behavior for failures. Use a separate saved
file to preserve shipped fixtures. Manual results have not yet been reported; the
current automated baseline is a Release build, 18 CTest entries and successful
Windows startup smoke checks. A clean-machine/public-install test remains separate.
