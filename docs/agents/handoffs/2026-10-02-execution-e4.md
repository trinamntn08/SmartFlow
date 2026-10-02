# Task handoff: E4 managed node work and refreshed desktop package

## Objective and scope

Complete the authorized execution implementation with a shared processing-thread
budget for graph nodes and node-internal parallel work. Deliver the updated native
test folder with repeatable sequential/parallel startup verification.

## Current state

`NodeWork.h` provides bounded `parallelFor`, cancellation polling and the current
node thread limit. A caller participates in internal work; nested helpers are
serial and all child threads join before return/exception. The scheduler reserves
the declared node thread limit within the per-run budget before dispatching and
releases it only after synchronized completion. Default contributions reserve one
thread. Sequential node scheduling can use reserved internal parallelism; a budget
of one makes both levels sequential. Arbitrary library/unmanaged threads remain
outside the managed guarantee.

CLI `--parallel --threads <1..64>` makes native launch checks repeatable and syncs
the visible controls. Thread-only arguments retain sequential mode; malformed
arguments exit 7. Package verification adds scene/data/component parallel runs.
No vendor sources or persisted project schemas changed.

## Verification

Release build (`cmake --build --preset windows-local-release --parallel 1`) passed.
`ctest --preset windows-local-release-tests --parallel 4` passed 24/24, with a final
38.39-second run after adding a synchronous progress-observer invalidation
regression. `npm.cmd run check` passed formatting, type checks, nine persistence
tests and the production build; `git diff --check` passed. Checks used the
established unrestricted execution environment; native tests used repository-local
TEMP/TMP and the documented font.

`cmake --install build/native --config Release --prefix
D:/dev/projects/SmartFlow/build/desktop-test` refreshed the Windows folder.
`powershell -NoProfile -ExecutionPolicy Bypass -File scripts/Test-DesktopPackage.ps1`
passed ten native startup/render configurations with developer Qt paths removed
and matching installed/build executable hashes. Packaged parallel scene and
component screenshots were visually inspected: mode/thread controls, completed
step counts, node states and restored scene/table viewers are legible. Invalid
thread values 0, 65, text and missing values were verified with hidden, waited
packaged processes and each exited 7. Initial direct PowerShell invocation did
not wait for the GUI process; it was replaced with `Start-Process -Wait -PassThru`
to read actual exit codes.

The package report is refreshed after committing to record the final source
commit. Only metadata/docs change after the final binaries were verified.

`native_node_work` verifies mixed graph/internal
work capacity, sequential nodes with internal threads, nested work, exact-once
results, joined exception/cancellation paths and serial/parallel result parity.
Native UI/scheduler/data/scene tests retain their prior checks. Parallel scene,
data and component startup entries now exercise real executable options.

## Decisions and open questions

See [decision 0017](../../architecture/decisions/0017-native-execution-concurrency.md)
and [execution](../../EXECUTION.md). Thread budgets are per run, not a machine-wide
quota. Source-level safety declarations and deep-clone correctness remain trusted
extension obligations. No sanitizer/race instrumentation or clean-machine Windows
compatibility was established. Full desktop interaction acceptance remains pending
the user's manual results; offscreen/native startup/render tests do not replace it.

## Next steps

Collect packaged edit/undo/save/relaunch/Open and sequential/parallel interaction
results. Address demonstrated issues in separate verified checkpoints. Selective
recomputation, remote work, arbitrary plugin loading and production domain features
remain separate future work.
