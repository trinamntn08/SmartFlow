# 0008: Retained-document editor commands

Status: accepted for N5e, 2026-10-01.

## Decision

GraphProject now owns DocumentHistory instead of a mutable legacy pipeline.
Semantic edits and undo operate on the retained DOM. Inspector StepDetails are
detached copies; they cannot mutate execution or become a source for saved files.
DocumentSession exposes individual inspection copies while keeping its executable
graph gated by diagnostics. Unsupported content produces execution diagnostics,
without submitting a partial graph to a worker.

WorkspaceScene routes creation, selection deletion, connection and disconnection
through the document command layer. Selection deletion is one candidate edit,
including all incident unknown edges. Its undo stack also hosts the existing
workspace-only movement commands, so shortcuts retain chronological undo behavior
without changing semantic revisions for movement. Inspector Apply issues a
document parameter command directly; the previous nested parameter command is
removed. Initial demonstration setup clears its command history.

The copied QtNodes scene gains four virtual command-routing methods and a virtual
undo-stack accessor. Default implementations preserve its existing behavior.
Only user-interaction call sites route through these methods. SmartFlow's scene
overrides them; vendor code does not depend on SmartFlow documents or extensions.
This small isolated patch supersedes the N3 decision to leave all canvas sources
unchanged. Provenance and license notices remain intact; the original checkout
is never modified or consulted at build/runtime.

PipelineCanvas synchronizes incrementally from the retained selected graph.
Synchronization suppresses edit callbacks and keeps stable canvas IDs and current
positions through semantic undo. Only generated display envelopes reach QtNodes.
Clipboard/legacy file import remains disabled. Unknown node types use zero-port
placeholders labeled with package/type; unresolved edges remain in the DOM even
when not drawable. A hidden edge still occupies its target input. Known but
unsupported parameter content remains retained and blocks execution.

## Limits and next work

Canvas positions, selection, pinned outputs and viewer camera still need explicit
workspace persistence. Canvas identity/position caches are transient, including
across model-level replacement; file Open should restore its own workspace state.
The context menu lists available node titles; unavailable placeholders cannot be
created from the node library. There is no new file dialog or Save/Open action in
this checkpoint. Package loading, production rendering and the independent data
extension remain future work.

Model replacement prepares a candidate and clears old commands. The existing
execution revision/generation checks discard obsolete completions. Tests exercise
replacement, unsupported-content blocking, file round trips through the editor
model and delete/disconnect undo with opaque metadata. Whole-document history and
inspection copies favor correctness over large-graph optimization.
