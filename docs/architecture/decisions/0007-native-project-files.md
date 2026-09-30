# 0007: Native project-file foundation

Status: accepted for N5a, the first part of N5, 2026-09-30.

## Decision

Reuse the schema-v1 envelope from [decision 0002](0002-project-contract.md), rather
than making QtNodes snapshots or legacy StepDetails binary JSON into a file format.
`native/app/project/ProjectFile` implements registry-independent create, validate,
parse, serialize, read and atomic write operations. It links Qt Core and the
already-audited nlohmann JSON headers; no widget, pipeline or domain library is
required by this target.

Keep the complete JSON DOM, validating only known structural fields. Unknown
extension/node/parameter/connection fields, package metadata, assets, workspace
configuration and unresolved endpoints survive round trips. Loading does not
repair graphs or imply that they can execute. Persisted project and workspace
state remain distinct; the file layer does not manufacture execution results.

Native writes serialize and validate before opening a QSaveFile, then commit its
temporary file. Direct-write fallback is disabled. Invalid documents and failed
replacement must not truncate an existing project. The future editor must handle
reported failures without discarding the active project or its dirty state.

Native parsing rejects duplicate JSON keys, unsupported envelope versions,
duplicate identities, invalid known fields, nonfinite numbers, integer tokens
outside signed/unsigned 64-bit range, files above 16 MiB and nesting beyond 128
containers. These are explicit native input limits. Floating-point tokens use
finite IEEE doubles. Unknown native 64-bit integers round-trip exactly; the
TypeScript prototype uses JavaScript numbers and cannot guarantee that precision
outside its safe integer range. Shared fixtures test the interoperable subset.

## Current boundary

N5a is a file-codec checkpoint, not editor save/open. It does not connect files
to GraphProject, draw missing nodes, resolve extensions, serialize viewers, or
provide application actions. The full N5 milestone remains incomplete.

The next adapter must retain the source DOM, overlay only intentional edits, and
keep unavailable content intact. Validate the complete candidate before replacing
the active graph. Do not call legacy fixup/default initialization on loaded nodes
or turn the existing trusted undo snapshot loader into an untrusted file parser.
Explicit package/type/version metadata belongs in the contribution contracts;
do not infer persisted identities by splitting display labels.
