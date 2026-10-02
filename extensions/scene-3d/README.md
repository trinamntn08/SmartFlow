# 3D scene extension

Status: native primitive preview introduced at N4, integrated with project/workspace
persistence and collapsed components through N6f.

`src/` implements cloneable browser scene values, type validation and the five
primitive operations, with native persisted IDs/ports/parameter ranges. Shared
fixtures compare transform coordinates. `apps/web` composes a bounded orthographic
canvas viewer; see [decision 0022](../../docs/architecture/decisions/0022-browser-scene-preview.md).

The required included domain package. `native/` implements Cube, Transform,
Material, Scene and Merge nodes, a versioned scene output type, and an interactive
orthographic preview. Geometry, transform composition and materials reuse the
audited source in `native/vendor/scene-math`; the old checkout is never required.

The main app starts with Cube → Transform → Material → Scene. Edit parameters
through the inspector and Apply; the pinned Scene output updates with undo/redo.
Pin output changes the viewed node without changing graph data. Drag the preview
to orbit, scroll to zoom, double-click to frame, and click an object to select it.
These viewer actions update saved workspace state without changing graph semantics or entering the undo stack.

Transform supplies translation, Y rotation and nonuniform positive scaling.
Material sets RGB albedo. Scene exposes one input as the final scene; Merge joins
two required scene inputs. Outputs own independent geometry/material values.
The scene model is currently a flat list of objects with source-node identities.

This is a bounded opaque-primitive preview, not a production renderer: fixed
lighting, depth-sorted painted faces, at most 64 generated boxes and coordinates
within 100,000 units. Intersecting surfaces are not rendered reliably. Textures,
transparency, PBR shading, import, hierarchy editing, configurable lights and
configurable camera projections are pending. Camera and object selection are persisted through opaque native viewer hooks; computed scene results are not serialized.

Native behavior tests are in `native/tests/SceneTests.cpp`; they run with CTest.
The source-level contribution interface is in `native/sdk/WorkspaceExtension.h`.
`smartflow.exe --numeric` constructs only the independent numeric workflow.
There is no JavaScript package here yet; TypeScript runtime integration is deferred.

Open the shipped `examples/scene.smartflow` to restore scene parameters and camera.
The same scene nodes can be exposed through a collapsed component with a size
control and scene output; native tests cover edits, undo and save/reopen. Library
removal leaves retained instance snapshots usable. See
[components](../../docs/GRAPH_COMPONENTS.md),
[VS Code launch](../../docs/NATIVE_DEVELOPMENT.md#run-from-vs-code-with-cmake-tools)
and [desktop testing](../../docs/DESKTOP_TESTING.md). Automated scene/UI/startup
checks pass; packaged interaction acceptance is pending the user's manual results.
