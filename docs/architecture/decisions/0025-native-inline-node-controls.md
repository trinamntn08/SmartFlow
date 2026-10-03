# 0025: Native inline node parameter controls

Status: accepted.

## Context

The user wants to edit parameters directly in graph nodes, similar to Blender's
node editor. Previously embedded widgets showed only execution timing; parameters
were edited in a separate inspector. Project commands, execution and component
boundaries must stay shared across the UI.

## Decision

Application-owned `NodeParameterPanel` widgets embed compact numeric controls in
each supported node. Typed values commit on Enter/focus change and spin steps
commit immediately. Updates use stable node IDs and the existing GraphProject
parameter command. Refreshes block signals and retain widgets when their shape is
unchanged. Other semantic edits discard local drafts; timing refreshes do not.

Transient compiled StepDetails retain delegate parameter order; component facades
retain authored public-control order. Embedded controls never expose component
body parameters or mutate definitions. Unsupported content remains retained.
The inspector and Use-mode tool interface continue using the same document.

`NodeBodyGeometry` puts sockets above controls to keep cards compact and preserve
existing saved positions/navigation. The vendored canvas adds one geometry setter
parallel to its existing painter setter. Its default geometry remains unchanged;
layout and document semantics live in the app. No new dependencies, schema changes
or concrete domain imports are introduced.

## Alternatives

Editing the copied delegate's StepDetails would modify disposable state and bypass
history. Rebuilding a whole card on each edit would disrupt text focus and risk
deleting a widget during its own signal. Placing fields beside sockets in the
default geometry would make nodes wider and overlap existing example layouts.

## Consequences and verification

Current inline editing supports the bundled double parameters. Other parameter
types are read-only until a concrete editor is added. Qt proxy-widget tests cover
canvas input routing, typed drafts, stepping, cancellation, execution, shared undo,
inspector/viewer synchronization, component scope, deletion and save/reopen.
Full native and web checks, offscreen renders and installed Windows launches
are required. Interactive Windows testing of the new controls is separately
reported; no browser UI change or production plugin API is implied.
