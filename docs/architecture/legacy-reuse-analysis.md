# Legacy repository reuse analysis

Date: 2026-09-27. Status: initial source audit and recommendation, not an implementation decision.

Superseded implementation recommendation: the user selected native C++/Qt reuse.
See [decision 0003](decisions/0003-native-first.md). This audit remains historical
dependency evidence; its recommendation to exclude Qt and port to TypeScript
does not govern current work. See [current architecture](README.md) and
[the roadmap](../ROADMAP.md) for delivered native functionality.

## Scope and conclusion

The presumed source repository is `D:/dev/omi_code/studio_engine`, inspected at commit `d6f457e8415445d00eaf7e9fbf9e5047768ea637`. Its working tree also contained an untracked `documentation/VisualWorkspaceProposal.md`; this assessment does not depend on that file. SmartFlow's working tree was clean before this analysis.

At the audit date, SmartFlow implemented only the React browser starter. Core, SDK, runtime, and extensions then contained ownership documents, not executable libraries.

Recommend selective reimplementation of useful behavior in TypeScript, with source references and focused tests. No complete legacy module is recommended for direct copying into the first browser slice. The legacy C++ pipeline foundation is useful design evidence, but it depends on other engine libraries. Qt widgets and the scene application introduce much larger dependency chains.

This audit reads selected headers, implementation sections, and dependency manifests. It does not certify all legacy behavior, build the engine, or establish compatibility with existing projects. No legacy code or assets have been copied.

## Source-to-destination map

All source paths below are relative to the legacy repository root.

| Priority                       | Legacy source                                                                                                                                                  | Useful behavior                                                                     | SmartFlow destination and treatment                                                                                                                                                                                                                                            |
| ------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| First                          | `tp_pipeline/inc/tp_pipeline/StepDetails.h`, `PipelineDetails.h`; `tp_pipeline/src/PipelineDetails.cpp`                                                        | Node identity, parameter values, port connections, duplication and ID remapping     | Reimplement a versioned graph document and project commands in `packages/core`. Use explicit source/target node and port IDs; keep visual positions and routing separate from execution semantics.                                                                             |
| First                          | `tp_pipeline/inc/tp_pipeline/StepDelegate.h`, `StepDelegateMap.h`, `Parameter.h`                                                                               | Named node types, typed ports, registration, descriptions, bounds and enum controls | Design minimal node/type/parameter contracts in `packages/extension-sdk`. Add package identity, contract versions, migration and execution capabilities. Keep parameter values in the project and control descriptions in node definitions.                                    |
| First                          | `tp_pipeline/src/PipelineManager.cpp`; `tp_pipeline/inc/tp_pipeline/StepContext.h`                                                                             | Resolving producers from connected inputs and running upstream dependencies         | Reimplement DAG validation and scheduling in `packages/runtime`. Reject cycles and invalid inputs explicitly; add cancellation and protection against publishing obsolete results. Do not import the native task queue.                                                        |
| First                          | `tp_pipeline/src/PipelineResultsManager.cpp`                                                                                                                   | Per-node results and profiling dependency records                                   | Implement minimal status, errors and inspectable outputs in `packages/runtime`. Defer detailed history and profiling. The source contains extensive commented-out code; do not treat it as implemented functionality.                                                          |
| First                          | `tp_qt_pipeline_widgets/src/widgets/PipelineNodesWidget.cpp`, `StepParametersEditorWidget.cpp`; `tp_qt_pipeline_widgets/src/StepDelegateNodeDelegateModel.cpp` | Canvas-to-model mapping, connection editing, selection and parameter inspection     | Rebuild interactions in `apps/web` against public project commands and SDK metadata. QtNodes identifiers and undo internals must not become the persisted graph format.                                                                                                        |
| First                          | `ntn_scene_builder/src/workspaces/PipelineWorkspace.cpp`, `ntn_scene_builder/src/PipelineContext.cpp`                                                          | Composition of graph, parameter editor, output inspection and node registration     | Use as workflow references for `apps/web`. Replace the concrete scene, renderer, Temporal and desktop settings composition with explicit bundled extension registration.                                                                                                       |
| First, narrowly                | `ntn_scene_3d/src/ApplyTransform.cpp`, `ApplyMaterial.cpp`; scene/object structures in `ntn_scene_3d`                                                          | Transform hierarchy and material assignment semantics                               | Implement a small independent model and primitive/transform/material/scene nodes in `extensions/scene-3d`. The old transform implementation handles parent matrices and keyframes; material application relies on asset fetching and handles. Neither is a drop-in basic node. |
| Later                          | `tp_pipeline/inc/tp_pipeline/step_delegates/PipelineStepDelegate.h`, `PipelineWrapper.h`, `PipelineLibrary.h`                                                  | A pipeline exposed as a node                                                        | Revisit for reusable graph components after the first workflows work. Replace filesystem library and native wrapper assumptions with explicit exposed inputs, outputs and parameters.                                                                                          |
| Reference only                 | `ntn_scene_3d/inc/ntn_scene_3d/UndoRedo.h`                                                                                                                     | Grouping related edits into one undo action                                         | Implement domain-independent project commands in `packages/core`; this legacy undo stack takes a `Scene*` and cannot be the platform undo model.                                                                                                                               |
| Defer pending a separate audit | `ntn_operations`, `ntn_scene_3d_gltf`, `ntn_preview_emcc`, `ntn_scene_3d_emcc`, `pipelines`                                                                    | External execution, import/export, browser preview behavior and example workflows   | Consider only for a demonstrated integration need. File presence and repository guidance are not proof of portable or standalone behavior. Existing pipeline files require format, node dependency and asset review before reuse.                                              |

