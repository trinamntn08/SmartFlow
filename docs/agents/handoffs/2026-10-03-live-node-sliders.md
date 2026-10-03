# Task handoff: live node slider updates

## Objective and scope

The user wants changes applied while dragging, rather than only on release.

## Current state

Sliders send the first value immediately and coalesce subsequent movements at
40 ms intervals. The graph, inspector and live execution update during a drag;
release flushes the final value. Execution's existing 80 ms scheduling window no
longer restarts for every change, avoiding starvation under continuous input.
Work remains asynchronous and obsolete execution results remain suppressed.
Disabling Live still updates the project and requires Run for computed results.

DocumentHistory accepts explicit unique edit-group tokens. Consecutive commands
within one slider gesture merge their final semantic snapshots, preserving the
original undo snapshot. Other edits never merge into the gesture. Escape restores
the starting value through the same group and removes a net-zero command.
External changes cancel pending timer/pointer work without overwriting new values.
Normal edits, saved schema and workspace state retain their existing semantics.

## Verification

- `cmake --build --preset windows-local-release`: passed.
- With `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf`,
  `ctest --preset windows-local-release-tests -j 4`: all 32 entries passed.
  Slider tests verify semantic updates and completed execution while the mouse
  remains held, multiple updates merged into one undo, no release-only edit,
  Escape restoration/net-zero history and external-change cancellation.
- `npm.cmd run check`: formatting, types, workspace tests and build passed.
- `cmake --install build/native --config Release --prefix build/desktop-test`
  and `scripts/Test-DesktopPackage.ps1`: all 13 installed Windows startup/render
  checks passed, with executable hashes matching the final Release build.
- Inspected the packaged scene render; field placement and sliders remain intact.
- `git diff --check`: passed. No old-repository edits.

## Decisions and open questions

No package boundary changes. Slider cadence balances responsive updates with
bounded graph recompilation. Slow graphs may take longer to produce results;
manual Windows feedback on the new behavior remains unreported.

## Next steps

Try continuous slider changes with Live enabled in scene and data workflows.
