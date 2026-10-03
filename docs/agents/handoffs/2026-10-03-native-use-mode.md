# Task handoff: native Use mode

## Objective and scope

Implement the user's authorized next generic product experiment: operate a
prepared graph through a component's exposed controls and result viewer, then
return to Build. Prove it with required 3D support and independent data workflows.

## Current state

- `UseWorkspace` selects collapsed instances and their exposed outputs, presents
  numeric controls in authored interface order, and uses an independent viewer
  supplied by the configured extension. Unsupported controls retain their values.
- Build/Use switching keeps the canvas and panel widgets alive. Component/layout
  menus and Build output pinning are hidden or disabled in Use; file, undo/redo,
  Run/Cancel, live updates and scheduling remain available.
- Apply uses the existing semantic parameter command. Both modes share the same
  background executor, cancellation, diagnostics and stale-result rejection.
  Obsolete parameter editors cannot apply to a newer project revision.
- `smartflow.native-use@1` stores per-graph mode, component/output selection and
  opaque viewer state. Hidden Build navigation/panel sizes do not overwrite the
  last visible saved state. Unknown namespaces/fields/viewer shapes are retained.
- `--use` overrides the restored workspace mode. Smoke mode exits directly so a
  workspace override cannot open an unsaved-changes prompt during startup checks.
- `examples/scene-tool.smartflow` is a complete scene component with size,
  position X, rotation Y and RGB controls. The existing data component example
  works with `--use`. Both are installed in `build/desktop-test`.
- Existing table UI checks now locate the Build viewer explicitly because both
  modes own independent table widgets.

## Verification

- `cmake --preset windows-local`: passed.
- `cmake --build --preset windows-local-release`: passed.
- `$env:SMARTFLOW_TEST_FONT='C:/Windows/Fonts/segoeui.ttf'; ctest --preset windows-local-release-tests -j 4`:
  all 27 entries passed. The new suite covers scene/data edits, undo/redo, live
  and explicit runs, cancellation, stale editors, output selection, save/reopen,
  Build layout/navigation/selection retention, unsupported components and opaque
  future workspace content.
- `npm.cmd run check`: passed (formatting, type checks, workspace tests, Chromium
  interaction tests and production build). No browser interface behavior changed.
- `cmake --install build/native --config Release --prefix "$PWD/build/desktop-test"`:
  passed; Qt/MSVC runtimes, both executables, examples and documentation refreshed.
- `.\scripts\Test-DesktopPackage.ps1`: all twelve installed Windows startup,
  execution and render configurations passed with developer Qt environment
  variables removed. Installed executable hashes match the build.
- Visually inspected `build/native/native/use-data-smoke.png`,
  `use-scene-smoke.png` and packaged `verification/scene-tool.png`, `data-tool.png`.
  Labels/controls are readable, long interfaces scroll, and both viewers render.
- `git diff --check` and Markdown formatting checks passed.

The process sandbox blocked MSBuild/Node subprocesses, Qt deployment and tests'
Windows temporary-file saves. These checks passed with process/file access;
nested Qt moc failures were resolved using direct `cmake -E cmake_autogen` calls.
Initial failures exposed hidden-panel size capture and smoke quit/save behavior;
both were corrected before the final passing run.

Real desktop interaction of this new interface and clean-machine compatibility
were not verified. The user's positive manual feedback applies to the preceding
version; automated native computer-use remains deferred.

## Decisions and open questions

[Decision 0023](../../architecture/decisions/0023-native-use-mode.md) keeps tool
interfaces on existing component contracts and state outside semantic history.
The audience/domain remain open. This is an interface within SmartFlow; no
standalone tool executable, dynamic plugin loader or production rendering added.
Only existing numeric editors and one selected tool/output viewer are supported.

## Next steps

Try the packaged examples and refine Use mode from concrete feedback. Real data
input/output, richer controls, multiple tool viewers and browser Use mode remain
separate candidate milestones rather than approved implementation work.
