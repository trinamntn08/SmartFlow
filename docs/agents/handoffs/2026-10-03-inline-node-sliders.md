# Task handoff: inline numeric parameter sliders

## Objective and scope

The user requested slider bars to change node parameters easily. Numeric graph
rows now contain a horizontal slider below their label and precise entry field.

## Current state

Dragging previews the numeric field locally and commits one existing project
command on release. Escape cancels. Keyboard/track adjustments commit immediately.
Undo, typing, inspector/viewer edits and exposed component controls share the
same document. External changes cancel pending drags, including subsequent
pointer movement/release. Unfocused wheel events remain available to the canvas.

Sliders normalize double ranges to 10,000 positions and use parameter step sizes
for keyboard adjustment. Domains wider than 10,000 use a stable window of at least
200 units/200 steps around the value; typing outside it moves the window. Full
numeric bounds remain available in the entry field, and tooltips show the window.
Enabled state and ranges come from current compiled metadata. No schema, package
boundary or concrete domain dependency changes.

## Verification

- `cmake --build --preset windows-local-release`: passed.
- With `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf`,
  `ctest --preset windows-local-release-tests -j 4`: all 32 entries passed.
  Added mouse drag/release, one-command undo, Escape, external-update cancellation,
  keyboard stepping, large typed values and numeric-bound checks to inline tests.
- `npm.cmd run check`: formatting, type checks, workspace tests and build passed.
- `cmake --install build/native --config Release --prefix build/desktop-test`
  and `scripts/Test-DesktopPackage.ps1`: all 13 installed Windows checks passed;
  executable hashes match the final build.
- Visually inspected native scene and installed scene/component renders. Compact
  slider tracks fit below the values without overlapping sockets or labels.
- `git diff --check`: passed. No old-repository edits.

## Decisions and open questions

The graph/scene updates on release; the number previews during dragging. Manual
Windows feedback on the new sliders remains unreported. See
[inline controls](../../INLINE_NODE_CONTROLS.md).

## Next steps

Try slider ergonomics with scene and independent data/component parameters.
