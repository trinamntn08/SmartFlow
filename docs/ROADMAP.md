# Roadmap and current state

## Available now

- Project name and branding: SmartFlow.

- Independent local repository and product proposal.
- Shared agent instructions and development workflow.
- React/TypeScript browser starter with npm workspaces.
- Formatting, strict type checking, production build, and editor tasks.
- Documented platform and extension ownership.

The starter is a setup page. It does not yet implement graph editing, scene rendering, execution, or extension loading.

## Next: define and implement the first vertical slice

1. Specify the graph/project document and minimal node/type/viewer contracts.
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
- Whether desktop packaging or remote services are needed.

Follow [the product proposal](product/SmartFlowProposal.md) for requirements. The bootstrap framework is replaceable and does not settle the final product platform.
