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
- `interaction/ObjectInteractionController`: original gesture state, bounded
  move/rotate/scale parameters, disposable preview and one command at release.
- `SceneViewer`: Qt composition, input dispatch and opaque workspace state.
- `native/app/workspace/ViewerCommandAdapter`: domain-independent, revision-gated
  parameter editing and public component-control resolution.
- `native/vendor/scene-controls`: licensed legacy plane/ray helpers and attributed
  CAD pan adaptation; see its manifest and isolated patch notes.

The legacy scene layers were audited for their separation of navigation,
selection and snapshot-based gestures. Their assets, timeline and map renderer
dependencies are not imported. This remains a primitive mesh preview; a scene
hierarchy, imported assets and depth-buffered production renderer are future work.

## Object editing

Open `examples/scene-interaction-tool.smartflow` for a Use-mode component exposing
all supported transform controls. The default Build scene also supports editing.
Choose an object by clicking its mesh or using the object selector. Choose Move
(G), Rotate Y (R) or Scale (S), then drag the mesh. Move edits the selected X/Y/Z
axis; clicking a colored axis handle selects and drags that axis. Rotate Y uses
horizontal motion; Scale applies one factor to all axes, preserving their ratio.
Ctrl snaps translation to 0.1 units, rotation to 5 degrees or scale factor to 0.1.

The drag previews original geometry without changing the project. Release commits
one undoable graph command. Escape cancels. Camera navigation, switching controls,
output replacement or project edits cancel unfinished gestures. MMB/Shift-drag
still pans; right drag orbits. Saving/reopening reproduces the edited graph.

The viewer edits the latest Transform driving the chosen mesh and previews all
objects sharing it; the status reports the affected count. A cube without a
Transform is read-only. Axis moves pointing directly into the camera require
another view. Parameters clamp to their declared ranges; oversized preview
coordinates are rejected. In Use mode, the selected component must expose the
requested controls. The older scene-tool exposes only position X and rotation Y,
so Y/Z moves and scale explain the missing controls without changing the project.

The new example uses a new component identity. Its saved definition remains
immutable; edits update instance overrides. See [decision 0024](architecture/decisions/0024-native-viewer-commands.md).
