# Windows first test version: 2026-10-02

Status: ready for manual testing. Full desktop interaction acceptance awaits results.

## Launch

Open `build/desktop-test/bin/smartflow.exe` for the scene workflow, or
`build/desktop-test/bin/smartflow-data.exe` for the independent table workflow.
Keep the entire test folder together. No browser server, Node.js or developer
Qt PATH is needed. The folder includes a README, guides and shipped examples.

Use **File > Open** with the matching `examples/scene.smartflow`,
`examples/data.smartflow` or `examples/components.smartflow`. Save your work to
a separate file. Follow [the full procedure](DESKTOP_TESTING.md) for details.

## Delivered in this version

- Native node editing, typed connections, parameter inspector and semantic Undo/Redo.
- Background execution/cancellation and pinned result viewers.
- Primitive 3D scene workflow plus an independent table/filter/summary workflow.
- Atomic Save/Open with layout, navigation, selection and viewer restoration.
- Selectable widget regions with H/V splits, swapping, closing and saved dividers.
- Components with collapsed snapshots, exposed ports/controls, import/export and removal.
- Isolated **Edit a copy** drafts and explicit **Update selected instance** with
  compatibility validation, preserved values and exact snapshot undo.
- Retention of unknown extension/project content with unsupported-execution diagnostics.

## Automated evidence

Release configure/build and the native suite are verified; all 18 CTest entries
passed after N7. The required web checks pass, including nine persistence tests.
Seven installed Windows configurations start, execute and render with developer
Qt environment variables removed. Installed executable hashes match the build.
Packaged scene and component screenshots were visually inspected.

The package checker records source commit, executable SHA256 values and screenshots
in `build/desktop-test/verification/startup-results.json`. Startup/offscreen evidence
does not establish manual file-dialog/desktop interaction or clean-machine acceptance.

## Record results

Also test the region selectors, H/V split, X close and divider resizing. Confirm
the saved arrangement returns after relaunch/Open; use **Layout > Reset widget
layout** to recover defaults. See [workspace widgets](WORKSPACE_WIDGETS.md).

Tested date, executable, Windows version and package source commit:

| Check                           | Expected result                                                   | Your result / reproduction steps |
| ------------------------------- | ----------------------------------------------------------------- | -------------------------------- |
| Scene edit and Undo/Redo        | Cube size changes the preview and restores correctly              | Pending                          |
| Scene navigation                | Orbit, zoom, frame and selection work                             | Pending                          |
| Data edit and Undo/Redo         | Minimum 30 gives count 2, total 79 and mean 39.5                  | Pending                          |
| Node/connection edits           | Add/connect/delete, then Undo/Redo restore graph behavior         | Pending                          |
| Component creation/insertion    | Bound exposed inputs/controls execute as one node                 | Pending                          |
| Component file exchange/removal | Import/export works; removal preserves existing snapshots         | Pending                          |
| Edit a copy                     | New entry has edited defaults; original instances stay unchanged  | Pending                          |
| Update selected instance        | Compatible update works; overrides/pinning persist; Undo restores | Pending                          |
| Save/relaunch/Open              | Parameters, layout, selection and viewer state restore            | Pending                          |
| File errors/unsaved prompts     | Errors preserve active work; Save/Discard/Cancel behave correctly | Pending                          |

For failures, include the exact steps, expected/actual result and whether the
failure repeats after relaunch. Manual results should determine the next fixes.

## Remaining scope

The 3D renderer is a bounded primitive preview. Asset import/bundling, nested
components, input fan-out interfaces, automatic migrations, CSV import and dynamic
plugin loading remain future work. Public installer/signing and clean-machine
compatibility remain separate delivery work. These limits are also listed in
[the roadmap](ROADMAP.md).
