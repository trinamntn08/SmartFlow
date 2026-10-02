# Checkpoint N6c: native component authoring and library UI

## Objective and scope

Continue N6b with a native selection-to-component authoring workflow and saved
component library. Use explicit project commands for creation/insertion and verify
undo, execution and save/reopen. Collapsed component nodes remain separate work.

## Current state

- Added Components > Create from selection and Component library to both apps.
  `ComponentDialogs` stays in the workspace adapter, using public node
  registrations and the existing registry-independent component commands.
- Authoring lists free inputs, outputs and saved parameters in three tabs. Every
  boundary endpoint is required; users name exposed endpoints and choose extra
  outputs/controls. Creation copies the selection into the retained library and
  leaves the source graph unchanged. Component/instance IDs are generated.
- Library insertion lists compatible source outputs for each input and edits
  numeric controls with registered bounds. Unchanged controls retain their exact
  saved values; other parameter editors use retained defaults. Missing bindings,
  unavailable definitions and malformed catalogs report errors without mutation.
- Dialogs check semantic revision before acceptance. Cancel/validation failures
  preserve project state and redo history. Insertion is one semantic command;
  copied nodes are selected, placed beside the existing graph and framed through
  workspace state. These remain separate ordinary nodes.
- Added a data-only `native_component_ui` suite. Updated component/development/
  desktop guides, roadmap, README and architecture documentation.

## Verification

- CMake configure and `cmake --build build/native --config Release --parallel 8`
  passed with process permission. Direct `cmake -E cmake_autogen ... Release`
  generation used the documented Qt process-launch workaround for workspace and
  test targets. No vendor sources or old checkout were changed.
- `ctest --test-dir build/native -C Release --output-on-failure`: all 16 checks
  passed, including both domain startup/examples and the new menu/dialog tests.
  Qt bin was added to PATH; optional `SMARTFLOW_TEST_FONT` used a local Segoe UI
  font for offscreen rendering (not bundled).
- UI tests cover exposed inputs/outputs/controls, two independently executing data
  instances (totals 79 and 104), single-command undo/redo, failed-edit redo
  retention, no node overlap, save/reopen and insertion from the reopened library,
  compatible input filtering against a numeric source, stale dialogs,
  cancellation and unavailable/malformed catalog preservation.
- Visually inspected `build/native/native/component-author-smoke.png`,
  `component-library-smoke.png` and `component-workspace-smoke.png`. An initial
  placement render showed overlap; placement was corrected and the final render
  plus intersection checks passed.
- `cmake --install build/native --config Release --prefix
D:/dev/projects/SmartFlow/build/desktop-test` refreshed both packaged apps. The
  existing optional DX12 compiler deployment warning remains.
- Both packaged executables passed `--project <shipped-example> --smoke-test`
  on the native Windows platform, with screenshot/font options. `Start-Process`
  used `-WindowStyle Hidden -Wait -PassThru` and each process exited 0. Scene/data
  renders were inspected as `build/native/n6c-scene-packaged.png` and
  `n6c-data-packaged.png`. Initial packaged offscreen attempts exited
  -1073740791 without rendering; deployment includes `qwindows.dll`, not the
  offscreen platform plugin. Offscreen verification uses the developer Qt kit.
- `npm.cmd run check` passed formatting, TypeScript, nine persistence tests and
  production build. `git diff --check` passed.
- The computer-use skill was initialized through `@oai/sky`; `sky.list_windows()`
  reported unavailable native pipe (os error 2). Real desktop interactions/native
  file dialogs could not be verified. Full N5 acceptance remains pending.

## Decisions and open questions

See [decision 0012](../../architecture/decisions/0012-native-graph-component-foundation.md).
There is no collapsed node, instance-control inspector, catalog edit/delete/export,
definition migration, nested execution or live definition updates. The library
belongs to the saved project; insertion adds ordinary independent node copies.
The UI obtains endpoints/types from active registrations and adds no domain branch
or new public SDK contract. Large component authoring/performance is not validated.

## Next steps

1. Design and implement collapsed component nodes with exposed inputs/outputs,
   parameter controls and execution/result mapping as a separately verified step.
2. When desktop control becomes available, exercise packaged scene/data editing,
   component dialogs, Undo/Redo, viewer interaction, native Save/Discard/Cancel
   and save/relaunch/Open before marking full N5 desktop acceptance complete.
3. Keep completed checkpoints separately documented and committed.
