# Task handoff: native inline node controls

## Objective and scope

The user requested editable parameters inside graph nodes, similar to Blender,
instead of requiring a separate inspector widget. This checkpoint changes native
Build-mode cards across numeric, scene and data workflows; the inspector and Use
mode remain available through shared project commands.

## Current state

`NodeParameterPanel` embeds compact numeric controls and existing execution timing.
Enter/focus change applies typed values; spin steps apply immediately. Escape
cancels drafts and an unfocused field leaves wheel events to canvas navigation.
Semantic changes refresh fields with blocked signals and retain widgets where
possible; timing updates do not replace drafts. Stable IDs/revisions prevent
stale disposable StepDetails access. Declared bounds/enabled state apply.

Transient steps retain delegate parameter order and component facades retain
authored public-control order. Component definitions remain immutable and private
body parameters stay hidden. Unknown node/parameter content remains retained.

`NodeBodyGeometry` places sockets above fields, keeping cards compact without
changing saved positions/navigation. The vendored QtNodes geometry setter is a
small isolated hook documented in its patch notes. Layout and edit semantics
remain in the application; no scene imports or dependencies were added.

## Verification

- `cmake --preset windows-local`: configuration passed.
- `cmake --build --preset windows-local-release`: Release build passed.
- With `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf`,
  `ctest --preset windows-local-release-tests -j 4`: all 32 entries passed.
- `npm.cmd run check`: formatting, TypeScript, 15 web adapter tests, 45 core
  tests, four SDK tests, five runtime prototype tests and production build passed.
- `cmake --install build/native --config Release --prefix build/desktop-test`
  and `scripts/Test-DesktopPackage.ps1`: installation and all 13 Windows startup/
  execution/render configurations passed with developer Qt environment removed.
  Installed executable SHA256 values match the final Release build.
- Visually inspected inline numeric, scene and component offscreen renders and
  installed Windows scene/component renders. Socket labels and fields fit the
  compact cards; the scene preview renders after inline edits.
- `git diff --check`: passed. No old-repository edits or runtime references.

New Qt tests exercise actual
proxy-widget canvas clicks/typing, no graph movement while editing, deferred
typing/Enter, stepping/Escape, execution, undo/redo, inspector/viewer sync,
component isolation, deletion, external draft invalidation, opaque data and
save/reopen. Existing timing-label tests now locate the footer inside each card.

## Decisions and open questions

See [decision 0025](../../architecture/decisions/0025-native-inline-node-controls.md)
and [inline controls](../../INLINE_NODE_CONTROLS.md). Existing bundled numeric
parameters are editable; other types remain read-only until concrete editors are
needed. Interactive Windows acceptance of these new controls remains unreported.
The previous positive manual feedback applies to an earlier desktop version.

## Next steps

Try the refreshed native Build canvas and refine parameter ergonomics from user
feedback. Richer parameter types or numeric scrub gestures can be separate work.
