# Architecture boundaries

Updated: 2026-10-02. Native C++17/Qt6 is primary, per
[decision 0003](decisions/0003-native-first.md); the first test delivery is Windows,
per [decision 0009](decisions/0009-desktop-test-release.md).

## Current native implementation

| Area                         | Ownership and implemented behavior                                                                                                                  |
| ---------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| `native/app/main.cpp`        | Composition of the required scene extension and optional data configuration; separate data-only executable excludes scene libraries                 |
| `native/app/project`         | Strict schema-v1 files, retained documents, commands/history, component definitions/snapshots, standalone files and disposable execution projection |
| `native/app/pipeline`        | Background execution adapter over audited pipeline/data/task sources; validation, cancellation and grouped component results                        |
| `native/app/workspace`       | Qt canvas/inspector adapters, file actions, component dialogs, viewer selection and workspace state                                                 |
| `native/sdk`                 | Source-level node/type/factory/viewer contribution contracts, without concrete domain imports                                                       |
| `extensions/scene-3d/native` | Scene types/nodes and bounded interactive primitive preview using audited geometry/material math                                                    |
| `extensions/data/native`     | Independent table types/nodes and viewer using the same contracts                                                                                   |
| `native/vendor`              | Copied sources, preserved licenses/provenance and isolated local patches; no old-checkout dependency                                                |
| `native/tests`               | Behavior, Qt UI, file round-trip, failure/cancellation and startup checks                                                                           |

The composition root registers contributions. Workspace, document and execution
adapters do not import a concrete domain. `smartflow-data` validates this by linking
without scene or scene-math libraries. The SDK and adapters are native migration
features; no dynamic plugin loader or production extension runtime exists.

## Retained state and execution

The schema-v1 file codec retains unknown JSON and other graphs, uses strict numeric
and structural validation, and writes atomically. The editor opens the first graph;
unsupported content displays placeholders/diagnostics and blocks execution rather
than being silently dropped. Definitions and standalone files share strict JSON
transport; schema validation stays with their respective adapters.

Separate state responsibilities:

- Project: graph nodes/connections, parameters, component catalogs/snapshots,
  package metadata and asset references.
- Workspace: canvas positions/navigation, selection, selected/pinned output aliases
  opaque viewer state, selectable widget regions and divider proportions.
  Outer window geometry is not yet persisted.
- Execution: disposable compiled steps, worker snapshots, status/errors and computed
  outputs. Results are not serialized; general caching is not implemented.
- Domain output: immutable-by-convention scene/table values supplied by extensions.

Viewer navigation/selection saves workspace state. Parameter edits use explicit
project/history commands. Undo of semantic edits preserves the current workspace.
No domain object is required in an empty project.

The executor validates an acyclic graph, runs off the UI thread, propagates failures
and discards cancelled output. Revision checks prevent stale runs replacing current
results. The adapter now supports bounded sequential/parallel scheduling through
explicit options, with one coordinator, private cloned invocation data and
conservative delegate/resource policies. Native controls select sequential/parallel
mode and thread limits; synchronized live node/component progress is published on
the Qt thread with stale-run rejection. See [execution](../EXECUTION.md).
Managed node-internal helpers share reserved per-run capacity and join their
workers before returning; nested helper work is serial.
See the [execution architecture review](execution-review-2026-10-02.md) for evidence,
required changes and acceptance tests. Streaming, simulation, remote execution and
production scheduling remain future decisions.

## Reusable components

Definitions expose named inputs, outputs and controls. The library supports
selection extraction, catalog storage, collapsed insertion by default and optional
expanded copies. A collapsed node retains a complete definition snapshot and
separate control values; it executes independently of subsequent catalog changes
or removal. Compilation expands private body steps and creates canvas/inspector
facades from public registrations.

Body dependency barriers wait for all external inputs; external consumers wait for
all body steps, including unexposed branches. Grouped results share exposed members
without renaming their data. Both visible and expanded dependency cycles reject.
Nested components and exposed input fan-out are unsupported.

Standalone import adds a catalog entry through one history command. Identical
content is a no-op and conflicting identity/version content rejects. Export and
library removal do not update instance snapshots. Removal targets one array entry
in the current revision and is undoable, including opaque entries. External asset
bundling/relocation, definition migration and live updates are unimplemented.
The library edits supported definitions in an isolated draft and saves fresh-ID
copies, preserving existing snapshots. An explicit one-instance command adopts a
supported definition with matching interface names/types, preserving existing
control values, connections and workspace aliases. Incompatible choices reject;
undo restores the previous snapshot. No automatic update or migration exists.

See [the component guide](../GRAPH_COMPONENTS.md) and decisions
[0012](decisions/0012-native-graph-component-foundation.md),
[0013](decisions/0013-collapsed-native-components.md) and
[0014](decisions/0014-native-component-files.md).

## TypeScript prototype and planned web track

`apps/web` now contains a retained graph editor with React Flow, parameter inspection
and strict file import/download (W2b). Core owns persistence/commands/history and
SDK owns immutable bundled registration. The data and scene source workspaces own
editor metadata and data execution. `packages/runtime` provides typed DAG execution; browser rendering and
component authoring are pending. The intended dependency direction
is application composition to core/SDK/runtime/extensions, with SDK depending only
on core and runtime depending only on core plus SDK. Concrete extensions never
become core/runtime dependencies.

[Decision 0018](decisions/0018-parallel-web-development.md) activates a parallel
web track without changing native delivery priority. W1 is documentation/audit
only; [the plan](../WEB_IMPLEMENTATION_PLAN.md) defines implementation checkpoints
and [the compatibility audit](web-compatibility-audit-2026-10-02.md) records
original transport gaps. W1b now implements strict core JSON preflight and a web
UTF-8 byte adapter, with shared native/browser fixtures and explicit numeric
rejection under [decision 0019](decisions/0019-browser-json-transport.md).
W2a adds retained core commands/history and immutable SDK extension lookup.
Workspace updates stay outside semantic undo/redo. W2b editing/file actions are
implemented under [decision 0020](decisions/0020-browser-canvas.md). W3a worker
execution is implemented; W3b controls and table viewer are implemented; W4 scene capability is complete; W5a snapshot execution is complete; W5b authoring is complete; W6 delivery acceptance is next. See [the web guide](../WEB_DEVELOPMENT.md).

## Verification and open work

Current Release builds, 24 CTest entries and packaged startup checks passed. Full
Windows desktop interaction acceptance is pending manual testing, as recorded in
[the roadmap](../ROADMAP.md). Automated native computer-use is deferred by the user.
The primitive viewer is not production GPU rendering; importer/asset handling,
performance policy, public packaging/signing and broader compatibility are open.

## Decision records

[Decision records](decisions/README.md) retain rationale at the date of each step;
follow-ups/current guides supersede historical pending items. The initial legacy
audit records evidence, not current implementation recommendations. Consult the
[roadmap](../ROADMAP.md) for delivered status and
[product proposal](../product/SmartFlowProposal.md) for product constraints.
