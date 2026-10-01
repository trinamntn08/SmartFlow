# Checkpoint: desktop test release direction and verification

## Objective and scope

Make the first user test version explicitly the native desktop app and verify
that the current preview works. This is a delivery/verification checkpoint after
N5f, not completion of N5 or workspace persistence.

## Current state

- AGENTS, README, product proposal, roadmap and development guides now require
  Windows C++/Qt desktop delivery first. Browser delivery is deferred. See
  [decision 0009](../../architecture/decisions/0009-desktop-test-release.md).
- The Windows CMake install target prepares `build/desktop-test/bin/smartflow.exe`
  with Qt runtime/plugins/configuration and app-local MSVC runtime libraries.
  The install prefix must be absolute; the testing guide uses `$PWD`.
- Added CTest scene/numeric app startup checks and a scene inspector-edit/save/
  fresh-window-reopen regression, including visible viewer output and canvas nodes.
- Open now picks a registered terminal output by default and fits canvas contents
  into the view. This fixes invisible loaded results and clipped canvas nodes
  while full workspace restoration is pending. No scene import was added to core.
- [Desktop testing guide](../../DESKTOP_TESTING.md) provides build/install/run
  commands, the acceptance walkthrough and current preview limitations.

## Verification

- CMake configure, Release build and install passed with process permission.
  Used the documented direct scene-test cmake_autogen workaround before building.
  Initial relative install prefix was rejected by Qt; documented absolute command
  succeeded. Deployment warned about absent DX12 compiler DLLs; current software
  preview/startup tests do not use Direct3D 12.
- All eight CTest checks passed. After the final canvas fit adjustment, rebuilt
  and reran scene/workspace and both app-startup checks: all four passed.
- Packaged native-window smoke startup passed with PATH limited to Windows system
  directories and QT_PLUGIN_PATH/QT_QPA_PLATFORM_PLUGIN_PATH/QT_QPA_PLATFORM cleared.
  Screenshot: `build/native/desktop-packaged-smoke.png`. This validates the local
  folder without a developer Qt PATH, not a clean Windows installation.
- Used the computer-use skill on the real packaged desktop window. Verified size
  edit 2 to 3, Apply/recompute, Undo to 2, Redo to 3, and 3D orbit. Native Save As
  saved `build/desktop-test/desktop-verified.smartflow`, with a clean filename title.
  Closed/relaunched, opened through the native dialog, and confirmed size 3 and a
  rendered scene. A later edit to 4 exercised the close prompt, Cancel preserving
  the active edit, and Ctrl+S clearing the dirty marker. Read back size 4 from disk.
- Save/Discard branches of the close/Open prompts were not all exercised manually;
  file failure isolation is covered by the native tests. Final canvas fitting was
  validated in the scene regression; the earlier real-desktop run exposed the issue.
- `npm.cmd run check` passed: formatting, typechecks, nine persistence tests and
  production build. These maintain the deferred prototype and are not desktop
  acceptance evidence.

## Decisions and open questions

This is a local development preview. Canvas/viewer configuration persistence,
the independent non-3D extension, public installer/signing and clean-machine
compatibility remain unfinished. Domain and audience remain open. No old
repository edits, new downloads, plugin system or delegation were introduced.

## Next steps

1. Continue N5 with canvas/viewer workspace persistence and scene reopen acceptance.
2. Add the separate non-3D workflow through the native extension contracts.
3. Use the desktop guide for each user test build; record actual desktop checks
   and limitations. Keep checkpoints separate and committed.
