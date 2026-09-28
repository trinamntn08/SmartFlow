# Extension SDK

Status: initial TypeScript contracts; no registration or execution implementation yet.

Own public contracts for domain types, nodes, parameter controls, viewers, package compatibility, and execution capabilities. Depend only on the core; do not import concrete extensions.

Exports provisional extension, type, port, parameter, node, viewer identity,
migration and execution capability contracts. Viewer mounting and registry behavior
remain to be implemented. The package typechecks without DOM or React types.
