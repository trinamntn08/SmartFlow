# 0016: Selectable workspace widgets in split regions

Date: 2026-10-02
Status: accepted for N7.

## Context

The user requested selectable widgets placed anywhere in the app window, like
the old pipeline workspace. Read-only inspection of the old `PipelineWorkspace`
and `tp_qt_application_framework::SplitWidget` confirms region selectors,
horizontal/vertical splits, close controls and layout persistence. The current
SmartFlow window had a fixed graph/viewer/inspector split.

## Decision

Implement native `PanelWorkspace` in the workspace adapter, with a widget selector
and H/V/X controls in each leaf region. H adds a right-hand region, V adds a region
below, and X closes a region. Dividers resize the regions. Available widgets are
Graph, Node library, Inspector, Execution results and the configured Result viewer.
The numeric workflow has no domain viewer. **Layout > Reset widget layout**
restores the default arrangement. The node picker/Add node controls move from the
global toolbar into the selectable Node library widget.

Each widget has one live instance. Selecting an already visible widget swaps the
two region assignments. Closing hides its widget while leaving it selectable.
Widgets keep a QWidget parent during moves, preserving inspector values and viewer
objects. A workspace graph-view adapter suppresses QtNodes' automatic refit on
Show; explicit initial/default fitting and saved navigation remain host-owned.

Persist `panels` beside navigation/selection in the existing native editor
workspace entry for the active graph. Its version-1 object stores a binary tree:
leaves have stable `panel` IDs; branches have `split` orientation, two `children`
and two integer `sizes`. Preserve unknown metadata in supported trees. Unavailable
widget IDs display a retained placeholder and can be explicitly replaced. Reject
duplicate nonempty assignments, invalid trees and excessive size/depth; limit to
32 regions. Future/malformed whole layouts remain untouched in the document and
use the default visible layout. Explicit Reset authorizes replacing that layout.

Layout changes update workspace dirty state only, without semantic undo entries,
execution revision changes or recomputation. Save/Open restores assignments and
divider proportions. Outer window geometry remains separate and is not persisted.
Existing projects without panel state use the default layout.

## Alternatives and consequences

QDockWidget docking would offer a different interaction from the inspected
selector/split workflow. Copying the old display/application framework would
introduce unnecessary dependencies. This implementation uses existing Qt Widgets
and fresh host code; no old source, settings or assets were copied or modified.
The build/runtime does not depend on the old checkout. Multiple independent
canvases/viewers, floating windows and extension widget registration remain future
work; current panels use the existing contribution configuration.

## Verification

Native tests cover UI selector swaps, splitting/closing, live widget identity,
navigation preservation, workspace-only edits, persisted divider proportions,
save/reopen execution, reset and unknown/future content retention. Existing scene,
data, component and file suites must continue to pass. Inspect default/custom
offscreen rendering and refresh the Windows test folder. Full manual interaction
acceptance remains pending user results.
