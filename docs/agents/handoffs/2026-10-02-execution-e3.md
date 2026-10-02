# Task handoff: E3 native execution controls and live progress

## Objective and scope

Expose bounded scheduling in the desktop UI and publish live per-node/component
progress without stale events or worker UI access. Node-internal work follows E4.

## Current state

Toolbar Sequential/Parallel and Threads controls update transient run options.
Audited numeric, data and scene contributions declare reentrancy. Execution
results shows live states, optional fractions and compiled-step completion counts.
Components aggregate visible state without changing counts. Run-tagged snapshots
are synchronized and read by the existing Qt completion timer. Revision and request
generation checks reject old progress/output. Cancel also suppresses an already
completed future that has not been published and shows Cancelling until drained.

No persisted schema or vendor files changed. Details and extension obligations are
in [the execution guide](../../EXECUTION.md).

## Verification

Release build (`cmake --build --preset windows-local-release --parallel 1`) passed.
`ctest --preset windows-local-release-tests --parallel 4` passed 20/20 in
37.12 seconds, including offscreen startup. `npm.cmd run check` and
`git diff --check` passed. Checks used the previously established unrestricted
process/filesystem environment, repository-local TEMP/TMP and the documented
font. The rendered `execution-parallel-smoke.png` was inspected: both Work nodes
show Running/50%, Join shows Waiting, and toolbar controls/counts are legible.
`native_execution_ui` exercises actual controls, gated branch overlap,
waiting joins, nested fractions, Qt-thread updates, edit/mode invalidation and new
run IDs. It writes `execution-parallel-smoke.png`. Scheduler tests now inspect live
counts/fractions, component aggregation and terminal cancellation. Scene and data
tests compare parallel output against their sequential baselines.

## Decisions and open questions

See [decision 0017](../../architecture/decisions/0017-native-execution-concurrency.md).
Counts measure terminal compiled work, not estimated time or successful nodes.
Modes/options are session-local and do not dirty project state. Arbitrary mutable
third-party payloads still need a correct recursive clone implementation.

## Next steps

Reserve capacity for managed node-internal work, test nested/exception/cancellation
behavior and refresh the desktop test package. Full manual packaged interaction
acceptance remains pending; offscreen UI tests do not satisfy it.
