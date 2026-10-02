# Task handoff: VS Code CMake Tools native launch setup

## Objective and scope

The user will test manually and has deferred automated desktop interaction
acceptance. Configure JSON files to build and launch SmartFlow from VS Code's
CMake Tools extension. Keep machine paths local and preserve the future web setup.

## Current state

- Shared `CMakePresets.json` supplies Windows/MSVC x64 configure templates,
  Release/Debug build templates and a Release CTest template. Qt comes from the
  local preset environment. Build/install directories stay under the repository.
- Ignored `CMakeUserPresets.json` is prepared for the installed Qt MSVC kit.
  Its visible presets are `windows-local`, `windows-local-release`,
  `windows-local-debug` and `windows-local-release-tests`. It is intentionally
  excluded from the commit. A portable example is under `docs/examples/`.
- VS Code settings enable preset use, configure-on-open and build-before-run.
  Native CMake Tools launches inherit the configured Qt runtime environment.
  Recommended CMake Tools and C/C++ extensions are already installed locally.
- `.vscode/launch.json` adds selected-target, data and component-example choices
  using the MSVC debugger. CMake Tools supplies the target path and Qt prefix,
  so no build configuration or machine-specific executable path is hardcoded.
- README/development guide document preset selection, launch target selection,
  play/F5 and command-line equivalents. The roadmap records manual acceptance
  as pending. No application/vendor implementation changes were made.

## Verification

- CMake 3.23 accepts `cmake --list-presets=all` and `cmake --preset windows-local`.
- `cmake --build --preset windows-local-release`: passed.
- `ctest --preset windows-local-release-tests`: 17/17 passed using the inherited
  Qt PATH, without a separate PATH override in the test command.
- Windows build-tree `smartflow.exe --smoke-test` launched with the configured
  Qt root and repository working directory via `Start-Process -Wait`, exited 0.
  Screenshot/log: `build/native/vscode-launch-smoke.*`.
- Installed CMake Tools 1.24.42 exposes `cmake.cacheVariable` and
  `cmake.launchTargetPath`; its launch/debug environment implementation merges
  the active configure environment. C/C++ 1.34.4 is installed.
- `npm.cmd run check` passed formatting, TypeScript, nine persistence tests and
  the production build. `git diff --check` passed. Debug preset parsing was
  checked; the separate Debug configuration was not built.
- VS Code's buttons/debugger were not operated through native computer-use.
  Per user request, interactive verification is left to manual testing; it is
  not a blocker for delivering the editor configuration and does not mark N5
  desktop acceptance complete.

## Decisions and open questions

Use supported CMake presets rather than storing the local Qt installation in
tracked editor settings. The local preset can be edited when Qt moves/upgrades.
Launch JSON expects the single Qt root used by these presets in CMAKE_PREFIX_PATH.
Build-tree launch is for development; packaged acceptance still uses the desktop
test folder. No global environment or VS Code installation was changed.

## Next steps

1. User selects the local configure/build preset and `smartflow` or
   `smartflow-data` launch target, then runs the app from CMake Tools.
2. User reports manual scene/data/component/save-reopen observations before N5
   acceptance is marked complete. Automated native computer-use is deferred.
