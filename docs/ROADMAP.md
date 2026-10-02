# Roadmap and current state

Updated: 2026-10-02. Native C++17/Qt6 on Windows is the primary implementation.
The application domain and audience remain open; included 3D support is required.

## Available now

SmartFlow is a working desktop development preview with graph editing, typed
connections, numeric parameter inspection, semantic undo/redo, background
execution, cancellation and result viewers. Project files preserve unsupported
extension content and other graphs; unsupported graphs report diagnostics and
cannot execute. Open/Save/Save As and workspace restoration are implemented.

The main app includes Cube, Transform, Material, Scene and Merge with a primitive
3D viewer. The independent data app has Sample table, Filter rows, Summary and a
table viewer; it does not link the scene extension or scene-math libraries.

Components support extraction, saved catalogs, collapsed snapshot instances,
exposed ports/controls, output selection/pinning, optional expanded insertion,
standalone import/export and undoable library removal. They are native source-level
features, not a production runtime or dynamically loaded plugin system.

VS Code CMake Tools configure/build/test presets and launch configurations are
ready for manual use. A CMake install folder bundles both executables, Qt/MSVC
runtime dependencies and the scene, data and component examples.

## Verification and desktop acceptance

The latest implementation/configuration checks passed: native Release build,
17 CTest entries, Windows startup smoke checks and `npm.cmd run check` (formatting,
TypeScript, nine persistence tests and web production build).

**Full N5 desktop interaction acceptance remains pending.** Automated native
computer-use reported an unavailable pipe. The user has deferred that automation
and will test manually. No manual results have been reported yet. Build-tree
launch, offscreen tests and packaged startup do not establish packaged
edit/undo/save/relaunch/Open acceptance or clean-machine compatibility.

Use [desktop testing](DESKTOP_TESTING.md) to record those results and
[VS Code setup](NATIVE_DEVELOPMENT.md#run-from-vs-code-with-cmake-tools) to run the
development build. The browser starter is deferred and cannot satisfy desktop
acceptance.

## Completed checkpoints

These rows describe delivered behavior; dated handoffs retain each step's original
verification and limitations. Later checkpoints supersede earlier pending items.

| Checkpoint          | Delivered behavior                                                 | Record                                                        |
| ------------------- | ------------------------------------------------------------------ | ------------------------------------------------------------- |
| TypeScript contract | Schema-v1 persistence and provisional SDK contracts                | [contract](agents/handoffs/2026-09-27-contract-checkpoint.md) |
| N1                  | Copied QtNodes canvas and native shell                             | [N1](agents/handoffs/2026-09-27-native-checkpoint-n1.md)      |
| N2                  | Audited pipeline foundation and background execution adapter       | [N2](agents/handoffs/2026-09-28-native-checkpoint-n2.md)      |
| N3                  | Workspace, inspector and command-based edits                       | [N3](agents/handoffs/2026-09-30-native-checkpoint-n3.md)      |
| N4                  | Included primitive scene extension and viewer                      | [N4](agents/handoffs/2026-09-30-native-checkpoint-n4.md)      |
| N5a                 | Atomic schema-v1 project file codec                                | [N5a](agents/handoffs/2026-09-30-native-checkpoint-n5a.md)    |
| N5b                 | Retained-document execution projection                             | [N5b](agents/handoffs/2026-09-30-native-checkpoint-n5b.md)    |
| N5c                 | Structural retained project commands                               | [N5c](agents/handoffs/2026-09-30-native-checkpoint-n5c.md)    |
| N5d                 | Retained semantic undo/redo                                        | [N5d](agents/handoffs/2026-10-01-native-checkpoint-n5d.md)    |
| N5e                 | Canvas/inspector integration and unavailable placeholders          | [N5e](agents/handoffs/2026-10-01-native-checkpoint-n5e.md)    |
| N5f                 | Open/Save/Save As and dirty-state tracking                         | [N5f](agents/handoffs/2026-10-01-native-checkpoint-n5f.md)    |
| N5g                 | Layout/navigation, selection, pinned output and viewer persistence | [N5g](agents/handoffs/2026-10-01-native-checkpoint-n5g.md)    |
| N5h                 | Independent native table workflow                                  | [N5h](agents/handoffs/2026-10-01-native-checkpoint-n5h.md)    |
| N5i                 | Shipped examples and fresh-process project launch checks           | [N5i](agents/handoffs/2026-10-01-native-checkpoint-n5i.md)    |
| N6a                 | Reusable definitions and atomic expanded insertion                 | [N6a](agents/handoffs/2026-10-01-native-checkpoint-n6a.md)    |
| N6b                 | Selection extraction and catalog commands                          | [N6b](agents/handoffs/2026-10-02-native-checkpoint-n6b.md)    |
| N6c                 | Native authoring/library dialogs                                   | [N6c](agents/handoffs/2026-10-02-native-checkpoint-n6c.md)    |
| N6d                 | Collapsed snapshots, exposed controls/ports and grouped results    | [N6d](agents/handoffs/2026-10-02-native-checkpoint-n6d.md)    |
| N6e                 | Standalone component import/export                                 | [N6e](agents/handoffs/2026-10-02-native-checkpoint-n6e.md)    |
| N6f                 | Undoable catalog-entry removal, preserving instances               | [N6f](agents/handoffs/2026-10-02-native-checkpoint-n6f.md)    |
| IDE setup           | VS Code CMake Tools presets and native launch configurations       | [IDE](agents/handoffs/2026-10-02-vscode-cmake-tools.md)       |

## Next work

1. Collect manual packaged scene/data/component interaction and file round-trip
   results before marking full N5 accepted.
2. Scope component definition editing, version/migration policy and any explicit
   instance update mechanism before implementation. Preserve snapshot reproducibility.
3. Address demonstrated usability or correctness issues from manual testing in
   separate verified checkpoints.

Nested components, input fan-out interfaces, asset import/bundling/relocation,
production 3D rendering, broader scene hierarchies, richer table schemas and
CSV import remain unimplemented. Public installer/signing, distribution and
clean-machine Windows compatibility remain future work.

## Preserved future web prototype

`apps/web` is a React/TypeScript setup page, not a graph editor. `packages/core`
implements schema-v1 document persistence; `packages/extension-sdk` contains
provisional contracts. `packages/runtime` is reserved, and the domain extensions
have no TypeScript implementation. Browser graph execution, rendering, extension
loading and delivery remain deferred.

## Open decisions

Audience/market, project distribution license, broader extension packaging and
migration, asset storage, performance targets, additional operating systems and
remote services remain open. C++/Qt, the copied QtNodes canvas, the initial native
schema-v1 file contract and current source-level SDK are implemented choices;
their long-term compatibility/performance policies remain to be established.

See [architecture](architecture/README.md), [product proposal](product/SmartFlowProposal.md)
and the [historical handoff index](agents/handoffs/README.md). The old checkout is
never a build/runtime dependency and must remain unchanged.
