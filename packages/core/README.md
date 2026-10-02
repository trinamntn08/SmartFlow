# Core

Status: preserved TypeScript schema-v1 document/persistence prototype; native
C++/Qt is the active application implementation.

Own the serializable project and graph model. Keep this package independent of UI frameworks, browser APIs, concrete extensions, and scene types.

Exports JSON document types, `createProject`, `parseProject`, and `serializeProject`.
Loading preserves unavailable extensions and unresolved connections. Run `npm test`
from the root for nine persistence behavior tests. TypeScript project commands
and semantic graph validation remain deferred, not the next native checkpoint.
Native commands/validation live in `native/app/project` and `native/app/pipeline`. See [the contract decision](../../docs/architecture/decisions/0002-project-contract.md).
