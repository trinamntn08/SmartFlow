# Checkpoint N4: included native scene workflow

## Objective and scope

Continue from N3 with the required primitive/transform/material/viewer slice,
reuse audited legacy sources without modifying the old repository, and keep
scene types out of the platform workspace and execution implementation.

## Current state

- `extensions/scene-3d/native` contributes Cube, Transform, Material, Scene and
  Merge nodes with versioned identifiers and owned scene output values.
- Default app starts Cube → Transform → Material → Scene, pinning the last output.
  Parameter Apply and undo/redo recompute the viewer. Orbit, zoom, frame and object
  selection remain viewer state; they do not issue project edits.
- Copied 391 source/header/license files for Geometry3D, MeshKeyFrame, Material,
  required concrete material variants, GLM headers and nanoflann. No vendor source
  modifications. See [audit](../../../native/vendor/scene-math/README.md).
- Native contribution contracts in `native/sdk/WorkspaceExtension.h` supply
  node presentation, registry/factory composition, presets and a viewer factory.
  Workspace and executor contain no scene imports. Main is the composition root.
- `--numeric` constructs the numeric workflow without registering scene nodes or
  constructing a scene viewer. Its behavior suite does not link the scene library.
- The inspector scrolls for larger parameter sets. Output pinning separates the
  viewed result from the node currently being edited. Invalidated outputs clear.
- Nonuniform transforms recompute normals after the copied transform routine;
  parameter and coordinate bounds and a 64-object cap produce execution errors.
- Roadmap, architecture decision, extension guide and development guide updated.

## Verification

- `cmake -S . -B build/native -G "Visual Studio 17 2022" -A x64
-DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/msvc2022_64`: passed.
- `cmake --build build/native --config Release --parallel 8`: passed.
- The previously documented nested Qt moc process restriction recurred. Direct
  `cmake -E cmake_autogen .../AutogenInfo.json Release` for changed workspace and
  test targets with process permission, followed by rebuilding, succeeded.
- `ctest --test-dir build/native -C Release --output-on-failure`: all four suites
  passed. Scene report has seven entries including setup/cleanup, zero failures.
  Its five cases cover geometry/bounds/normals/material snapshot independence,
  inspector recomputation and undo/redo image restoration, camera and selection
  separation from project revision/undo, merge/invalid input feedback, and the
  64-object cap. Report: `build/native/native/scene-results.xml`.
- Both default scene and `--numeric` startup checks with `--smoke-test -platform
offscreen --smoke-font C:/Windows/Fonts/segoeui.ttf` exited 0, confirmed by
  waiting for the GUI processes. Inspected `build/native/n4-smoke.png` and
  `build/native/n4-numeric-smoke.png`: readable labels/controls; connected scene
  nodes, colored shaded cube and Complete results; numeric output remains 42.
- `npm.cmd run check` with process permission: passed formatting, all workspace
  type checks, eight persistence tests and production build.
- Verified all 391 source and copy hashes against the manifest: zero mismatches.
  Legacy HEAD remains `d6f457e8415445d00eaf7e9fbf9e5047768ea637`; its only status
  entry remains the pre-existing untracked `documentation/VisualWorkspaceProposal.md`.
- No old checkout paths found in app, tests, SDK, extension code or root/native
  build entry points. Historical paths exist only in audit documentation.

## Decisions and open questions

See [decision 0006](../../architecture/decisions/0006-native-scene-extension.md).
The checkpoint reuses lower-level legacy geometry/material modules, not the full
asset/timeline-dependent scene application or its renderer. The preview uses
orthographic projection, fixed lighting and sorted painted faces. Intersecting
surfaces are not reliably rendered. A production depth-buffered renderer, import,
textures, transparency, hierarchy editing and configurable lighting are pending.

This is static native extension composition, not a production plugin system or
stable file format. Scene output serialization is explicitly unsupported. Native
project persistence and unknown-extension preservation remain N5. The numeric
fixture does not replace the planned independent data/text extension validation.

Only Windows/MSVC and offscreen Qt rendering/input were verified. No real desktop
input, default desktop font discovery, GPU path, installer or cross-platform
verification is claimed. Runtime scene work is bounded; no large-scene performance
claim is made. Existing distribution-license decisions remain unresolved.

## Next steps

N5: native project save/reopen with explicit versioning and lossless unknown
extension content, then a separate data/text extension through the same public
native contracts. Preserve project/workspace/result separation. Keep broader 3D
renderer work visible as pending rather than treating this preview as production.
