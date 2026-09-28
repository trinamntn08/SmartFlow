# Task handoff: checkpoint 1 — project persistence and SDK contracts

## Objective and scope

Start the saved legacy reuse plan in verified increments. Preserve the old
repository. This checkpoint implements the first bounded task recommended by
`docs/architecture/legacy-reuse-analysis.md`; later steps remain pending.

## Current state

- Core is a private npm workspace with versioned graph/project JSON documents,
  separate workspace state, structural validation, loading and saving.
- Unknown extension payloads and dangling connections survive reopening.
- SDK is a private workspace with initial framework-independent contracts.
- Root check includes persistence tests. Lockfile includes both new workspaces.
- No legacy source, assets, build files or machine settings were copied. Legacy
  node and port concepts were referenced and independently implemented.
- The pre-existing untracked reuse analysis was preserved.

## Verification

- `npm.cmd run typecheck`: passed for web, core and SDK.
- `npm.cmd test`: initial seven persistence tests passed after allowing the Node
  test runner's child process (sandbox initially returned `spawn EPERM`).
- Final `npm.cmd run check`: passed with process permission enabled. Formatting,
  all workspace typechecks, eight persistence tests, and the production build passed.
- Old repository HEAD remains `d6f457e8415445d00eaf7e9fbf9e5047768ea637`.
  Its only reported change before implementation was the existing untracked
  `documentation/VisualWorkspaceProposal.md`. No write operations target it.
- No UI changed; browser interaction is not part of this checkpoint.

## Decisions and open questions

See [project contract decision](../../architecture/decisions/0002-project-contract.md).
Unknown content preservation is at the JSON value level. Node/package resolution,
automatic migrations, viewer adapters and execution are not implemented.

## Next steps

1. Commands and undo/redo, semantic connection validation, DAG execution.
   Test invalid ports, missing packages, cycles, upstream failures, downstream
   invalidation and cancellation that cannot publish stale results.
2. Included 3D nodes and SDK viewer, then canvas/inspector integration with browser
   verification of transform editing, undo and save/reopen.
3. Independent table/filter/summary extension, then reusable graph components.

Keep an individual tested checkpoint record for each step. Legacy import remains
a separate feature; these files do not open old native project formats.
