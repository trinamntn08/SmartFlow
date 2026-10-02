# Task handoff: first Windows version ready for testing

## Objective and scope

Finish the first native test version after separately committed N6g (`0ba91fa`)
and N6h (`e5b211e`). Refresh the runtime folder, verify its installed executables,
and provide clear test instructions and a result sheet.

## Current state

- `build/desktop-test` contains both executables, Qt/MSVC runtimes, examples and
  README/testing guides. Build artifacts remain ignored by Git.
- Added `scripts/Test-DesktopPackage.ps1` for executable hash comparison, required
  runtime checks and seven actual Windows startup/render configurations with
  developer Qt variables removed and PATH reduced to system folders.
- The script records screenshots, source commit and binary hashes in the package
  verification folder. Failed runs remove the previous success report first.
- Added [test-version notes/result sheet](../../TEST_VERSION.md) and synced the
  roadmap/README/desktop procedure. No desktop interaction automation was resumed.

## Verification

Passed `cmake --preset windows-local`, Release build and CMake install.
Passed eight relevant CTest startup/project-launch entries after the package
configuration change. Passed all seven `Test-DesktopPackage.ps1` Windows checks
with binary hashes equal to the build, runtime files present and screenshots saved.
Passed `npm.cmd run check`: formatting, TypeScript, nine persistence tests and web
build. `git diff --check` passed.
Previously passed N6h native checks cover all 17 CTest entries, including offscreen
startup. Packaged Windows scene and component rendering screenshots were inspected
and show expected cube/table results with readable controls.

## Decisions and open questions

The first Windows app is ready for user testing. Full N5 manual interaction
acceptance remains pending; no manual results were supplied. Clean-machine/public
distribution remains separate. Definition copies and explicit updates are delivered;
general migration, nested components and asset import remain future work.

## Next steps

Launch the installed executables and follow the test sheet. Record reported
results and fix demonstrated issues in separate verified commits.
