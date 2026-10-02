# Native node execution

Use the toolbar to select **Sequential** or **Parallel** and set **Threads**
(1..64 per run). Sequential runs one ready node at a time. Parallel runs independent
ready nodes concurrently within the configured limit. With Threads set to 1,
parallel mode has no branch overlap. A dependency chain still runs upstream
before downstream regardless of the thread setting.

The scene, numeric and independent data nodes are audited reentrant contributions.
Other delegates serialize by default until their source-level execution policy
opts into reentrancy. Named exclusive resources serialize conflicting operations,
including across windows. External side effects needing a particular order must
also have explicit graph dependencies.

**Execution results** shows live Waiting, Ready, Running, Complete, Failed,
Skipped and Cancelled states. Nodes reporting fractional work show their current
percentage. The progress bar counts compiled steps that have finished, including
failures, skips and cancellation; it is not a time estimate or a success indicator.
Components appear as one visible node and aggregate their internal steps without
double counting. Final errors and result viewers still show only validated outputs.

Changing mode/thread settings cancels outdated work. With Live updates enabled,
the latest graph runs after the current work drains; otherwise press Run.
Cancel shows Cancelling until active work stops, then Cancelled. A cancelled run
publishes no outputs. Edits, document replacement and new Run requests reject old
progress and outputs. Mode/thread options and progress are transient and do not
alter project files or undo history.

## Extension execution contracts

`native/sdk/ExecutionPolicy.h` defines reentrancy, resource names and thread limits.
Only trusted native contributions can declare safety. Const delegate methods alone
do not establish reentrancy. A delegate must not mutate the graph, registry, UI or
another invocation, retain context/progress pointers, or continue work after it
returns. Long work must poll cancellation; non-cooperative delegates can delay
shutdown.

Legacy inputs are mutable, so the adapter deep-clones each input member before
invocation and clones published outputs. Factories must recursively own their
clones and be safe for const clone calls across runs. Missing/failed/aliased clones
fail the node. This costs memory and copying; the legacy API still does not enforce
deep constness, and arbitrary extension code is not sandboxed.

The coordinator owns dependency counters, readiness, completion and result
aggregation; workers only execute their private contexts. Component bodies wait
for all external inputs, and external consumers wait for the entire body. A failed
or invalid output skips its consumers while unrelated branches can finish.

## Work inside a node

`native/sdk/NodeWork.h` supplies `parallelFor`, `nodeThreadLimit` and
`nodeWorkCancelled`. Declare a node's internal `threadLimit` in its execution
policy; the scheduler reserves that capacity, capped by the run's Threads value.
Unspecified nodes reserve one thread. Graph workers and managed internal work
share this budget, so concurrent nodes cannot each consume the entire limit.
Sequential mode still runs one node at a time, but that node can use its reserved
internal threads. Set Threads to 1 to make both levels sequential.

Inside a delegate, partition private data by index:

```cpp
std::vector<double> values(count);
if(!smartflow::parallelFor(count, [&](size_t index) {
    values[index] = process(index);
})) return false; // cancellation
```

The caller participates, child workers join before return/exception, and nested
helpers run serially on their current thread. Callbacks must operate on disjoint
owned data or synchronize their shared state. Poll `nodeWorkCancelled()` inside
long callbacks. Worker callbacks must not touch Qt UI, graph/registry state or
launch unmanaged threads; third-party library threads require an explicit budget
agreement and are not automatically controlled. Factory cloning and the scheduling
coordinator are outside the processing-thread count. Multiple separate runs each
have their own budget; named resources coordinate exclusivity across windows.

For repeatable native launches, use `--parallel --threads 4`. `--threads` alone
uses sequential node scheduling with the given internal-work budget. Invalid
thread arguments exit 7. These options also update the displayed toolbar controls.

Selective caching, streaming, remote execution and process isolation remain
future work.

See [decision 0017](architecture/decisions/0017-native-execution-concurrency.md) and
the [original architecture review](architecture/execution-review-2026-10-02.md).
