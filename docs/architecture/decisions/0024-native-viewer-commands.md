# 0024: Generic viewer commands and modular native scene interaction

Status: accepted.

## Context

The user requested legacy scene, object interaction and controller integration,
with clear modules. Navigation and selection are workspace state; object edits
must reproduce through the graph and share undo with the inspector. The platform
must continue to support projects and extensions with no scene types.

The old map/scene controllers depend on asset databases, timelines, renderer
services and a Z-up camera. They cannot be copied wholesale into this preview.

## Decision

The scene-3d extension owns model/snapshots, camera controllers, rendering/picking
and a snapshot-based object gesture controller. Licensed ray/plane/keyframe
helpers are reused; camera pan is an attributed CADController adaptation.
Legacy selection and transform layers guide responsibilities without importing
their scene asset/timeline/map dependencies. The application keeps its Y-up
coordinates and existing versioned viewer workspace fields.

The source-level native SDK adds an optional `ViewerCommands` capability for
bounded double parameters. A viewer queries an execution node/parameter and
requests a revision-gated atomic edit. The application resolves ordinary graph
nodes or public controls on an expanded component. Use viewers are scoped to
their selected component; unexposed controls are read-only. No domain imports
enter the SDK or command adapter, and no binary plugin ABI is promised.

Each drag captures original transform parameters and immutable geometry. Move
uses the focal plane and world axis; rotation edits the existing Y rotation;
scale applies one bounded factor preserving axis ratios. Preview applies the
matrix delta to all meshes sharing that Transform. Release creates one history
command; Escape, navigation, output replacement or project changes discard it.
Published execution outputs and embedded component definitions stay untouched.

## Alternatives

Directly mutating viewer geometry would lose changes on recomputation and bypass
undo/save. A scene-specific command in the platform would violate domain
independence. Copying the old renderer/controllers would bring unneeded assets,
timeline and renderer dependencies into this bounded prototype.

## Consequences and verification

Transforms intentionally edit the latest Transform driving an object, potentially
affecting a group. The viewer reports that scope. Earlier graph parameters remain
available in Build mode. Cube generation without a Transform is read-only.
Rotation is Y-only and the renderer remains a QPainter primitive preview.
This does not implement production rendering, asset import, scene hierarchy,
timelines, a plugin loader or a general manipulator/animation system.

Native tests exercise camera math, stable branch identities, nearest picking,
immutable previews, cancellation, stale revisions, atomic undo, component control
scope and save/reopen. Independent numeric/data command tests link no scene
library. Both offscreen input/rendering and installed Windows startups are
required; human desktop testing of new gestures remains separately reported.
