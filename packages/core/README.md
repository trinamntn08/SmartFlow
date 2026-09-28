# Core

Status: initial versioned document and persistence implementation.

Own the serializable project and graph model. Keep this package independent of UI frameworks, browser APIs, concrete extensions, and scene types.

Exports JSON document types, `createProject`, `parseProject`, and `serializeProject`.
Loading preserves unavailable extensions and unresolved connections. Run `npm test`
from the root for persistence behavior tests. Commands and semantic graph validation
are the next checkpoint. See [the contract decision](../../docs/architecture/decisions/0002-project-contract.md).
