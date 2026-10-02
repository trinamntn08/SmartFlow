# Task handoff: E2 bounded native dependency scheduler

## Objective and scope

Implement sequential and parallel DAG scheduling over owned invocation data.
UI controls/progress and node-internal parallel work follow separately.

## Current state

`DependencyScheduler` compiles producer edges and complete component barriers,
validates the actual expanded plan, then runs a fixed-size worker pool. One
coordinator owns readiness, output cloning, failure propagation and completion.
Readiness uses unique producer dependencies and stable stored-order tie breaks.
Workers signal completions through a mutex/condition-variable channel. No-ready
work waits while predecessors run. Cancellation stops dispatch and drains active
workers before destroying contexts; cancelled outputs are discarded.

`PipelineExecution::submit` snapshots options and validates budgets/resources.
Sequential defaults to one node worker; parallel uses at most `maxThreads`.
Unknown delegates serialize globally; explicit exclusive resources also serialize
across separate executors/windows. Audited reentrant policies allow overlap.
Factory cloning stays on each coordinator. Domain factories must remain immutable
and support safe concurrent clone calls across separate runs.

## Verification

Release build (`cmake --build --preset windows-local-release --parallel 1`), full
CTest (`ctest --preset windows-local-release-tests --parallel 4`, 19/19 in
43.39 seconds) and `npm.cmd run check` passed outside the process/filesystem
sandbox with repository-local temporary files and the documented test font.
After strengthening resource exclusion and adjacent-component tests, the
scheduler target was rebuilt and its CTest entry rerun before commit.
`git diff --check` passed. The new
`native_scheduler` entry tests reversed diamonds across modes/limits, gated
branch overlap and thread caps, serial defaults, resources across executors,
mutable fan-out isolation, duplicate producer ports, failed hidden component
branches, cancellation drain, invalid options and 1000 independent nodes.
Existing pipeline/component/data/scene tests exercise regression behavior.

## Decisions and open questions

See [decision 0017](../../architecture/decisions/0017-native-execution-concurrency.md).
The total budget currently bounds node workers; node-internal reservations and a
managed work helper follow in E4. UI still uses the default sequential options.
No vendor sources, dynamic plugin API or TypeScript runtime changed.

## Next steps

Expose mode/thread controls and live run-tagged node/component progress in Qt;
add managed internal work and reservations. Refresh the desktop package after UI
verification. Full manual desktop interaction acceptance remains pending.
