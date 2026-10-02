# Task handoff: current documentation synchronization

## Objective and scope

Synchronize repository documentation with the delivered native preview through
N6f and the VS Code CMake Tools setup. This is a documentation-only checkpoint;
application code, build configuration, vendor sources and licenses are unchanged.

## Current state

- Updated the roadmap, architecture, development guides, desktop testing,
  product implementation status, component guide and repository/area READMEs.
- Added documentation, decision and handoff indexes. Dated records retain their
  historical scope; current guides explain which earlier pending items are complete.
- Clarified that the TypeScript application is a preserved setup prototype,
  its runtime is reserved and native execution is implemented separately.
- Corrected the vendor overview's stale migration status without changing
  copied sources, manifests, licenses or per-module provenance.

## Verification

Passed `npm.cmd run check`: formatting, TypeScript, nine persistence tests and
web production build. `git diff --check` passed. A local Markdown file-link check
resolved all 224 links across 68 Markdown files.
No native code changed, so native builds/CTest were not repeated for this step.
The preceding IDE checkpoint verified Release configure/build, 17/17 CTest
entries and Windows startup checks; the Debug configuration was not built.

## Decisions and open questions

Full N5 packaged desktop interaction acceptance remains pending the user's manual
results. The user deferred automated native computer-use after an unavailable
connection; no manual results have been reported. Offscreen/startup/build-tree
checks do not establish packaged edit/undo/save/relaunch/Open acceptance.

[Current roadmap](../../ROADMAP.md), [desktop test procedure](../../DESKTOP_TESTING.md)
and [component guide](../../GRAPH_COMPONENTS.md) describe current scope and limits.
Component definition editing, version/migration policy and explicit instance
updates still need a scoped decision. Distribution/signing and clean-machine
compatibility remain open.

## Next steps

Collect manual desktop test results, then address demonstrated issues in separate
verified checkpoints. Preserve the current snapshot-instance behavior and the
independent non-3D workflow. Do not resume deferred automation without new user
instructions.
