# Parallel web implementation plan

Updated: 2026-10-02. W1 planning/audit, W1b transport and W2a commands/registry are complete. Browser
editor and execution milestones below are pending. Native C++17/Qt6 remains the primary
implementation and the first user test delivery remains packaged Windows desktop.

## Goal and initial scope

Develop a browser workspace alongside desktop maintenance. The first usable web
preview must edit, execute and save/reopen the included primitive scene workflow
and an independent table workflow. Start with local browser computation and file
import/download, without accounts, a database or a remote execution service.

The existing React/TypeScript/Vite scaffold and npm workspace are the starting
point. A graph-canvas or rendering dependency must be selected in its implementation
checkpoint after reviewing current documentation, licensing and a small integration
spike. This plan does not select those dependencies.

## Parallel work and ownership

- Desktop work continues with packaged manual acceptance and demonstrated fixes.
  Browser checks cannot establish desktop acceptance.
- Web work changes `apps/web`, the TypeScript packages and browser extension
  sources. Keep native and browser implementations in distinct source directories.
- Coordinate project-format changes through a decision record and fixtures consumed
  by both languages before either app writes a changed contract. Use separate
  commits for completed, verified checkpoints; isolated worktrees are appropriate
  when separate contributors work concurrently.

Share persisted semantics, identifiers, examples and behavioral expectations.
Qt widgets and the C++ executor are separate implementations, not directly reusable
React/TypeScript libraries. Reuse reviewed domain behavior through matching tests;
do not introduce a native bridge or WebAssembly port without a concrete need.

Ownership remains: core for retained documents and commands; SDK for public
contracts; runtime for registry-independent validation/execution through SDK
contracts; extensions for domain behavior; web for composition and DOM adapters.
Runtime and SDK must not import concrete extensions. Add manifests to reserved
workspaces only when their implementation begins.

## Checkpoints and acceptance

| Checkpoint     | Deliverable                                                     | Acceptance evidence                                                                                                                                                                              |
| -------------- | --------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| W1 (complete)  | Plan, source audit and recorded compatibility probes            | [Audit](architecture/web-compatibility-audit-2026-10-02.md), documented gaps and follow-up gates                                                                                                 |
| W1b (complete) | Strict browser file transport and shared compatibility fixtures | Duplicate keys, depth/size boundaries and unsafe numeric tokens cannot silently corrupt imports; native and TypeScript agree on the supported subset; all shipped projects retain opaque content |
| W2a (complete) | Retained commands/history and extension lookup                  | Add/remove/connect/parameter edits undo and redo exactly; unknown fields and inactive graphs survive; workspace navigation is independent of semantic history                                    |
| W2b            | Browser library, canvas, inspector and file actions             | Create/edit/connect/delete, dirty-state indication, undo/redo and import/download/reopen work through browser interaction; unsupported nodes display placeholders                                |
| W3a            | Worker runtime and bundled data extension                       | Sample → Filter → Summary matches native example behavior; typed ports, DAG errors, failure, cancellation and stale-result rejection are tested; no scene dependency                             |
| W3b            | Execution controls/status and table viewer                      | Browser stays responsive during work; run/cancel and output selection work; obsolete results are identified; data graph reopens and executes                                                     |
| W4             | Bundled browser scene extension and interactive viewer          | Cube/Transform/Material/Scene/Merge example runs; camera navigation changes workspace; deliberate parameter edits use commands; scene project reopens and executes                               |
| W5a            | Component contracts, standalone files and collapsed execution   | Existing component example executes with named inputs/outputs/controls; snapshots survive catalog removal; unsupported/nested content is preserved and diagnosed                                 |
| W5b            | Component authoring and workspace restoration                   | Extraction, import/export, edit-as-copy and explicit compatible update have undoable behavior; saved browser layout and pinned outputs restore                                                   |
| W6             | Browser acceptance and delivery instructions                    | Both workflows pass interaction and file exchange checks in the documented supported browser set; production build and deployment instructions are verified                                      |

W2 depends on W1b. W3 follows W2, W4 follows W3, and W5 requires the retained
commands/runtime foundations. W6 requires both domains and the agreed component
scope. W3 is an intermediate data preview; required 3D support remains part of
the first complete web preview. Native parallel scheduling parity is not a gate
for the initial browser runtime; begin with deterministic dependency execution
inside a worker and measure before adding a worker pool.

## Compatibility rules

Keep schema-v1 envelope and persisted package/type/port IDs. Preserve unknown
extension fields, component catalogs/snapshots, assets and other graphs. Loading
does not repair or execute a graph. Resolve package/node contract versions before
execution and show unsupported capabilities explicitly. Opening a file does not
upload assets or trigger operations.

Browser state uses a versioned `smartflow.web-editor@1` workspace namespace and
retains `smartflow.native-editor@1` unchanged. Map portable positions, selection,
navigation and output references explicitly when initializing browser state from
a native file; do not interpret Qt layout or viewer payloads as browser settings.
Define viewer-state conversion in W4/W5 rather than silently rewriting it.

Until W5, component instances remain preserved placeholders and cannot execute.
W1b implements explicit rejection of unsafe integer-valued numbers and negative
zero before document conversion; see [decision 0019](architecture/decisions/0019-browser-json-transport.md).
It does not claim lossless import of arbitrary native files. W2 file actions must
leave the original file/current document intact and explain unsupported values.

Local filesystem asset paths are references, not browser permissions. Until asset
selection/bundling is implemented, retain them and report unavailable inputs.
Imported file plus downloaded replacement is the initial save workflow; optional
browser file handles and recovery storage are separate later enhancements.

## Verification and deferred scope

Run `npm.cmd run check` for web implementation changes, meaningful command/runtime
tests and browser interaction checks for UI changes. Contract changes also require
native CMake build/CTest and fixture acceptance on both sides. Record actual
browser versions, responsiveness checks and limitations in each handoff. File
exchange checks must exercise native save → browser edit/save → native reopen,
including unsupported content; parse-only tests do not prove editor preservation.

Accounts, cloud storage, remote execution, native bridges, arbitrary plugin loading,
collaboration, production asset import/rendering and a public deployment provider
remain outside this initial plan. Browser/desktop execution parity is limited to
explicitly implemented nodes and versions. Audience and application field stay open.

W2a commands/history and immutable bundled extension lookup are complete;
see [the checkpoint](agents/handoffs/2026-10-02-web-w2a-history.md).
Next implementation task: W2b browser canvas, inspector and file actions.
[W1b evidence](agents/handoffs/2026-10-02-web-w1b-transport.md) records transport
tests; browser file interaction and editing remain unimplemented.
