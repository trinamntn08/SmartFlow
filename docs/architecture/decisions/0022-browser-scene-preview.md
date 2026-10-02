# 0022: Bounded browser primitive scene preview

Date: 2026-10-02. Status: accepted for W4.

The bundled scene extension owns cloneable geometry, material values, type checks,
and Cube/Transform/Material/Scene/Merge execution. Runtime imports only public
contracts. Match native persisted IDs, parameter bounds, transform order and the
64-object/100000-coordinate limits. Shared fixtures compare native and browser
coordinates with a tolerance for native float arithmetic.

Use a small Canvas2D orthographic painter adapter, following the native preview
scope from [decision 0006](0006-native-scene-extension.md). No new renderer
dependency is necessary for this primitive slice. Fixed lighting, back-face
culling and depth-sorted triangles provide camera orbit/zoom/framing and object
selection. This is not a depth-buffered renderer; intersecting surfaces, textures,
transparency, imported assets and large scenes remain outside preview scope.

Geometry work executes in the worker. Bounded projection/painting runs in the UI.
Camera and selection update the browser workspace only. The explicit Increase
source size action routes through the editor's project parameter command and undo.
It reports an unsupported source for a source outside the active graph.

Portable native yaw/pitch/span/target/selection fields initialize a missing browser
scene viewer; the adapter bounds-checks them before rendering. Only those named
fields are mapped; native payloads and unknown fields remain retained unchanged.
Results remain transient and are regenerated after reopening.
