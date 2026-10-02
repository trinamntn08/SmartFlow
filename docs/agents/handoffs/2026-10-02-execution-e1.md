# Task handoff: E1 execution contracts and data isolation

## Objective and scope

Begin the authorized execution implementation by defining source-level policies
and making mutable legacy invocation inputs private. E1 does not yet enable
parallel scheduling or change the desktop UI.

## Current state

`native/sdk/ExecutionPolicy.h` defines conservative reentrancy/resource policies,
thread demand and sequential/parallel options. `ExecutionData.h` deep-clones each
input and published output through the registered factories, rejecting missing,
failed, aliased or incompatible clones. No vendor files were changed.

## Verification

`cmake --build --preset windows-local-release --parallel 1` passed outside the
process sandbox after Qt moc was blocked inside it. `ctest --preset
windows-local-release-tests` passed 18/18 (109.10 seconds), with repository-local
TEMP/TMP and the documented test font outside the filesystem sandbox.
`npm.cmd run check` passed formatting, type checks, nine persistence tests and
production build. `git diff --check` passed. Pipeline
tests cover an input-mutating delegate, forwarding a private input under another
identity and missing-clone failure propagation. Existing scene/data suites check
their domain clone behavior and unchanged execution/save/reopen semantics.

## Decisions and open questions

See [decision 0017](../../architecture/decisions/0017-native-execution-concurrency.md).
Deep cloning adds copy/memory cost. Future factories must implement recursive
isolation. Native contracts remain source-level; no TypeScript runtime is added.

## Next steps

Implement the bounded dependency scheduler, then Qt controls/progress and shared
thread budgeting, each in a separate verified commit. Desktop manual acceptance
remains pending; E1 has no UI changes.
