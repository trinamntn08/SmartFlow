# Scene controller migration

Reviewed sources from legacy commit `d6f457e8415445d00eaf7e9fbf9e5047768ea637`
were copied into this repository. `copy-manifest.json` records original byte hashes;
the checkout is never a build or runtime dependency. MIT notices from
`tp_math_utils` and `tp_maps` are included unchanged.

`Plane`, `Ray` and `Intersection` supplement the existing scene-math library.
`adapted/LegacyNavigation.h` adapts the orthographic pan normalization from
`tp_maps/src/controllers/CADController.cpp`, retaining its MIT attribution.
SmartFlow uses a Y-up camera basis, its existing zoom response, and the legacy
cursor-anchored zoom principle. See [local changes](SMARTFLOW_PATCHES.md).

The legacy scene selection/movement layers and object gizmo controllers were
reviewed as behavior references. They depend on the legacy map renderer, scene
assets and timelines and have no module license in that checkout; their source
has not been copied. SmartFlow's object/gesture controllers are independent
implementations using the licensed geometry and keyframe helpers.
