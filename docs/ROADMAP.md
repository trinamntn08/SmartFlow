# Roadmap and current state

## Available now

- Native C++/Qt is the primary implementation direction; TypeScript is retained for a future extension.
- First native migration: copied QtNodes canvas, independent CMake build, numeric graph preview and native tests.
- Pipeline runtime, inspector and 3D modules have not yet been copied.
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
2. Next, N2: audit and copy pipeline/data/task dependencies with licenses, test node/port and execution behavior.
3. Copy and adapt the pipeline inspector/workspace; test parameter edits and undo; record N3.
4. Copy the required 3D modules and their audited dependencies; test primitive/transform/material/viewer behavior; record N4.
5. Establish native project persistence, unknown-extension preservation and a non-3D workflow; record N5.

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
