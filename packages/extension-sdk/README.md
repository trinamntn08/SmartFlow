# Extension SDK

Status: provisional TypeScript contracts and W2a bundled extension registration;
execution remains pending. The active native source-level contribution SDK is
`native/sdk/WorkspaceExtension.h`, used by scene and data extensions.

Own public contracts for domain types, nodes, parameter controls, viewers, package compatibility, and execution capabilities. Depend only on the core; do not import concrete extensions.

Exports provisional extension, type, port, parameter, node, viewer identity,
migration and execution capability contracts, immutable `ExtensionRegistry` and
`validateParameter`. Registry lookup uses persisted package/type/version IDs and
checks typed port compatibility. Viewer mounting and execution remain pending.
The package typechecks without DOM or React types.
