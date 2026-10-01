# Native graph components

Status: model/command foundation, N6a. End-user authoring and collapsed component
nodes are not available yet. A component currently inserts a reusable subgraph as
ordinary nodes through an atomic document command.

## Definition contract

A retained definition has these fields:

| Field                      | Meaning                                       |
| -------------------------- | --------------------------------------------- |
| `format`                   | `smartflow.graph-component`                   |
| `schemaVersion`, `version` | Both 1 for the initial contract               |
| `id`, `title`              | Nonempty identity and display title           |
| `graph`                    | Project-style graph ID, nodes and connections |
| `inputs`                   | Array of `{id, target: {nodeId, portId}}`     |
| `outputs`                  | Array of `{id, source: {nodeId, portId}}`     |
| `controls`                 | Array of `{id, target: {nodeId, parameter}}`  |

The internal graph must be nonempty and acyclic. Endpoints must reference nodes
in the body. An exposed input cannot already have an internal producer. Exposed
input/control targets cannot be repeated. Controls must reference saved body
parameters. Unknown fields are retained, including integer values up to the
project codec's 64-bit limits. The component model does not infer domain types.

Definitions live in `project.components`, an optional retained array. Older
codecs preserve it without interpreting it. A future/malformed definition can
stay in a loaded document; explicit instantiation reports unsupported content
instead of dropping it. Do not modify a definition under an existing ID/version.

## Command API

Construct `project::GraphComponent` with a retained definition. Then call
`DocumentHistory::instantiateComponent(component, instanceId, controls, inputs)`.
`controls` is an object mapping exposed IDs to override values. Omitted controls
use body defaults. `inputs` maps every required exposed input ID to an external
`{nodeId, portId}` endpoint in the original destination graph.

The result supplies `outputs` and `controls` objects containing remapped endpoints.
Use existing connection/parameter commands for subsequent edits. The inserted
nodes receive stable IDs in an instance namespace. A second instance has separate
parameters/results. Reusing an instance identity fails if it collides with nodes
or edges already in the destination graph. Catalog insertion and body expansion
undo as a single semantic command.

The registered nodes enforce parameter types/bounds and connection types. Newly
introduced unavailable nodes, bad ports or missing required inputs reject the
command without changing the active document or redo history. Unrelated existing
unknown content remains retained and continues to block execution.

## Verification and next work

`native_components` tests two table filter/summary instances with distinct
thresholds and outputs, parameter edits, save/reopen, exact definition/opaque
retention, atomic undo/redo and rejection. Neither the component library nor its
data-based tests link scene-domain libraries.

Next: component-authoring/library commands and UI, then a collapsed instance
node with exposed parameter controls and execution/result mapping. Recursive
components, definition migration, live catalog updates and external distribution
remain later work. See [decision 0012](architecture/decisions/0012-native-graph-component-foundation.md).
