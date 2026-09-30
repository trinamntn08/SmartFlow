# 3D scene extension

Status: initial native implementation, checkpoint N4.

The required included domain package. `native/` implements Cube, Transform,
Material, Scene and Merge nodes, a versioned scene output type, and an interactive
orthographic preview. Geometry, transform composition and materials reuse the
audited source in `native/vendor/scene-math`; the old checkout is never required.

The main app starts with Cube → Transform → Material → Scene. Edit parameters
through the inspector and Apply; the pinned Scene output updates with undo/redo.
Pin output changes the viewed node without changing graph data. Drag the preview
to orbit, scroll to zoom, double-click to frame, and click an object to select it.
These viewer actions do not edit the project or enter the undo stack.

Transform supplies translation, Y rotation and nonuniform positive scaling.
Material sets RGB albedo. Scene exposes one input as the final scene; Merge joins
two required scene inputs. Outputs own independent geometry/material values.
The scene model is currently a flat list of objects with source-node identities.

This is a bounded opaque-primitive preview, not a production renderer: fixed
lighting, depth-sorted painted faces, at most 64 generated boxes and coordinates
within 100,000 units. Intersecting surfaces are not rendered reliably. Textures,
transparency, PBR shading, import, hierarchy editing, configurable lights and
camera persistence are pending. No 3D project/result serialization is exposed.

Native behavior tests are in `native/tests/SceneTests.cpp`; they run with CTest.
The source-level contribution interface is in `native/sdk/WorkspaceExtension.h`.
`smartflow.exe --numeric` constructs only the independent numeric workflow.
There is no JavaScript package here yet; TypeScript runtime integration is deferred.
