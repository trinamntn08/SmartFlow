# Task handoff: N7 selectable workspace widgets

## Objective and scope

Restore the user's old-repository selector/split workspace interaction in SmartFlow.
Inspect the old repository read-only and keep build/runtime independent from it.

## Current state

- Added native `PanelWorkspace` region selectors and H/V/X controls; existing
  widgets swap or hide without recreation. Node picking is a selectable panel.
- Separated Inspector and Execution results so either can be placed independently.
- Stored version-1 region trees and divider sizes in active-graph workspace state,
  retaining unknown metadata/IDs and unsupported layouts. Added explicit reset.
- Added a host graph-view adapter to preserve navigation when reparenting/showing
  widgets; vendor QtNodes sources remain unchanged.
- Added `native_panels` behavior tests, bringing CTest entries to 18, and synced
  architecture, testing, development, roadmap and user instructions.

## Verification

Passed native Release configure/build, 18/18 CTest entries, all seven installed
Windows startup/render checks, and `npm.cmd run check` (formatting, TypeScript,
nine persistence tests and web production build). `git diff --check` passed.
Inspected default and rearranged offscreen layouts, including readable widget
selectors, controls, inspector and viewer. Layout changes are workspace-only
and preserve widget identities and graph zoom. The Windows test folder was refreshed.
Real desktop interaction remains manual; automated computer-use stays deferred.

## Decisions and open questions

[Decision 0016](../../architecture/decisions/0016-selectable-workspace-widgets.md)
records the retained layout contract and read-only legacy behavior reference.
Each widget has one live instance. Floating windows, duplicate independent viewers
and an extension widget API remain future work.

## Next steps

Test selectors, split/close, resizing and Save/relaunch/Open in the refreshed
Windows folder. Record manual results and fix demonstrated issues separately.
