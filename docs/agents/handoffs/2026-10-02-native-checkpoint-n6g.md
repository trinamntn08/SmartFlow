# Task handoff: N6g component editing copies

## Objective and scope

Finalize the native first test version in separate verified commits. N6g delivers
component body editing in an isolated workspace and saves a new immutable copy.

## Current state

- Component library opens an isolated draft with the existing canvas/inspector.
- Save copy reuses the interface chooser with saved names and retains opaque
  definition/body/matching-interface fields. A fresh ID preserves source snapshots.
- Publication validates registrations, parameters, exposed ports and complete
  required input boundaries, and is one catalog undo command.
- Added an in-memory workspace document loader used by both file Open and drafts.
- Added independent table and scene behavior tests, including isolation,
  cancellation, stale/invalid publication, undo/redo and save/reopen execution.

## Verification

Passed Release build, 17/17 CTest entries (including offscreen startup),
`npm.cmd run check` (formatting, TypeScript, nine persistence tests and production
build), and `git diff --check`.
Qt code generation uses the documented direct-autogen workaround. Tests that
write temporary files require process access beyond the restricted sandbox.
The offscreen draft screenshot was inspected: canvas, toolbar, hint and Save/Cancel
controls are readable. Real Windows interaction acceptance remains manual.

## Decisions and open questions

[Decision 0015](../../architecture/decisions/0015-component-editing-scope.md) governs
immutable copies. Explicit instance replacement is the next separate step.
General migration, nested components and automatic updates remain out of scope.

## Next steps

Implement explicit compatible snapshot replacement, verify and commit it separately.
Refresh the packaged Windows test folder and report launch paths/checks.
