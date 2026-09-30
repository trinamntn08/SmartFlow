# N4 copied geometry and material foundation

391 source/header/license files copied from the historical checkout
`D:/dev/omi_code/studio_engine` at commit
`d6f457e8415445d00eaf7e9fbf9e5047768ea637`. This path records provenance only;
it is not used by the build or runtime. `copy-manifest.json` records each source
and copied SHA-256. No copied source is modified.

## Selected dependencies

| Module | Purpose | Preserved notices |
| --- | --- | --- |
| tp_math_utils | Geometry3D, MeshKeyFrame transform composition, Material and its four concrete material variants, JSON helper headers | Module LICENSE: MIT plus the included Dan Sunday/softSurfer notice |
| lib_glm | Header-only vector/matrix operations required by the math API; complete header set | Module LICENSE: dual Happy Bunny/MIT text, per-file notices |
| lib_nanoflann | Header required by Geometry3D's normal/vertex utilities | BSD LICENSE and header notice |

The explicit CMake target compiles seven source files. `Material.cpp` directly
references all four material variants, so their implementations are retained
even though the preview only sets the OpenGL material's albedo. nanoflann is
required by the compiled Geometry3D translation unit; the first workflow does
not use adaptive normal generation or vertex welding. GLM's broad `ext.hpp`
include requires a broad header closure. No legacy build scripts, binaries,
configuration or assets are copied.

## Audit and adaptations

Reviewed dependency manifests and include closure, triangle indexing and winding,
triangle expansion, face normals, bounds, transform composition, material copy
ownership, and the parameter/collection integration. The copied material copy
constructor clones its concrete material objects; tests check upstream snapshots
remain unchanged after later material edits. This is a focused migration audit,
not a comprehensive security or correctness certification of unused APIs.

`Geometry3D::transform` applies rotation alone to normals. The extension therefore
recomputes face normals after nonuniform scale. `MeshKeyFrame` supplies the copied
translation/rotation/scale matrix implementation; graph parameters are validated
explicitly before calling it. Imported geometry and legacy JSON loading are not
exposed. New primitive construction supplies bounded, valid triangles.

The high-level `ntn_scene_3d` transform/material operations require object
containers, timeline/keyframes and asset fetching. The legacy scene-map renderer
adds a larger graphics/application dependency chain. Neither is included in this
checkpoint. New extension glue uses the lower-level audited math implementation
and a small Qt painter preview, with the rendering limits described in the
extension README. The old checkout is unchanged.
