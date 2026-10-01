# Checkpoint N6a: reusable graph-component foundation

## Objective and scope

Continue the roadmap with reusable graph definitions and atomic subgraph
instantiation while N5 real desktop acceptance remains blocked. This checkpoint
implements model/command behavior, not collapsed component nodes or authoring UI.

## Current state

- Added a registry-independent `GraphComponent` model and `smartflow_components`
  library, depending only on the project codec and Qt Core. The versioned
  definition records its retained body and named input/output/control bindings.
- Instantiation inserts separate ordinary node/edge copies with deterministic,
  length-prefixed instance namespaces, wires required external inputs and returns
  exposed output/control endpoints. Controls override only their own instance.
- Definitions are retained in optional `project.components`. Identical ID/version
  entries are reused; conflicting content and malformed catalog shapes reject.
  Unknown definition/node/edge/endpoint data survives, including large integers.
- Session preparation validates active node contracts and exposed output ports,
  rejecting newly introduced diagnostics before mutation. Existing unavailable
  content stays retained and blocks execution. Unknown catalogs still round-trip
  through the project codec without being interpreted on load.
- DocumentHistory exposes one-command instantiation/catalog insertion, with exact
  semantic undo/redo and current-workspace retention. Rejected commands preserve
  the redo branch. Neither component code nor its data-based tests imports scene
  types. See [decision 0012](../../architecture/decisions/0012-native-graph-component-foundation.md)
  and [the component guide](../../GRAPH_COMPONENTS.md).

## Verification

- CMake configure and Release build passed with process permission. Used direct
  `cmake -E cmake_autogen` for document history and the component test before
  building, per the documented workaround. No vendor sources were changed.
- All fifteen CTest checks passed, including the new `native_components` suite,
  both offscreen app modes, independent data startup, saved-example startup,
  native project/session/history/workspace/scene/data tests and pipeline tests.
- Component behavior checks two instances (totals 79 and 104), independent control
  edits, actual file save/reopen, retained catalogs/opaque fields, exact semantic
  undo, workspace isolation, redo preservation on rejected edits, bad controls,
  missing/bad input sources, bad output ports, ID conflicts, cycles and unsupported
  definitions. An initial ambiguous single-element JSON graph assignment failed
  validation; explicit array construction fixed it and the suite passed.
- Generated component library/test projects contain no scene dependency matches.
  `cmake --install build/native --config Release --prefix "$PWD/build/desktop-test"`
  passed, refreshing both app executables. The existing optional DX12 compiler
  deployment warning remains unchanged.
- `npm.cmd run check` passed: formatting, TypeScript, nine persistence tests and
  production build. `git diff --check` passed.
- No UI layout changed. Native offscreen startup checks passed. The computer-use
  helper was retried through `sky.list_windows()` and still reported unavailable
  native pipe (os error 2); real desktop interaction was not possible this session.

## Decisions and open questions

Instances currently expand into ordinary nodes. There is no collapsed wrapper
node, component-library/authoring UI, public plugin system, nested execution,
fan-out input binding or definition migration. Existing instances do not update
when a catalog definition changes. New catalog definitions use contract version 1;
future/unknown entries are preserved but cannot be explicitly instantiated.
The model/command implementation favors transactional correctness over large
project performance. Full N5 desktop acceptance remains separately pending.

## Next steps

1. Add component extraction/catalog commands and a library/authoring UI as a
   separate native checkpoint, using explicit project commands and meaningful
   undo/save/reopen tests. Then implement collapsed instances with exposed controls
   and execution/result mapping; do not claim this already behaves as one node.
2. When the computer-use helper works, verify packaged scene/data editing,
   undo/redo, viewer interaction, native file dialogs and save/relaunch/Open,
   including Save/Discard/Cancel. Mark full N5 only after those checks pass.
3. Keep verified checkpoints documented and committed separately. Preserve the
   domain-independent platform, required included 3D workflow and old repository.
