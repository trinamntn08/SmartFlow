# 0006: Bundled native scene extension

Status: accepted for checkpoint N4, 2026-09-30.

## Context

Required 3D must work through the graph without making a scene mandatory in all
projects. N3 only composes numeric nodes. The old full scene application includes
asset services, timelines and renderer dependencies beyond this vertical slice.

## Decision

Place native 3D nodes, scene output and viewer in `extensions/scene-3d/native`.
Copy the required lower-level Geometry3D, MeshKeyFrame, Material, GLM and
nanoflann dependencies with provenance and notices into `native/vendor/scene-math`.
Do not link the original checkout or copy the old application configuration.

Expose initial source-level native contribution contracts in `native/sdk`:
registrations, presentation metadata, demo presets and an output viewer factory.
The composition root chooses contributions. The graph, canvas adapter and worker
do not import the scene extension. Workspace tests continue linking without it;
`--numeric` runs without constructing a scene registry or viewer. This is static
composition, not a binary plugin ABI, dynamic loader, or stable persisted schema.

The 3D extension owns versioned scene and node identifiers, Cube, Transform,
Material, Scene and Merge nodes. Outputs own geometry/material values, copied
before modification. Scene preserves a single input as a viewable scene; Merge
combines two required scene inputs. This first scene representation is a flat
object list, separate from the execution graph. Runtime result serialization is
explicitly unsupported; N5 will persist graph parameters, not these results.

Use a small orthographic Qt painter preview for opaque primitives. Orbit, zoom,
frame and object selection change viewer state only. A pinned result stays visible
while editing other nodes; invalidation clears it immediately. No viewer control
currently edits geometry; future editing controls must issue project commands.

## Consequences and limits

N4 proves primitive/transform/material execution and interactive preview with the
existing undo and revision-gated publication model. The preview has fixed lighting
and depth-sorted faces, not depth-buffered/PBR rendering. Intersecting surfaces,
transparency, textures, asset import, scene hierarchy editing, configurable lights,
camera persistence and a production GPU renderer remain future work.

The first slice bounds scenes to 64 generated boxes and coordinates within
100,000 units. Mesh work runs on the worker; bounded projection/painting runs on
the UI thread. No large-scene performance claim is made. Copied unused math APIs
are not exposed as imported-file parsing or project persistence. N5 still needs
unknown-extension preservation and a separate non-3D extension workflow.
