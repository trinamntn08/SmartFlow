# 0002: Initial project document and extension contracts

Date: 2026-09-27. Status: accepted for the first implementation checkpoint.

## Decision

Use a JSON envelope with `format: smartflow` and `schemaVersion: 1`. Store graphs,
package dependencies and asset references under `project`; store positions,
selection and viewer configuration under `workspace`. Execution results stay
outside this document. Graph, node, connection and asset IDs are explicit and
unique within their respective collections. Node identity includes package ID,
type ID and a positive integer contract version. Package versions are recorded
as nonempty strings; compatibility resolution is deferred.

Loading validates known structural fields and retains all unknown JSON fields.
It does not need an extension registry. Missing endpoints and unavailable node
types survive loading; the future runtime must diagnose them before execution.
Unsupported envelope versions are rejected explicitly, without rewriting files.
Preservation means JSON values, not original whitespace or numeric spelling.

Core exposes parse, serialize and empty-project functions. Serialization refuses
non-JSON values rather than silently dropping them. The SDK defines type, port,
parameter, node, viewer identity, migration and capability interfaces. Viewer
adapter IDs are placeholders for application adapters, not an implemented viewer
lifecycle. Migration is a declared contract, not automatic migration support.

## Legacy provenance

Inspected `D:/dev/omi_code/studio_engine` at the source revision recorded in
[the reuse analysis](../legacy-reuse-analysis.md). `tp_pipeline` headers
`StepDetails.h` and `PipelineDetails.h` supplied conceptual references for node
IDs, parameters and port links. This checkpoint contains independently written
TypeScript, no copied C++ source or assets. The old repository is read-only.
Direct copying would pull native engine dependencies into the browser path.

## Consequences and next checkpoint

The first contracts are deliberately provisional and have no renderer or UI
dependency. Both scene and table workflows can use them. The next checkpoint
adds explicit project commands, undo/redo, connection diagnostics and the DAG
runtime, including cancellation and stale-result tests. Rendering and canvas
library choices remain open.
