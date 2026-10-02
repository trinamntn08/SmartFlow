# Task handoff: native execution architecture review

## Objective and scope

Review chain correctness, sequential/parallel node processing, configurable
threading and progress architecture. The user clarified that this pass should
review and document required changes, rather than implement execution modes.
Reviewed code baseline: `63602fb`. No application, SDK or vendor sources changed.

## Current state

The [review](../../architecture/execution-review-2026-10-02.md) records prioritized
findings with source references, implemented guarantees, proposed implementation
checkpoints and acceptance tests. Architecture and roadmap now explicitly state
that node execution is sequential on one background worker. Parallel selection,
worker-count configuration and live per-node progress remain unimplemented.

Dependency/cycle validation, branch-local failure propagation, component
barriers, snapshot isolation and revision/request checks form the foundation.
Concurrency needs a single scheduler owner, bounded workers, synchronized
completion publication, immutable input/output rules, delegate/resource safety
metadata and thread-budget/lifetime contracts. Existing mutable legacy accessors
cannot establish safe arbitrary extension concurrency.

## Verification

- `cmake --build --preset windows-local-release --parallel 1`: passed, including
  both native applications and tests. The preset's default parallel build exited
  1 with no diagnostic errors; a verbose retry also exited 1. Serial build
  succeeded. No build configuration was changed.
- `ctest --preset windows-local-release-tests`: initially 10/18 passed inside
  the filesystem sandbox. Eight save/reopen-related suites raised unhandled
  exceptions; retrying with repository-local TEMP/TMP and the documented test
  font did not resolve them. Pipeline, graph and all eight startup/launch entries
  passed in that initial run.
- Full CTest rerun outside the filesystem sandbox, with TEMP/TMP pointing to
  `build/review-temp` and `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf`:
  **18/18 passed**, 98.43 seconds. The previously failing suites all passed.
  This isolates the failed runs to the restricted execution environment; the
  exact rejected filesystem operation was not instrumented. Native pipeline XML
  records 25 test cases, zero failures (including setup/cleanup).
- `npm.cmd run check`: passed formatting, TypeScript, nine persistence tests
  and production build before documentation edits. The post-edit sandbox run
  passed formatting/type checks but the Node test runner hit `spawn EPERM`.
  The full post-edit rerun outside the process sandbox passed all checks.
- `git diff --check`: passed. All five changed Markdown files were formatted
  with repository-local Prettier before the final checks.
- No interactive desktop acceptance or concurrency stress/race verification was
  performed. Offscreen startup passing does not establish packaged interaction
  acceptance. No parallel implementation exists to test in this review.

Temporary logs, screenshots and test results are ignored build output. The old
repository was not accessed or changed.

## Decisions and open questions

This is an evidence-backed review, not adoption of a final public runtime API.
[Decision 0004](../../architecture/decisions/0004-native-pipeline-migration.md)
still describes delivered sequential behavior. The proposed node safety defaults,
resource metadata and internal thread budgets need an implementation decision
record when that work begins. The native SDK remains a source-level contract;
the TypeScript runtime remains reserved.

## Next steps

Implement the four recommended checkpoints individually, each with meaningful
behavior tests, native/web checks, checkpoint documentation and a separate
commit. Prove dependency chains and diamonds, bounded branch overlap, component
barriers, safe fan-out, cancellation, stale-event rejection and UI-thread progress
publication before claiming parallel support. Retain scene workflows and validate
the same scheduler with the independent data extension.
