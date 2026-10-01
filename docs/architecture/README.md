# Architecture boundaries

## Current implementation direction

Native C++/Qt is primary, per [decision 0003](decisions/0003-native-first.md).
`native/app` composes the application and links the copied QtNodes library in
`native/vendor`. N2 adds selected pipeline/data/task sources and a separate
`native/app/pipeline` background execution adapter. It has no QtNodes or scene
dependency. N3 adds the project/canvas adapter, command-based inspector and
revision-aware execution controller. N4 adds source-level native contribution
contracts in `native/sdk` and the bundled extension in `extensions/scene-3d/native`.
The application composition root links it; workspace and execution code do not.
The extension reuses audited geometry/material math with a bounded primitive
preview. Broader scene and production renderer migration remain pending. See
[decision 0004](decisions/0004-native-pipeline-migration.md) and
[decision 0005](decisions/0005-native-workspace.md) and
[decision 0006](decisions/0006-native-scene-extension.md). The old checkout is
not a dependency. The TypeScript package diagram below
describes the preserved future-extension prototype, not the native build.

N5a adds `native/app/project/ProjectFile`, a Qt Core/JSON-only file layer sharing
the schema-v1 envelope with the preserved prototype. It retains unknown content
and performs atomic file writes. N5b adds the retained `DocumentSession` execution
adapter and explicit persisted identities in native registrations. N5c/N5d add
structural commands and retained-document undo; N5e connects them to the editor
and adds unavailable-node placeholders. N5f adds editor file actions and
retained-content dirty tracking; workspace persistence remains pending.
See [decision 0007](decisions/0007-native-project-files.md) and the
[editor integration decision](decisions/0008-retained-editor-commands.md).

## Product invariant

3D scene support ships with the product, while the core remains usable without a 3D data model. Prove this with an independent data/text extension.

## Planned dependency direction

```text
apps/web (composition root)
  -> packages/core
  -> packages/extension-sdk
  -> packages/runtime
  -> extensions/scene-3d
  -> extensions/data

packages/extension-sdk -> packages/core
packages/runtime -> packages/core + packages/extension-sdk
extensions/* -> packages/core + packages/extension-sdk
```

The native application explicitly composes the included scene extension. Core, SDK, and runtime never import concrete extensions. Rendering integrations belong in viewer contributions, not the graph model. The TypeScript core implements document persistence and its SDK provides initial contracts; its runtime and domain implementations remain planned.

## Separate kinds of state

- Project: graph nodes, connections, parameters, assets, and package dependencies.
- Workspace: panel layout, pinned outputs, selection, and navigation camera.
- Execution: node status, progress, cached results, cancellation, and errors.
- Domain output: scene hierarchy, table, image, or other typed result.

The execution graph and scene hierarchy are different structures. Editing an output must have a defined route back to project commands. Viewing an output must not implicitly rewrite graph parameters.

## Extension contract to design

Define stable identifiers, versions, dependencies, custom types, typed nodes, parameter editors, viewers, runtime capabilities, and saved-state migration. Use a minimal working 3D package to discover the contract, then test it with an independent domain before expanding the API.

Persist unknown nodes and their data losslessly when an extension is absent. Validate before execution and explain missing capabilities. Do not promise that all extensions run in every environment.

## Execution direction

Start with acyclic dataflow, explicit live-versus-manual execution, and inspectable results. Introduce worker, native, or remote execution behind explicit adapters when needed. Streaming and simulation are future semantic decisions, not implied support.

## Decisions

See [0001: bootstrap](decisions/0001-bootstrap.md) and [0002: project contract](decisions/0002-project-contract.md). Create a new decision record for consequential changes rather than silently rewriting settled rationale.
