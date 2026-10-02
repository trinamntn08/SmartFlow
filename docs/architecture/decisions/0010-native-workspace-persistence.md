# 0010: Native workspace persistence

Status: accepted for N5g, 2026-10-01.

## Decision

Persist editor state in `workspace["smartflow.native-editor@1"][graphId]`.
Positions and selection use retained node identifiers. Navigation stores canvas
scale and center; pinned output stores a node identifier. Viewer state lives in
an opaque `viewers` map keyed by the contribution's versioned state key.

OutputViewer supplies JSON capture/restore hooks and a change callback. The host
never imports scene types. The scene extension owns validation and defaults for
its yaw, pitch, zoom span, target and object selection. Transient execution
results are never serialized. Viewer selection uses an index in the current
bounded primitive output; durable object selection across reordered results
remains future work.

Workspace hooks use DocumentHistory's explicit field setter. They update dirty
state without changing semantic revision, submitting work or entering semantic
undo. Canvas movement retains its existing workspace undo behavior. Save and
unsaved-change checks capture current view state; UI hooks coalesce canvas
updates after Qt has applied them. Open suppresses capture during replacement
and restoration. Old queued captures cannot cross document replacement.

Merge owned fields into retained objects, preserving other workspace keys,
other graphs, missing-node positions and unknown viewer fields. Unrecognized
future object shapes are retained rather than converted. Restore validates
numeric ranges and ignores malformed known fields. Files without editor state
use default layout and terminal-output pinning, with canvas fitting.

## Limits

Only the first graph is opened by the current editor. Splitter sizes and window
geometry are not persisted. Canvas centers are subject to Qt scrollbar pixel
rounding and viewport size. This is a source-level contract for bundled native
extensions, not a production plugin ABI or loader.

## Subsequent layout work

N7 adds persisted selectable-widget split regions and divider proportions under
[decision 0016](0016-selectable-workspace-widgets.md), superseding the earlier
splitter-size limit. Outer window geometry remains unpersisted.
