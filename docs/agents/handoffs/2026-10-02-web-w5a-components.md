# W5a: Component contracts and collapsed execution

## Objective and scope

Implement the existing native component contract in the browser without changing
schema-v1 or interpreting unsupported content during project loading.

## Current state

Core validates and transports standalone component definitions, preserving opaque
fields. It checks nonempty DAG bodies, interface identities/bindings, internal
producers, saved controls and extraction boundaries. Nested definitions reject
explicit use while staying retained in loaded projects.

Runtime resolves collapsed snapshots through SDK contracts and executes their
private bodies after all exposed inputs arrive. Named outputs publish together
after the whole body completes; failures/cancellation discard the run. Controls
override private body parameters, not retained snapshots. Editor ports and numeric
controls now resolve component instances. Catalog removal does not affect execution.

## Verification

`npm.cmd run check` passed: formatting, all workspace type checks, 66 Node tests
and production editor/worker build. Chromium component interaction passed named
output execution, control edits and undo. Core/app tests cover standalone opaque
retention, duplicate transport keys, malformed/nested/cyclic bindings, extraction
boundaries, absent catalog execution, unsupported body contracts and atomic failure.
Relevant existing native component CTest checks validate the same shipped contract
and standalone example. Native sources and persisted format are unchanged.

## Decisions and open questions

Use the existing [component contract](../../GRAPH_COMPONENTS.md) and native
[snapshot semantics](../../architecture/decisions/0013-collapsed-native-components.md).
The preview uses deterministic worker execution, not native scheduling parity or
a production plugin system. Standalone UI import/export and authoring follow W5b.

## Next steps

W5b extraction/library/import/export/edit-as-copy/compatible updates and saved
browser layout. W6 production acceptance and native/browser/native file exchange.
