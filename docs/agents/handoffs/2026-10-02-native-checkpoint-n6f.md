# Task handoff: N6f undoable component-library removal

## Objective and scope

Continue after the verified N6e commit with explicit removal of component-library
entries, preserving independent retained instance snapshots. Keep the native
platform domain-independent and retain required scene and separate data workflows.

## Current state

- `DocumentSession::removeCatalogComponent(index)` removes exactly one retained
  entry from a catalog array. Missing/unsupported catalogs and invalid indices
  reject transactionally. Opaque and duplicate entries can be selected explicitly.
- `DocumentHistory` wraps removal as one undoable semantic edit. Undo restores
  original order and unknown content while preserving current workspace state.
  Failed removal does not discard redo history.
- The library has **Remove from library...**. Confirmation uses plain text and
  defaults to No, explaining that existing instance snapshots survive and Undo
  restores the entry. Revision checks run before confirmation and before mutation.
- After removal, the dialog refreshes its list/bindings, selects a neighboring entry,
  and disables removal if empty. Unsupported catalog containers are retained.
- No files, graph nodes, definitions in instance snapshots or external assets are
  deleted. Existing collapsed instances still execute and save/reopen even after
  the catalog becomes empty. This does not change definition identity/version.
- No dependencies, vendor changes or old-checkout references were added.

## Verification

- `cmake --build build/native --config Release -j 4`: passed with Qt 6.11.1/MSVC.
- With Qt `bin` on PATH and `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf`,
  `ctest --test-dir build/native -C Release --output-on-failure`: 17/17 passed.
  JUnit reports confirm both new removal behavior tests ran. The UI rendering
  capture was refined to wait for refreshed controls to finish layout and the UI
  check rerun before committing.
- `npm.cmd run check`: formatting, type checking, nine persistence tests and build
  passed. `git diff --check` passed.
- CMake install to the absolute `build/desktop-test` prefix passed (existing
  optional DX12 compiler warning). Packaged Windows scene, independent data and
  collapsed-component startup processes all exited 0. Screenshots/logs are under
  `build/native/n6f-*-packaged.*`; these do not verify interactive library removal.
- Model coverage includes duplicate/opaque entries, exact graph preservation,
  invalid-index redo retention, workspace isolation, empty/unsupported catalogs
  and instance execution/save/reopen with an empty catalog.
- UI coverage includes No/Yes confirmation, default response, unavailable entry
  removal, list refresh, empty/stale rejection, undo/redo and restored viewer output.
  Render artifact: `build/native/native/component-library-removal-smoke.png`.
- Native Windows computer-use remains unavailable: the current session's
  `sky.list_windows()` reported native pipe unavailable, file not found (OS error 2).
  Qt UI/offscreen tests and packaged startup do not constitute real desktop
  interaction acceptance. Full N5 remains pending that verification.

## Decisions and open questions

The [decision 0014 follow-up](../../architecture/decisions/0014-native-component-files.md)
records index/revision targeting and snapshot isolation. See
[the component guide](../../GRAPH_COMPONENTS.md). Definition editing, nested
components, migration, assets and automatic snapshot updates remain future work.
The native migration has no production runtime or dynamic plugin loader.

## Next steps

1. Complete packaged desktop interaction acceptance when native computer-use is
   available, covering scene, data, components and library import/export/removal.
2. Decide how component definition editing creates new identities/versions and
   whether explicit instance updates should exist; preserve reproducibility.
3. Keep nested execution semantics, asset exchange and distribution separate.
