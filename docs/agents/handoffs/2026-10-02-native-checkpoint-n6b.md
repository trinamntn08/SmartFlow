# Checkpoint N6b: component extraction and catalog commands

## Objective and scope

Continue N6a with the native authoring command foundation: extract a selected
subgraph into a reusable definition and store it through retained-document undo.
This checkpoint supplies APIs, not authoring UI or collapsed component nodes.

## Current state

- `GraphComponent::extract` copies selected nodes, internal edges and opaque graph
  metadata. The caller supplies input/output/control interfaces; every crossing
  connection must expose the endpoint inside the selection. Empty, duplicate or
  missing selections and incomplete interfaces reject without changing source.
- `GraphComponent::catalog` centralizes identity/version conflict checks used by
  both standalone catalog storage and instantiation. Identical definitions reuse
  the entry; unrelated unavailable catalog entries remain retained.
- DocumentSession provides structural catalog storage independently of installed
  node packages. DocumentHistory provides catalog and extraction commands, with
  atomic semantic undo/redo, current-workspace retention and identical-definition
  no-ops. Rejected extraction/catalog commands preserve the redo branch.
- The independent data tests cover extraction, preserved large integers and opaque
  node/edge/graph fields, unchanged source graphs, boundary checks, catalog
  conflicts/no-ops, unavailable definitions and save/reopen followed by actual
  execution of an extracted instance (total 79).
- Updated the component guide, roadmap, README, native development guide and
  decision 0012. No vendor sources, extension contracts or package boundaries changed.

## Verification

- `cmake --build build/native --config Release --parallel 8` passed with process
  permission. Nested Qt moc launches hit the documented environment restriction;
  direct `cmake -E cmake_autogen .../AutogenInfo.json Release` succeeded for
  history, components, workspace and dependent test targets, then rebuild passed.
- `ctest --test-dir build/native -C Release --output-on-failure` passed all 15
  checks, including component behaviors, both domains, offscreen startup, project
  launch errors and shipped-example startup. Qt bin was added to PATH for tests.
- `cmake --install build/native --config Release --prefix
D:/dev/projects/SmartFlow/build/desktop-test` refreshed both packaged executables.
  The existing optional DX12 compiler deployment warning remains.
- The packaged `smartflow-data.exe --project
build/desktop-test/examples/data.smartflow --smoke-test -platform offscreen`
  passed. No interactive desktop acceptance was performed; full N5 remains pending.
- `npm.cmd run check` passed formatting, TypeScript, nine persistence tests and
  production build. The final run needed process permission after a sandbox
  `spawn EPERM` error. `git diff --check` passed.

## Decisions and open questions

Extraction copies rather than replacing the selected graph. It requires explicit
interfaces and does not infer port types, automatically expose parameters or copy
boundary-edge metadata into bindings. The definition retains selected nodes and
internal edges exactly; body graph ID is normalized to `body`. Catalog storage is
structural, with registered types/bounds validated when instantiating. See
[decision 0012](../../architecture/decisions/0012-native-graph-component-foundation.md).

## Next steps

1. Add a component authoring/library UI over these commands, with user-selected
   interface endpoints and meaningful editor undo/save/reopen checks.
2. Implement collapsed instances with exposed controls and execution/result
   mapping as a separate checkpoint. Current instances expand ordinary nodes.
3. Verify packaged scene/data desktop interaction, dialogs, Save/Discard/Cancel
   and save/relaunch/Open when native computer control is available; only then
   record full N5 acceptance. This session did not retry native computer control.
