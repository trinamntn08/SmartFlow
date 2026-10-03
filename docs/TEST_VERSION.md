# Windows test version: refreshed 2026-10-03

Status: user reports the preceding version's manual testing is OK so far
(feedback received 2026-10-03). This refresh includes native Use mode, modular
scene interaction and inline node controls.
Individual checklist coverage and the tested executable/environment were not specified.
Full checklist acceptance and clean-machine compatibility remain unrecorded.

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
- Compact numeric controls directly inside graph nodes, including components'
  exposed controls. Enter or leaving the field applies; arrow steps apply immediately.
- Background execution/cancellation and pinned result viewers.
- **Run: ... ms** on graph nodes and a **Run ms** results column.
- **Process Gantt** widget with ready-queue/execution timing bars and node selection.
- Sequential/Parallel and Threads controls, live node/component progress and
  managed node-internal work sharing the configured per-run budget.
- Primitive 3D scene workflow plus an independent table/filter/summary workflow.
- Camera pan/zoom/views, object picking and undoable Move/Rotate Y/Scale gestures.
- Atomic Save/Open with layout, navigation, selection and viewer restoration.
- Selectable widget regions with H/V splits, swapping, closing and saved dividers.
- Components with collapsed snapshots, exposed ports/controls, import/export and removal.
- Isolated **Edit a copy** drafts and explicit **Update selected instance** with
  compatibility validation, preserved values and exact snapshot undo.
- Retention of unknown extension/project content with unsupported-execution diagnostics.
- Build/Use switching, exposed component controls and output viewers, shared
  undo/execution, and independently saved tool selection/viewer navigation.

## Try Use mode

Open `examples/scene-tool.smartflow` in the scene app; it opens in Use mode.
Open `examples/components.smartflow` in the data app and choose **Use** in the
toolbar. Change an exposed control and Apply, try Undo/Redo, select an output,
then Save As and reopen. Choose Build to return to the graph. See
[Use mode](USE_MODE.md) for the full procedure and supported scope.

## Automated evidence

Release configure/build and all 32 native CTest entries pass, including inline
canvas typing, focus change, cancellation, undo, shared widget synchronization,
components and save/reopen. The required web checks pass. Thirteen installed
Windows configurations start, execute and render with developer Qt environment
variables removed. Installed executable hashes match the build. Native inline
node and packaged scene/component screenshots were visually inspected.
Real desktop interaction acceptance of the new controls remains unreported.

The package checker records source commit, executable SHA256 values and screenshots
in `build/desktop-test/verification/startup-results.json`. Startup/offscreen evidence
does not establish manual file-dialog/desktop interaction or clean-machine acceptance.

## Process Gantt manual check

Choose **Process Gantt** in a region selector, Run each scene/data/component
example, and inspect queue/run values and bars. Click a row to select its graph
node. Split/move the widget, Save, relaunch/Open and confirm its placement returns;
Run again for fresh timings. See [timing semantics](GANTT_ANALYSIS.md).

## Record results

On 2026-10-03 the user reported: "already manually tested, it's ok so far".
No issues were reported. The rows below retain their original pending status
because results were not supplied individually; they do not mean no testing took
place. See the [feedback checkpoint](agents/handoffs/2026-10-03-manual-test-feedback.md).

Also test the region selectors, H/V split, X close and divider resizing. Confirm
the saved arrangement returns after relaunch/Open; use **Layout > Reset widget
layout** to recover defaults. See [workspace widgets](WORKSPACE_WIDGETS.md).

Tested date, executable, Windows version and package source commit:

| Check                           | Expected result                                                    | Your result / reproduction steps |
| ------------------------------- | ------------------------------------------------------------------ | -------------------------------- |
| Scene edit and Undo/Redo        | Cube size changes the preview and restores correctly               | Pending                          |
| Execution modes and threads     | Sequential/Parallel run correctly with thread limits 1, 2 and 4    | Pending                          |
| Progress and cancellation       | Node states/counts update; Cancel discards outputs and drains work | Pending                          |
| Scene navigation                | Orbit, zoom, frame and selection work                              | Pending                          |
| Data edit and Undo/Redo         | Minimum 30 gives count 2, total 79 and mean 39.5                   | Pending                          |
| Node/connection edits           | Add/connect/delete, then Undo/Redo restore graph behavior          | Pending                          |
| Component creation/insertion    | Bound exposed inputs/controls execute as one node                  | Pending                          |
| Component file exchange/removal | Import/export works; removal preserves existing snapshots          | Pending                          |
| Edit a copy                     | New entry has edited defaults; original instances stay unchanged   | Pending                          |
| Update selected instance        | Compatible update works; overrides/pinning persist; Undo restores  | Pending                          |
| Save/relaunch/Open              | Parameters, layout, selection and viewer state restore             | Pending                          |
| File errors/unsaved prompts     | Errors preserve active work; Save/Discard/Cancel behave correctly  | Pending                          |

For failures, include the exact steps, expected/actual result and whether the
failure repeats after relaunch. Manual results should determine the next fixes.

## Remaining scope

The 3D renderer is a bounded primitive preview. Asset import/bundling, nested
components, input fan-out interfaces, automatic migrations, CSV import and dynamic
plugin loading remain future work. Public installer/signing and clean-machine
compatibility remain separate delivery work. These limits are also listed in
[the roadmap](ROADMAP.md).