## Behaviors to change during reimplementation

1. **Preserve unknown content.** `PipelineDetails::clearDanglingInputs()` clears unresolved input references. `StepDelegate::fixupParameters()` is documented to remove unnecessary parameters, and `StepDetails` exposes `setValidParameters()`. SmartFlow must retain unsupported node payloads and connections when loading, report diagnostics, and block affected execution. Repair must be an explicit edit.
2. **Do not inherit binary persistence.** `PipelineDetails::saveBinary()` writes a blob index and JSON containing `Steps`. Start with a versioned SmartFlow document and separate asset references. Legacy import is a separate feature, not implied by copying the model.
3. **Separate saved and transient state.** `StepDetails` combines parameters, node position, connection anchors, and runtime-only overrides. SmartFlow should explicitly separate project, workspace and execution state, even if project and workspace are saved in one envelope.
4. **Design invalidation deliberately.** `PipelineManager::calculateCacheKey()` uses the delegate name, input timestamps and stringified parameters. Do not transplant this into a versioned extension runtime. Begin with correct downstream invalidation; add caching with deterministic typed values, input revisions and node/package versions when needed.
5. **Validate graph errors explicitly.** The inspected dependency resolver skips missing providers and self references. SmartFlow should diagnose missing endpoints, incompatible ports and cycles before execution, rather than rely on incidental scheduler behavior.
6. **Keep viewing and editing distinct.** Navigation camera changes belong to workspace state. Object edits must target graph parameters through commands, with an explicit mapping from output objects to their source nodes.

## Leave out of the initial migration

- Qt widgets, QtNodes internals, docking framework and desktop settings.
- The full scene project model, account/catalog structures and asset database integration.
- Renderer, Blender, Temporal, AI services, authentication and remote backend setup.
- Native build/deployment files, `TP_CONFIG`, binary roots and machine-specific settings.
- Vendored `lib_*` trees and bulk copies of `tp_*` libraries.
- Existing private assets and pipeline fixtures whose dependencies have not been reviewed.

Evidence for the dependency cost: `tp_pipeline/dependencies.pri` includes `tp_data`, `tp_task_queue`, `tp_utils`, image utilities and base64; `tp_qt_pipeline_widgets/dependencies.pri` additionally pulls QtNodes, charts, operations, maps and application framework libraries. `ntn_scene_3d/dependencies.pri` includes math/image utilities, timeline and other native libraries.

The inspected `tp_pipeline/LICENSE` is MIT and requires retaining its notice for copies or substantial portions. This is not a license inventory for the entire old repository. Record provenance and the applicable module notice for any actual source port.

## Recommended implementation order

1. Specify the versioned project document and minimal SDK contracts, using the legacy node/port/parameter concepts as references. Define stable package and node IDs and opaque preservation of unavailable extensions.
2. Implement and test project commands, serialization, connection validation and a minimal DAG runtime. Cover invalid ports, missing packages, cycles, upstream failure, downstream invalidation and cancellation that cannot publish stale results.
3. Implement primitive → transform → material → scene output in the included 3D extension, with a viewer supplied through the SDK. Select rendering and canvas libraries separately; this analysis does not settle those choices.
4. Connect canvas and inspector edits to project commands. Verify a transform edit updates the preview, undo restores it, and save/reopen preserves the graph and workspace.
5. Implement sample table → filter → summary through `extensions/data`, and verify it executes without importing or initializing the 3D extension.
6. Add reusable graph components; consider a legacy importer or native integration only against a concrete example that users need.

At the audit date, the next bounded implementation task was the document/SDK contract and its persistence tests. Copying the entire old pipeline or scene module first would bring dependencies that the standalone browser architecture does not need.
