# Roadmap and current state

## Available now

- Native C++/Qt is the primary implementation direction; TypeScript is retained for a future extension.
- First native migration: copied QtNodes canvas, independent CMake build, numeric graph preview and native tests.
- N2: selected pipeline/data/task sources copied with licenses and provenance;
  background execution adapter validates ports/dependencies, rejects cycles,
  propagates failures, isolates graph snapshots and supports cancellation.
  See [checkpoint N2](agents/handoffs/2026-09-28-native-checkpoint-n2.md).
- N3: canvas/inspector integration with parameter commands, undo/redo,
  background execution, live/manual runs, cancellation, and current-result
  inspection. See [checkpoint N3](agents/handoffs/2026-09-30-native-checkpoint-n3.md).
- N4: included native scene-3d extension with copied geometry/material math,
  Cube/Transform/Material/Scene/Merge nodes, and orbit/zoom/selection preview.
  See [checkpoint N4](agents/handoffs/2026-09-30-native-checkpoint-n4.md).
- Native persistence and production 3D rendering remain pending. The native work is
  a migration foundation, not a production runtime or plugin loader.
- N5a implements the standalone native schema-v1 file codec and atomic file I/O,
  preserving unknown JSON content without extension registration. Editor Save/Open,
  missing-node handling and the independent non-3D extension are still pending.
  See [N5a checkpoint and next-session handoff](agents/handoffs/2026-09-30-native-checkpoint-n5a.md).
- N5b adds explicit native node identities and a retained-document execution
  adapter, with transactional parameter edits and unsupported-content diagnostics.
  Editor Save/Open, missing-node display and the non-3D extension remain pending.
  See [N5b handoff](agents/handoffs/2026-09-30-native-checkpoint-n5b.md).
- Project name and branding: SmartFlow.

- Independent local repository and product proposal.
- Shared agent instructions and development workflow.
- React/TypeScript browser starter with npm workspaces.
- Formatting, strict type checking, production build, and editor tasks.
- Documented platform and extension ownership.
- Checkpoint 1: versioned project persistence and initial SDK contracts, with behavior tests.
  See [checkpoint record](agents/handoffs/2026-09-27-contract-checkpoint.md).

The browser starter is a setup page. It does not yet implement graph editing, scene rendering, execution, or extension loading.

## Next: define and implement the first vertical slice

The list below records the earlier TypeScript plan. It is superseded for current
implementation by this native migration sequence:

1. Completed N1: copied QtNodes canvas and native shell, with build/tests.
2. Completed N2: selected pipeline/data/task dependencies with licenses and native execution behavior tests.
3. Completed N3: copy selected pipeline widgets, compose the workspace/inspector, connect the execution adapter, and test parameter edits and undo.
4. Completed N4: copy the audited geometry/material foundation and dependencies; implement/test the included native primitive/transform/material/viewer slice. Production rendering and broader scene capabilities remain future work.
5. In progress, N5: file codec (N5a) and retained-document execution adapter (N5b) completed; next connect editor save/reopen and unknown-node display, then implement a separate non-3D extension workflow. Record full N5 only after acceptance checks pass.

Each checkpoint must include actual build/test results and remaining limitations.

### Earlier TypeScript prototype sequence (deferred)

1. Initial graph/project document and minimal node/type/viewer identity contracts implemented;
   validate and extend them during the runtime and viewer checkpoints.
2. Add a graph canvas and inspector without coupling persistence to the UI library.
3. Implement a minimal acyclic runtime and meaningful behavior tests.
4. Implement the included 3D package: primitive, transform, material, scene assembly, and viewer.
5. Connect a parameter edit to recomputation and the scene preview.
6. Save/reopen a project and preserve unsupported extension nodes.

Acceptance: create a scene graph, adjust a transform, see the updated scene, and save/reopen it with consistent parameters and viewer state.

## Then: prove extensibility

- Add a table/text workflow through the same SDK.
- Run that workflow without loading or initializing the 3D package.
- Package a graph as a reusable component.
- Check missing package and unsupported backend behavior.

## Later, when justified

- Asset import and larger scene performance.
- Worker execution, caching, and more detailed progress/cancellation.
- Desktop integration or remote execution for demonstrated needs.
- Broader domain packages and simplified use mode.

## Open decisions

- Audience, market, and license.
- Node canvas and 3D rendering libraries.
- Project schema, extension packaging, and compatibility policy.
- Browser storage and asset strategy.
- First-release performance targets and supported browsers.
- Native distribution packaging and whether remote services are needed.

Follow [the native-first decision](architecture/decisions/0003-native-first.md) for the approved implementation direction and [the product proposal](product/SmartFlowProposal.md) for domain requirements.
