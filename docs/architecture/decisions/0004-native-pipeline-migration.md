# 0004: Isolate the copied pipeline behind an execution adapter

Date: 2026-09-28. Status: implemented for checkpoint N2.

## Decision

Build a selected legacy pipeline/data/task source subset as `smartflow_legacy`.
Keep the SmartFlow execution adapter in `native/app/pipeline`, built separately
as `smartflow_pipeline`. Neither target depends on QtNodes, a scene model, or the
old checkout. The application canvas is connected in the subsequent N3 work.

The copied types are migration infrastructure, not the final project format or
public extension API. The TypeScript core/SDK remain separate prototypes.

## Execution boundary

The adapter snapshots a pipeline on its owning thread, then submits the snapshot
to one legacy task-queue worker. Delegates and the data factory are shared with
the worker and must remain immutable for the entire run. Delegates must not edit
the graph or UI during execution. Results are returned through a future and are
separate from the source graph. Each submitted run recomputes all nodes; caching
and selective downstream invalidation are deferred.

Validate node identities, delegate availability, exact port mappings, unique
output identities, required inputs, connection types, and cycles before invoking
the legacy scheduler. All declared inputs are required in this checkpoint.
Legacy disabled-node flags, output overrides, and fallback operations are
reported as unsupported. These semantics need an explicit later design.

Catch delegate failures and exceptions, validate produced outputs, and skip
dependent consumers while permitting independent branches to finish. Cancellation
is cooperative: delegates doing long work must call `Progress::poll()`. A cancelled
run returns no node outputs. A future UI integration must associate results with
the current project revision and reject obsolete completed runs; the adapter
does not automatically publish results into project or viewer state.

## Local legacy change

`PipelineManager` previously honored `fixupParameters=false` only at construction;
starting execution or receiving a change callback restored the default fixup.
It also cleared dangling inputs unconditionally. Preserve the constructor option
across rebuilds and only prune when fixup is explicitly enabled. The adapter
always disables fixup and rejects invalid graphs without repairing them.

Keep the vendor diff and original/copied hashes in `native/vendor/legacy`.
No legacy binary loader or parameter repair is exposed as SmartFlow persistence.

## Limits

This is a tested migration foundation for trusted bundled delegates, not a
production plugin system, sandbox, or complete runtime. The legacy binary
serialization helpers are compiled dependencies but are not a supported import
format. Native persisted unknown-extension preservation still belongs to N5.
Queue shutdown waits for active work; a delegate ignoring cancellation can delay
shutdown. Native viewer/inspector integration, commands/undo, selective
recomputation, and 3D execution remain subsequent checkpoints.
