# Checkpoint N5h: independent native data extension

## Objective and scope

Implement the separate non-3D validation workflow using public native contracts.
Keep the bundled scene workflow as the primary app default. Desktop interaction
acceptance for both packaged workflows follows as a separate checkpoint.

## Current state

- `extensions/data/native` owns the versioned label/value table type, factory,
  Sample table, Filter rows and Summary nodes, and read-only TableViewer.
- Default sample/filter/summary yields count 3, total 104 and mean 104/3.
  Minimum 30 yields count 2, total 79 and mean 39.5; empty input yields zeros.
  Sample multiplier and filter minimum use the existing generic inspector.
- The viewer saves selected-row workspace state using N5g hooks. Results are
  transient; published table values are copied without mutating upstream data.
- `smartflow.exe --data` selects the data configuration. The separate packaged
  `smartflow-data.exe` and `data_tests` link workspace/data without scene-3d or
  scene-math. Generated project/link inputs were inspected with no scene matches.
- No graph editor, pipeline, SDK or file-envelope changes were required. Only
  composition, CMake, the new extension and behavior tests changed.
  See [decision 0011](../../architecture/decisions/0011-independent-native-data-extension.md).

## Verification

- CMake configure and Release build passed with process permission:
  `cmake -S . -B build/native -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/msvc2022_64`,
  then `cmake --build build/native --config Release --parallel 8`.
- All eleven CTest checks passed: `ctest --test-dir build/native -C Release
--output-on-failure`, with Qt on PATH and optional test font. New checks cover
  both data startup modes, aggregation, empty input, snapshot isolation,
  inspector undo/redo, fresh-window saved parameter/layout/row selection,
  unknown viewer fields, invalid parameters and missing-package retention.
- CMake install to absolute `build/desktop-test` passed. Both executables share
  the same deployed Qt/Windows compiler dependencies; existing optional DX12
  compiler deployment warning remains unrelated to this software preview.
- Packaged data-only startup returned 0 with PATH limited to Windows directories
  and Qt overrides cleared. Screenshot: `build/native/n5h-data-packaged.png`.
  Inspected `build/native/native/n5h-data-reopened.png`, showing restored minimum
  30, total 79, mean 39.5 and selected Total row.
- `npm.cmd run check` and `git diff --check` passed.
- Native file dialogs and desktop interaction will be exercised in the next
  checkpoint. These checks are not clean-machine or production-renderer evidence.

## Decisions and open questions

The sample is bundled fixture data, not CSV import. Table schema is intentionally
limited to a label and finite numeric value. Maximum supported input is 1000
rows; values are bounded to 1,000,000. No arbitrary schema, editing, sorting,
asset output codec, browser package or plugin loader is added. Viewer selection
uses row index. Full N5 remains pending packaged interaction acceptance.

## Next steps

1. Verify packaged scene and data workflows through real desktop interaction:
   edit/undo/redo, viewer interaction and save/close/relaunch/Open restore.
2. Record actual acceptance and limitations, updating the roadmap without
   claiming broader production or clean-machine guarantees.
3. Continue reusable graph components as a separately designed and committed
   step after N5 acceptance. Preserve source-level extension boundaries.
