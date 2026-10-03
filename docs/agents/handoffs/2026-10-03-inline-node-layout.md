# Task handoff: simpler inline node field layout

## Objective and scope

The user reported that inline field placement was not clean and requested a
simple layout review. This follow-up changes only native graph card presentation.

## Current state

Fields occupy full-width rows with capitalized labels on the left and numeric
values on the right. Even row spacing, one shared background and no small spin
buttons reduce clutter. Up/Down, focused wheel adjustment and typing still use the
same controls and project commands. Timing uses a smaller muted footer.

Geometry uses a consistent ten-pixel inset and fills the available body width,
including nodes with long captions. The controls start after the actual socket
positions rather than independently estimated font metrics. Saved node positions,
undo, execution and component boundaries are unchanged.

## Verification

- `cmake --build --preset windows-local-release`: passed.
- With `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf`,
  `ctest --preset windows-local-release-tests -j 4`: all 32 entries passed.
  Existing inline tests cover actual canvas typing/focus change, stepping,
  cancellation, execution, undo, inspector/viewer sync and save/reopen.
- `npm.cmd run check`: passed formatting, type checks, workspace tests and build.
- `cmake --install build/native --config Release --prefix build/desktop-test`
  and `scripts/Test-DesktopPackage.ps1`: all 13 installed Windows checks passed;
  installed executable hashes match the Release build.
- Inspected native numeric and installed scene/component renders: fields align,
  fit inside the cards and remain clear without spin buttons. Windows interactive
  acceptance of this updated layout remains unreported.
- `git diff --check`: passed. No old-repository edits.

## Decisions and open questions

No package boundary or contract changes. See [inline controls](../../INLINE_NODE_CONTROLS.md).
Manual feedback on this refreshed layout remains unreported.

## Next steps

Try the refreshed desktop graph and refine from concrete feedback.
