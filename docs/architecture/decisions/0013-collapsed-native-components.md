# 0013: Collapsed native component snapshots

Date: 2026-10-02
Status: accepted for the native migration preview (N6d)

## Context

N6a–N6c retain reusable definitions and insert ordinary node copies. A reusable
tool also needs one visible node with exposed ports, editable controls and viewer
outputs. The retained graph must remain independent of QtNodes and domain types.

## Decision

Persist a schema-v1 node with package `smartflow.components`, type `instance`,
version 1, exposed values in `parameters`, and a complete definition snapshot
in `component`. Instances use their snapshot even when the catalog is absent;
there is no automatic catalog update. Existing expanded insertion remains an
API default, while the desktop library defaults to collapsed insertion.

The registry-independent component model remaps body identities. A project
adapter expands a disposable execution document, rewrites external bindings and
builds detached inspection facades from active registrations. The canvas owns a
display-only port interface; no component execution delegate or domain import
is required. Retained state is never rewritten by compilation.

The execution adapter snapshots result groups along with the graph. Public
pipeline contexts add dependency barriers: each body member waits for all external
inputs, and external consumers wait for every body member. Failures in unexposed
branches therefore block consumers too. Validate both expanded and grouped DAGs
before execution. Keep vendor sources unchanged.

Aggregate hidden results to the visible instance. Share exposed output members
without renaming their runtime identities. Publish no instance output after body
failure; cancellation discards the entire run. The workspace saves selected/pinned
output aliases separately from semantic control values.

## Consequences

Controls and connections use existing transactional document/history commands.
Independent table and required 3D workflows use the same implementation. Unknown
or malformed instances remain losslessly retained and block execution with
diagnostics. A definition cannot contain another instance; nested components,
definition migration/editing, export and live updates need future decisions.

Snapshots increase file size, but make saved execution reproducible without a
catalog lookup. This remains a source-level native migration feature; it does
not establish a production runtime or dynamically loaded plugin system.

## Subsequent library work

N6e/N6f implement standalone import/export and undoable catalog-entry removal in
[decision 0014](0014-native-component-files.md). These operations preserve existing
instance snapshots. Definition editing/migration, nested execution and live updates
remain unimplemented; consult the [current roadmap](../../ROADMAP.md).
