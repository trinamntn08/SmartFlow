# Task handoff: W2b browser editor and file actions

## Objective and scope

Deliver browser library/canvas/inspector and retained file actions after W2a.
Keep native priority and schema-v1. Execution is deliberately unavailable until
W3/W4; component instances remain retained placeholders until W5.

## Current state

The browser setup page is now a workspace with searchable library, typed named
ports, dragging/navigation, selection, numeric inspection, add/connect/delete,
undo/redo, keyboard shortcuts and dirty-state indication. Incompatible/occupied
inputs and newly introduced cycles reject. Unknown nodes render placeholders;
unrenderable dangling edges remain in the retained connection list and save file.

Strict import and download preserve assets, opaque envelope/project/node fields,
inactive graphs and native workspace content. Failed imports do not replace the
current project. Unsaved replacement asks to discard/download. New empty projects
have no domain dependency. Native positions/selection/pinning and scene-center
navigation map explicitly into separate browser workspace state. Viewer/panel
payloads remain unchanged in their native namespace. Unsupported workspace shapes
reject edits without partially adding a node.

Data and scene editor metadata live in their own new source-level workspaces,
with native IDs/defaults/ranges. React Flow is an application-only adapter.
Core history supports atomic project/workspace command submission while retaining
workspace-independent semantic undo. Selection updates follow user events rather
than reacting to projected selection, fixing a browser-only feedback loop found
by the interaction suite.

## Verification

- `npm.cmd run check`: formatting, all package TypeScript checks, 53 Node behavior
  tests (43 core, four SDK, six web adapters) and production build passed.
- `npm.cmd run test:browser`: three isolated Chromium tests passed on Windows;
  Chrome for Testing/Headless Shell 153.0.8010.12, Playwright 1.63.0.
  Verified inspector edit/undo/redo, download/reopen in a fresh page, native
  workspace retention, new nodes/typed port dragging, deletion/connection undo,
  unavailable-node preservation and duplicate-key import failure isolation.
  Page errors are checked by the suite.
- Inspected `build/web-tests/results/editor-inspector-edits-und-37587-ith-opaque-workspace-intact-chromium/editor.png`:
  library, canvas, selection, inspector and file controls render legibly.
- `git diff --check`: passed. Native schema/source are unchanged, so native checks
  were not rerun. Editor-level native → browser → native exchange is a W6 gate.

Connected computer-use browser inventory was empty and creating an in-app tab
reported unavailable. Repository tests use isolated Chromium/cache, not the user's
browser profile. Manual desktop acceptance remains pending.

## Decisions and open questions

[Decision 0020](../../architecture/decisions/0020-browser-canvas.md) records canvas,
license, metadata ownership, workspace conversion and isolated browser test tools.
The [web guide](../../WEB_DEVELOPMENT.md) describes current workflows and limits.
No scene renderer, worker runtime, component execution or dynamic plugin loading
exists in this checkpoint.

## Next steps

W3a: registry-independent worker execution with typed DAG validation/cancellation,
private invocation data, stale-run isolation and the independent data extension.
W3b then connects run/cancel/status and a table viewer to this workspace.
