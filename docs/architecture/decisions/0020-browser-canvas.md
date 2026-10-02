# 0020: Retained browser editor with React Flow

Date: 2026-10-02. Status: implemented in W2b.

## Context and decision

The browser editor needs typed named handles, selection, dragging, navigation and
keyboard interaction without owning persisted semantics. Use exactly
`@xyflow/react` 12.12.0, an MIT dependency with React ≥17 peer support. Reviewed
[custom nodes](https://reactflow.dev/learn/customization/custom-nodes),
[TypeScript integration](https://reactflow.dev/learn/advanced-use/typescript),
[connection validation](https://reactflow.dev/examples/interaction/validation),
and the [license](https://github.com/xyflow/xyflow/blob/main/LICENSE).
The local integration spike typechecks/builds and uses custom named handles.

React Flow nodes/edges are disposable UI projections. Retained core commands own
add/delete/connect/parameter edits; UI state never reconstructs the saved document.
Unavailable nodes display placeholders with inferred existing ports. Dangling edges
stay in the document and connection list even when they cannot render on canvas.
New connections require supported matching port types, free input occupancy and
an acyclic result. Unsupported package/type versions remain preserved.

Data and scene packages now own TypeScript editor metadata matching the native
IDs/defaults/ranges. Their browser execution capabilities remain empty until W3/W4.
This is source-level bundled composition, not a production plugin loader.

## Workspace and file behavior

Use `smartflow.web-editor@1`, retaining native workspace JSON unchanged. Initialize
portable positions/selection/pinning from native state. Qt scene-center navigation
maps explicitly to React Flow viewport translation using canvas dimensions and
scale. Qt panel/viewer payloads are not browser state. Unknown browser workspace
objects retain fields; incompatible container shapes reject edits.

Import uses strict original-byte transport. Failed imports preserve the current
document; unsaved replacement prompts to download or discard. Download prepares
a complete schema-v1 file with all inactive graphs/assets/opaque fields retained.
No local asset path grants browser access. Opening never executes a graph.

## Verification

Playwright 1.63.0 is an Apache-2.0 development dependency for isolated application
browser tests, with browser binaries inside ignored repository cache. Tests use
the [documented input](https://playwright.dev/docs/input),
[download](https://playwright.dev/docs/downloads) and
[local web server](https://playwright.dev/docs/test-webserver) APIs. This does not
control the user's browser profile or prove manual desktop acceptance.
See the W2b handoff for actual browser version, results and rendering inspection.
