# Extension SDK

Status: preserved provisional TypeScript contracts; no TypeScript registration
or execution implementation. The active native source-level contribution SDK is
`native/sdk/WorkspaceExtension.h`, used by scene and data extensions.

Own public contracts for domain types, nodes, parameter controls, viewers, package compatibility, and execution capabilities. Depend only on the core; do not import concrete extensions.

Exports provisional extension, type, port, parameter, node, viewer identity,
migration and execution capability contracts. Viewer mounting and registry behavior
remain to be implemented. The package typechecks without DOM or React types.
