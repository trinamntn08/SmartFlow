# Task handoff: W2a retained commands and extension lookup

## Objective and scope

Implement retained TypeScript graph commands/history and bundled extension lookup
before browser editor work. No persisted schema or native source changes.

## Current state

Core now exports transactional ProjectHistory and add/delete/connect/disconnect/
parameter commands. Full project snapshots preserve opaque fields, inactive graphs,
assets and stable IDs. Semantic undo/redo restores project content while retaining
current workspace. Workspace changes do not advance semantic revision. Dirty state
compares against the complete saved document; replacement validates before clearing
history. Failed/no-op edits retain redo, successful branches clear it, and neither
returned snapshots nor command callbacks can alias retained state.

The SDK exports an immutable bundled ExtensionRegistry with exact package/node
version lookup, registered type resolution and typed port compatibility. Registration
rejects duplicate IDs, unavailable dependencies, unknown port types and invalid
parameter defaults. Parameter validation covers control types, ranges and choices.
There is no dynamic plugin loader or browser executor yet.

## Verification

`npm.cmd run check` passed formatting, TypeScript, 43 core tests, four SDK tests,
three web transport tests and production build. The new tests exercise exact
deletion/edge restoration, stable IDs, workspace isolation, dirty state, rejected
commands/redo retention, branching, replacement and registry ownership. `git diff
--check` passed. Native code/schema and browser UI are unchanged; native and browser
interaction checks were not rerun for this model-only step.

## Decisions and open questions

History owns semantic project snapshots; the UI must use updateWorkspace for view
state and must validate registered ports/parameters before submitting semantic
commands. Connection commands enforce endpoint existence and single-input occupancy,
while registry/runtime validation owns domain contracts and execution cycles.

## Next steps

W2b: select and spike the browser canvas dependency, then add library/canvas,
inspector, placeholders, keyboard undo/redo and strict import/download actions.
