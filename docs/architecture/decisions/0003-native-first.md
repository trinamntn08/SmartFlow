# 0003: Native C++/Qt first, TypeScript deferred

Date: 2026-09-27. Status: accepted by explicit user direction.

## Decision

Start SmartFlow as a native C++/Qt application based on source copied from the
old studio engine repository. Preserve that repository without modification.
Keep the existing TypeScript prototype for a future extension. It does not
dictate native contracts or persistence. Browser delivery is a future option.

This supersedes the browser-first implementation recommendations in the reuse
analysis and the delivery assumptions of decisions 0001 and 0002. Their history
and the working TypeScript files are retained.

## First migration checkpoint

Copy the existing QtNodes canvas library and license. Replace its single
platform-export dependency and build it with an independent CMake target.
Add a Qt Widgets shell and a small numeric adapter to test graph creation,
connection propagation and canvas snapshot restoration. This is a migration
harness, not the final runtime or project format.

## Next boundaries

Audit and copy pipeline/data/task dependencies in independently buildable
steps, then migrate the pipeline inspector and 3D workflow. Preserve module
notices and record source hashes and local adaptations. No build may depend on
the location of the old checkout. Do not copy private assets or machine configs.

The product remains domain-independent with required 3D support. Unknown
extension preservation, undoable project edits and separation of saved and
execution state remain requirements. QtNodes snapshots are not yet a stable
SmartFlow document format. Native packaging and interactive 3D remain pending.
