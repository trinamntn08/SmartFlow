# 0005: Native workspace and execution integration

Status: accepted for checkpoint N3, 2026-09-30.

## Context

N2 provides an independent worker executor. The copied canvas and old widgets
must not become the semantic project model or mutate results into project data.
The previous workspace also has broader legacy application dependencies.

## Decision

Compose a small Qt workspace in `native/app/workspace`, reusing only the audited
double parameter editor and delegate port adapter from `tp_qt_pipeline_widgets`.
Keep vendor sources unchanged. `GraphProject` owns the legacy semantic graph and
monotonic revision; `PipelineCanvas` translates canvas operations and stable IDs.
Canvas positions and routing geometry remain presentation state. Numeric node
definitions are a bundled migration fixture, not a public extension interface.

Use one Qt undo stack for canvas commands and explicit parameter commands.
Deletion snapshots preserve stable step/output IDs, parameters and metadata.
They are trusted in-memory undo snapshots, not a native file format. Disable
clipboard operations until stable imported identities and compatibility are
implemented. N5 remains responsible for persisted unknown-extension content.

`ExecutionController` coalesces live edits, permits manual runs and cancellation,
and submits at most one run at a time. Publish only when both the project revision
and request generation still match. An edit clears displayed results immediately;
moving nodes does not invalidate execution. Results stay outside project state.

## Consequences

The inspector requires Apply before committing a draft. Runs recompute the whole
snapshot; caching, selective invalidation, progress streaming, extension loading,
and 3D remain later work. Cancellation is cooperative; window shutdown may wait
for a delegate that does not return. The workspace adds no dependency on a scene
model and does not replace the required future `scene-3d` extension.
