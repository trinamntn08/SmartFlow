# 0012: Native graph component foundation

Status: accepted for N6a, 2026-10-01.

## Decision

Implement a registry-independent GraphComponent model in `native/app/project`
with format `smartflow.graph-component`, schemaVersion 1 and contract version 1.
A definition owns a retained graph and named exposed inputs, outputs and controls.
Inputs/outputs bind to node/port endpoints; controls bind to node/parameter targets.

Keep the schema-v1 project envelope unchanged. Store definitions in the optional
`project.components` array, which old/native/browser codecs already preserve as
unknown content. The component model validates only definitions explicitly passed
to it; loading a file must never drop or rewrite unavailable catalog entries.
A matching component ID/version must have identical content, otherwise insertion
fails. Contract/version migration and external component packages remain future.

N6a instantiation expands ordinary nodes and edges into the selected graph. It
returns remapped exposed output/control bindings and wires required external
inputs. Length-prefixed instance namespaces keep IDs deterministic and distinct
when user IDs contain separators. Reject collisions instead of renaming existing
content. Definitions, opaque node/edge/endpoint fields, other graphs and workspace
state remain retained. Instance controls override only their copied body.

DocumentSession prepares the complete replacement with the active extension
registry, rejects newly introduced execution diagnostics and checks exposed output
ports before publishing. Existing unsupported content stays retained and continues
to block execution. DocumentHistory wraps expansion/catalog insertion in one
semantic command, so rejected commands preserve the undo redo branch. Undo retains
current workspace, per the existing document-history agreement.

## Boundaries and limits

N6b (2026-10-02) adds explicit selection extraction and standalone catalog
commands within these boundaries. Extraction copies selected nodes/internal edges
and retained graph metadata; every connection across the selection boundary must
have a caller-declared exposed endpoint. It leaves the source graph unchanged.
Catalog storage is structural and permits unavailable node packages; active
registration checks remain at instantiation. Identical catalogs are no-ops,
identity/version conflicts reject, and extraction/catalog storage undo atomically.
This provides an authoring API without changing project schema or execution.

The components library depends only on the project codec/Qt Core; it has no SDK,
canvas, renderer or domain dependency. The session/history adapters use public
registrations. Tests instantiate the data extension without scene libraries.

This is reusable subgraph insertion, not a collapsed component node, binary
plugin system or recursive component runtime. Required inputs are single-target;
fan-out input bindings and nested component instances are not implemented. Port
types and parameter bounds come from the active node registrations. Instances are
independent copies; changing a catalog definition does not update prior instances.
N6c (2026-10-02) adds modal authoring and library dialogs in the native workspace.
The author chooses named interfaces from active registrations; boundary endpoints
are required. The library retains unknown entries, filters external input sources
by port type and provides numeric controls using registered bounds. IDs are
generated automatically. Dialog acceptance invokes retained project commands;
cancellation/validation failures do not mutate the project. Graph revisions guard
against stale dialog edits. Inserted copies are placed beside existing nodes and
framed through workspace state. This does not add a component node type or change
execution semantics. A collapsed-node exposed-control inspector remains pending.
Full N5 desktop acceptance remains independently pending the computer-use helper.

## Subsequent checkpoints

N6d implements collapsed snapshot nodes and exposed-control inspection through
[decision 0013](0013-collapsed-native-components.md). N6e/N6f add standalone exchange
and undoable catalog removal through [decision 0014](0014-native-component-files.md).
The earlier pending items above record the foundation checkpoint's scope. Full N5
acceptance is now pending the user's manual desktop results; native computer-use
is deferred. See [the current roadmap](../../ROADMAP.md).
