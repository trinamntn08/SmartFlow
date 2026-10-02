# Core

Status: preserved TypeScript schema-v1 document/persistence prototype; native
C++/Qt is the active application implementation.

Own the serializable project and graph model. Keep this package independent of UI frameworks, browser APIs, concrete extensions, and scene types.

Exports JSON document types, `createProject`, `parseProject`, and `serializeProject`.
Loading preserves unavailable extensions and unresolved connections. Strict
transport rejects duplicate keys, invalid Unicode, unsafe integer-valued numbers,
negative zero and native-aligned byte/depth limit violations before returning a
document. See [numeric policy](../../docs/architecture/decisions/0019-browser-json-transport.md).
The web adapter owns UTF-8 byte decoding; core has no browser API dependency.
W2a adds `ProjectHistory`, retained add/delete/connect/disconnect/parameter commands,
transactional replacement and workspace-independent undo/redo. Run `npm test` from
the root for persistence, command and SDK tests. Execution validation remains deferred.
Native commands/validation live in `native/app/project` and `native/app/pipeline`. See [the contract decision](../../docs/architecture/decisions/0002-project-contract.md).
