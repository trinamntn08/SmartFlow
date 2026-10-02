# Task handoff: component editing scope

## Objective and scope

Continue from the completed documentation synchronization checkpoint by scoping
the next independent roadmap task while manual desktop results remain pending.
This is a documentation-only proposal; no application behavior changes.

## Current state

- Added [proposed decision 0015](../../architecture/decisions/0015-component-editing-scope.md).
- Scoped isolated draft editing saved under a fresh definition ID, followed by a
  separate explicit compatible replacement command for collapsed instances.
- Linked the proposal from the roadmap, component guide and decision index.
- Confirmed in current code that definition `version` only accepts 1 and catalog
  conflicts compare ID/version. Saved snapshots remain independent of the catalog.

## Verification

Passed `npm.cmd run check`: formatting, TypeScript, nine persistence tests and
web production build. `git diff --check` passed.
Native code did not change; native builds and interaction tests are not repeated.

## Decisions and open questions

Decision 0015 is proposed, not implemented or a completed N6 behavior checkpoint.
General revision/lineage and migration remain open. Full N5 desktop acceptance
still awaits manual results; previously deferred automation was not resumed.

## Next steps

Settle the proposal's scope before implementing an isolated definition editor.
Deliver editing and explicit replacement as separate verified, committed steps.
Use manual desktop reports to prioritize demonstrated correctness/usability issues.
