# W5b: Component authoring and browser workspace restoration

## Objective and scope

Complete browser component authoring without mutable catalog identities or
automatic snapshot updates; retain native workspace independently.

## Current state

The library supports extraction, strict local standalone import/download, immutable
catalog conflicts/no-ops, bound collapsed insertion and undoable removal. Isolated
copy drafts use the ordinary graph editor, parameters and history; publication
requires the unchanged destination revision and gives the definition a fresh ID.
Interface choices preserve retained metadata and named output aliases. Explicit
instance update compares public port/control contracts, connected ports and carried
values, then replaces one snapshot in one undoable command. Unknown/unavailable
content remains retained and displays diagnostics.

Browser layout controls show/hide library/inspector and adjust panel widths.
Layout, navigation, camera, row selection and pinned node/port use workspace state;
they do not enter graph history or invalidate execution. Opaque layout shapes
reject ordinary edits until explicit Reset. Native layout/viewers are retained.

## Verification

`npm.cmd run check` passed: formatting, all type checks, 68 Node behavior/transport
tests and production worker/editor build. Four Chromium component/authoring checks
passed: extraction/file exchange/insertion, isolated body edit/save-copy, compatible
update/exact undo and panel save/reopen independent of execution/history. The
component-library screenshot was inspected. Tests also cover atomic conflicts and
failed insertion, snapshot execution after catalog removal, retained overrides,
incompatible update rejection and preserved redo. Native format/source unchanged;
native component/startup tests passed in W5a.

## Decisions and open questions

Follow [immutable copy/update semantics](../../architecture/decisions/0015-component-editing-scope.md).
Browser layout is a separate bounded panel layout, not Qt widget-tree conversion.
Nested components, bulk migrations, arbitrary plugins and production imported-asset
rendering remain outside the preview. Native manual acceptance remains pending.

## Next steps

W6: verify production serving, both domains/components, actual native save →
browser edit/save → fresh native reopen, and publish local delivery instructions.
