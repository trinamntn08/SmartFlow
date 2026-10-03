# Native scene and interaction

The bundled scene-3d extension owns its domain model, camera, rendering and object
controllers. A project can still use only numeric or data workflows. The separate
`smartflow-data` executable does not link the scene libraries.

## Navigation and selection

Choose Orbit, Front, Right or Top in the viewer. Left drag or right drag orbits;
middle drag or Shift-drag pans. Wheel zoom keeps the cursor's focal-plane point
fixed. Frame, F or double-click fits the current scene. Clicking selects a mesh.
Camera and selection are workspace state and create no graph undo commands.
Build and Use keep separate viewer state.

Object identifiers derive from source nodes and merge branch paths. Selection
survives output invalidation and reordering; a missing object does not silently
select another mesh. Old files with an index-only selection remain readable.
Picking intersects actual triangles and chooses the closest visible surface,
independently of whether the widget has already painted.

## Module boundaries

- `extensions/scene-3d/native/model`: immutable output data, transient transform
  provenance, selection and bounds.
- `controllers/CameraController`: Y-up orthographic camera and projection;
  no widget or project commands.
- `rendering/SceneRenderer`: QPainter preview and triangle picking.
- `SceneViewer`: Qt composition, input dispatch and opaque workspace state.
- `native/vendor/scene-controls`: licensed legacy plane/ray helpers and attributed
  CAD pan adaptation; see its manifest and isolated patch notes.

The legacy scene layers were audited for their separation of navigation,
selection and snapshot-based gestures. Their assets, timeline and map renderer
dependencies are not imported. This remains a primitive mesh preview; a scene
hierarchy, imported assets and depth-buffered production renderer are future work.
