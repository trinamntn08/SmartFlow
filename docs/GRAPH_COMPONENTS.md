# Native graph components

Status: native authoring/library UI, N6c. Collapsed component nodes are not
available yet. A component currently inserts a reusable subgraph as
ordinary nodes through an atomic document command.

## Desktop workflow

1. Select nodes on the canvas (use Ctrl to add nodes to the selection), then open
   **Components > Create from selection**.
2. Give the component a title. The Inputs, Outputs and Controls tabs list
   available endpoints. Boundary connections are required and checked; give each
   exposed endpoint a unique name within its tab. Free inputs and terminal outputs
   are checked initially. Choose any additional outputs and parameter controls.
3. Click **Create component** to save it in the current project's library. The
   original nodes remain unchanged. Creation is one undoable project command.
4. Open **Components > Component library**, choose a saved component, select a
   compatible source output for each input, and adjust exposed numeric controls.
   Other parameter types retain their saved defaults.
5. Click **Insert component**. A separate copy of its ordinary nodes appears beside
   the existing graph, selected and framed in the canvas. Connect its output nodes,
   select them to inspect results or pin their output in the viewer. Insertion is
   one semantic undo command; positions and navigation are workspace state.
6. Save and reopen the project to retain both the library and inserted copies.

Identifiers are generated automatically. Validation errors stay in the dialog,
allowing correction without changing the project. Cancellation makes no edit.
Unavailable or malformed catalog entries stay visible/retained and report errors;
they cannot be inserted. Creating a component from unavailable selected nodes is
disabled. Dialogs reject edits if the graph changes while they are open.

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

## Extraction and catalog commands

`GraphComponent::extract(source, graphId, nodeIds, id, title, inputs, outputs,
controls)` copies a nonempty, unique node selection and its internal connections.
The caller supplies the interface arrays from the definition contract. Every
incoming boundary connection must expose its selected target; every outgoing
boundary connection must expose its selected source. Multiple outgoing edges can
share one exposed output. Invalid selections or incomplete boundaries reject.
Extraction leaves the original graph untouched, retaining its opaque metadata,
selected nodes and internal edges. The copied body graph ID becomes `body`.

`DocumentHistory::extractComponent(...)` extracts from the active graph and saves
the definition as one undoable catalog command. `catalogComponent(component)`
also stores an independently authored definition without inserting nodes. Saving
an identical definition is a no-op; conflicting ID/version content rejects before
changing the redo branch. Undo keeps current workspace state.

Catalog storage validates structure without requiring installed node packages.
Unavailable definitions remain retained and can be reused after their packages
become available. Instantiation still checks active registrations, typed ports
and parameter bounds. Extraction does not infer ports, automatically expose
parameters, replace selected nodes or copy boundary-edge metadata into bindings.

## Verification and next work

`native_components` tests two table filter/summary instances with distinct
thresholds and outputs, parameter edits, save/reopen, exact definition/opaque
retention, atomic undo/redo and rejection. Neither the component library nor its
data-based tests link scene-domain libraries.

N6b also checks extraction, boundary validation, catalog conflicts/no-ops,
workspace-preserving undo/redo and real file reopening followed by execution.

`native_component_ui` drives the actual menu/dialog workflow through Qt in the
independent data app. It verifies exposed endpoints, separate control values,
execution, non-overlapping placement, undo/redo, save/reopen/library reuse,
compatible-input filtering, cancellation and unavailable-content retention.

Next: a collapsed instance
node with exposed parameter controls and execution/result mapping. Recursive
components, definition migration, live catalog updates and external distribution
remain later work. See [decision 0012](architecture/decisions/0012-native-graph-component-foundation.md).
