# Architecture boundaries

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

The application explicitly composes bundled extensions. Core, SDK, and runtime never import concrete extensions. Rendering integrations belong in viewer contributions, not the graph model. These are planned boundaries; only the web starter currently has executable code.

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

See [0001: bootstrap](decisions/0001-bootstrap.md). Create a new decision record for consequential changes rather than silently rewriting settled rationale.
