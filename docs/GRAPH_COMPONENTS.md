# Native graph components

Status: implemented through N6h: authoring, collapsed instances, file exchange
and undoable library removal. Components
insert as one node by default, with ordinary-node insertion also available.

## Desktop workflow

Collapsed instances also provide the controls and outputs for native
[Use mode](USE_MODE.md), a simplified tool interface sharing the same project,
execution and undo history.

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
5. Leave **Insert as one node** checked and click **Insert component**. The
   component appears beside the graph with its exposed ports and controls. Select
   it, choose an **Output** in the toolbar and click **Pin output**. Connect its
   exposed ports like ordinary nodes. Uncheck the option to insert separate body
   nodes instead. Insertion is
   one semantic undo command; positions and navigation are workspace state.
6. Save and reopen the project to retain both the library and inserted copies.

Identifiers are generated automatically. Validation errors stay in the dialog,
allowing correction without changing the project. Cancellation makes no edit.
Unavailable or malformed catalog entries stay visible/retained and report errors;
they cannot be inserted. Creating a component from unavailable nodes or existing component instances is
disabled; nested definitions are unsupported. Dialogs reject edits if the graph changes while they are open.

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
`DocumentHistory::instantiateComponent(component, instanceId, controls, inputs, collapsed)`.
`controls` is an object mapping exposed IDs to override values. Omitted controls
use body defaults. `inputs` maps every required exposed input ID to an external
`{nodeId, portId}` endpoint in the original destination graph.

The result supplies `outputs` and `controls` objects containing remapped endpoints.
Use existing connection/parameter commands for subsequent edits. The API defaults `collapsed` to false for compatibility; the desktop library
defaults it to true. Expanded body nodes receive stable IDs in an instance namespace. A second instance has separate
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

N6d tests collapsed table and scene instances, exposed controls and output aliases,
chained connections, grouped failures/cancellation, cycle rejection, undo/redo,
opaque unavailable instances and save/reopen. The shipped `components.smartflow`
example restores the summary output and minimum control in the data-only app.

## Collapsed instance contract

A schema-v1 project retains one node with `packageId: smartflow.components`,
`typeId: instance`, `version: 1`, exposed control values in `parameters`, and the
complete definition snapshot in `component`. The saved snapshot governs execution,
independently of catalog availability or later catalog changes. Unknown fields
and unavailable/malformed instances remain retained, with diagnostics blocking
execution. Editing a control changes its instance parameters, not its snapshot.

The session expands a disposable execution graph and presents a facade for exposed
ports and controls to the canvas/inspector. Results collapse back to visible node
identities and share exposed members without renaming their data. All exposed
inputs gate the body; downstream nodes wait for all body steps, including branches
that do not provide an exposed output. A failed body publishes no instance output.
Both visible and expanded graphs must be acyclic. Output choice/pinning is saved
in workspace `selectedPort`/`pinnedPort`, separately from project parameters.

Nested components, definition migration and live updates remain future work.
[Editing scope](architecture/decisions/0015-component-editing-scope.md) implements
fresh-ID copies and explicit compatible snapshot replacement.
This is a native migration feature, not a production
plugin system. See [decision 0013](architecture/decisions/0013-collapsed-native-components.md).

## Import and export

Open **Components > Component library** and click **Import component...** to read
a `.smartflow-component` or `.json` file into this project's library. Import works
when the library is empty. It adds no canvas nodes; choose bindings and insert the
component afterward. A new catalog entry is one undoable edit. Importing identical
content is a no-op; conflicting content under the same ID/version rejects.

Select a definition and click **Export component...** to save its standalone file.
Export does not change the project, active project path, dirty state or history.
The file contains the definition's saved defaults and opaque fields; current
instance control edits and workspace state are separate. Copying a file does not
bundle external assets or relocate asset references. Try the shipped
`examples/filtered-summary.smartflow-component` in another data project.

Unavailable node packages do not prevent structural import/export. Their
components remain in the library but cannot be inserted until supported. Malformed
or future definition contracts report errors. Cancel and failed operations preserve
project state; writes use atomic replacement. Closing the library does not undo
imports already completed; use Undo for those catalog edits. Files have the same strict JSON,
16 MiB and nesting limits as project files. See
[decision 0014](architecture/decisions/0014-native-component-files.md).

## Remove a library entry

Select an entry in **Components > Component library**, click **Remove from
library...** and confirm. The default response is No. Removal changes this
project's catalog only; it does not delete files or alter any graph/instance
snapshot. Existing collapsed instances still execute and reopen without a catalog.
Undo restores the complete entry, including unknown fields, in its original order.

Unavailable and malformed entries in a catalog array can be explicitly removed.
An unsupported catalog container is preserved and cannot be edited through this
operation. Empty or stale selections reject without changing project/history.
`DocumentHistory::removeCatalogComponent(index)` removes exactly the selected
array entry as one semantic command; workspace changes remain separate.

## Edit a copy

Choose a supported definition in the library and click **Edit a copy...**.
The separate draft workspace provides the same node canvas, inspector and local
Undo/Redo as the main window. Edit nodes, connections and parameter defaults.
Click **Save copy...** to choose the title and exposed interface; existing names
are prefilled where their endpoints still exist. Saving adds a definition under
a fresh ID as one undoable project edit. The source entry and all existing
instances remain unchanged. Cancel discards draft edits.

Exposed inputs are unbound in the draft; insert the saved copy with source bindings
to test its results. All required unconnected inputs must be exposed before
publication. Invalid ports/parameters, unavailable nodes, nested components and
stale destination revisions reject without changing the destination project.
Version fields remain 1; a saved copy does not migrate an existing definition.
Unknown fields on retained body objects and the definition survive, as does
metadata on interface entries whose endpoints remain exposed.

## Update one instance

Select one collapsed node and use **Components > Update selected instance...**.
Choose a library definition and review the current/chosen body nodes, defaults,
connections and interface. **Apply** replaces only this instance's snapshot as
one undoable command. Other instances, library entries and expanded copies are
unchanged. Existing control values, external connections, node identity and
viewer output aliases remain in place; new saved defaults do not reset overrides.

Both definitions must be supported and expose the same input/output/control
names. Port/control types must match, current values must fit the new parameter
bounds, and the updated graph must validate. Incompatible choices show an inline
error and disable Apply. Changed content needs a different definition ID. Cancel
and rejected/stale edits preserve the project and redo branch; identical content
is a command no-op. There are no automatic, bulk or interface-remapping updates.
`DocumentHistory::replaceComponentInstance(nodeId, component)` is the command API.
