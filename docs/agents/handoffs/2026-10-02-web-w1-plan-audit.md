# Task handoff: W1 parallel web plan and compatibility audit

## Objective and scope

Start the user-approved web plan alongside desktop work. Deliver a concrete plan
and native/TypeScript file compatibility audit. No application code, dependency,
native behavior or packaged executable is changed.

## Current state

[The plan](../../WEB_IMPLEMENTATION_PLAN.md) defines W1b strict transport, retained
commands/editor, worker execution/data, required scene-3d, components/workspace and
browser delivery checkpoints. [Decision 0018](../../architecture/decisions/0018-parallel-web-development.md)
activates the web track while retaining native priority and Windows acceptance.
Current README, roadmap, architecture, development and product direction link the
plan without claiming an implemented browser editor/runtime.

[The audit](../../architecture/web-compatibility-audit-2026-10-02.md) records source
evidence and a reproducible Node probe. The shared fixture and all three shipped
projects round-trip semantically through TypeScript, including opaque component
and native workspace fields. Duplicate keys/deep nesting are accepted where native
rejects; an opaque 64-bit integer is silently rounded. Standalone component files
have no TypeScript parser. These gaps remain unfixed in this documentation step.

## Verification

- Repository-root Node.js 24.19.0 probe from the audit: four semantic round-trips
  passed; malformed/unsupported probes produced the recorded outcomes.
- `npm.cmd run check`: passed formatting, TypeScript, nine persistence tests and
  the production browser build.
- Local Markdown links: all 165 targets in the 11 changed/new documents exist.
  `git diff --check`: passed.
- No native sources changed, so CMake/CTest and desktop interaction checks are not
  rerun. No browser UI changed; browser interaction is not verified by this step.

## Decisions and open questions

Browser-local execution, import/download and separate versioned workspace state
are the initial direction. Numeric preservation/rejection policy must be resolved
in W1b before conversion. Canvas/rendering dependencies, supported browser versions,
deployment provider, asset bundling and broader parity remain open. No remote
service, bridge or arbitrary plugin loader is added.

## Next steps

Implement W1b with shared malformed transport fixtures, byte/depth boundaries and
numeric corruption checks; establish native/browser acceptance for the supported
subset. Commit that verified step before starting W2. Desktop packaged manual
acceptance and demonstrated fixes continue independently.
