# Task handoff: N6h explicit component instance updates

## Objective and scope

Continue finalizing the first native test version after committed N6g.
Deliver explicit compatible snapshot replacement for one collapsed instance.

## Current state

- Added session/history commands preserving node identity, parameters, opaque
  instance fields, edges, catalog, other instances/graphs and workspace.
- Validate supported definitions, matching public names/types, current parameter
  bounds and graph compatibility before publishing. Changed content requires a
  fresh definition identity. Rejections preserve redo; identical content is a no-op.
- Added **Components > Update selected instance...** with a readable body/default
  comparison and inline compatibility diagnostics. Apply is one undoable edit.
- Added data execution/isolation/save-reopen tests, actual menu/dialog/pinned-output
  checks and scene override/bounds tests.

## Verification

Passed Release build and all 17 CTest entries: 16 passed in the full run, then
`ctest --preset windows-local-release-tests --rerun-failed --output-on-failure`
passed the UI suite after its pre-dialog selection capture was allowed to finish.
Passed `npm.cmd run check` (formatting, TypeScript, nine persistence tests and
production build), final documentation formatting and `git diff --check`.
Inspected `component-update-smoke.png`; comparison text, choice and buttons are readable.
Real packaged interaction acceptance remains pending manual results.

## Decisions and open questions

[Decision 0015](../../architecture/decisions/0015-component-editing-scope.md) is
implemented through N6h. Automatic/bulk updates, interface remapping, general
migration and nested definitions remain outside the first version.

## Next steps

Refresh and verify the Windows runtime test folder; provide direct launch paths
and the desktop procedure. Do not mark manual N5 acceptance complete without results.
