# 0007: Native project-file foundation

Status: accepted for N5a; extended by N5b/N5c, 2026-09-30.

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

N5b adds `DocumentSession`, retaining the entire file while compiling one selected
graph into an execution snapshot. Native node registrations now explicitly carry
package ID, type ID and contract version. Matching uses that tuple, never labels
or parsing a combined delegate name. Ambiguous/incomplete registrations reject.

The adapter reads parameter metadata from a fresh disposable delegate prototype,
then decodes only supplied parameters. It never injects defaults into the loaded
document or runs fixup on a loaded node. Initial parameter codecs handle bounded
double, string and bool values. Unknown/missing/invalid parameters, missing node
types or versions, unresolved endpoints, unsupported ports, duplicate input
producers and mismatched port types generate diagnostics. A graph with these
diagnostics cannot be submitted through `executableGraph`; no partial executable
graph is exposed. Cycles and remaining runtime rules are still checked by N2.

Explicit parameter edits build a replacement document/session first and swap on
success. Unedited node/edge fields, graphs, assets and workspace values remain
intact. Stable node IDs come from the file; output collection IDs are transient
and regenerated because persisted connections use node IDs and stable port names.
The legacy step loader receives only a generated ID/delegate envelope, never raw
file parameters, mappings or binary content.

Package dependency versions are retained but not resolved by N5b. Exact node
contract versions gate execution. Unknown parameters conservatively block the
entire selected graph rather than guessing their semantics. Arbitrary extension
parameter codecs and partial-branch execution are not provided here.

N5a is a file-codec checkpoint, not editor save/open. It does not connect files
to GraphProject, draw missing nodes, resolve extensions, serialize viewers, or
provide application actions. The full N5 milestone remains incomplete.
N5b connects retained documents to execution and explicit parameter edits, but
does not yet integrate the session with the canvas or application file actions.

The next adapter must retain the source DOM, overlay only intentional edits, and
keep unavailable content intact. Validate the complete candidate before replacing
the active graph. Do not call legacy fixup/default initialization on loaded nodes
or turn the existing trusted undo snapshot loader into an untrusted file parser.
Explicit package/type/version metadata belongs in the contribution contracts;
do not infer persisted identities by splitting display labels.

## Structural commands (N5c)

The retained session now creates empty projects with a selected empty graph and
supports node creation/deletion and connection creation/deletion. Callers supply
stable IDs once so a later undo command can reuse them. New nodes alone receive
registered defaults, encoded through the same bounded double/string/bool codecs.
Unsupported or invalid defaults reject the entire command. A runtime type must
map to a unique persisted identity, so creation cannot guess between registrations.

Every semantic command prepares a complete candidate before replacing the source
and projection. Deleting a node deliberately removes all its incident edges,
including opaque ones; unrelated content remains exact. Disconnect identifies an
edge by stable ID and can remove unresolved edges. New connections require known
nodes, named ports, compatible types and a vacant input. An unresolved existing
edge still occupies its input and is never silently overwritten. Other diagnostics
may remain during editing; cycles are still checked by the execution adapter.

Workspace commands replace one explicitly named top-level field after validation,
preserving all other fields and the runtime projection. The caller owns that field
and must merge nested content when appropriate. Node creation does not invent
package dependency versions: contributions currently expose node contract versions,
not resolved package versions. Package dependency management remains future work.
These commands do not yet provide Qt undo commands or connect the demo canvas.

## Retained-document undo (N5d)

`DocumentHistory` owns a session and a Qt undo stack. Commands store complete
semantic JSON snapshots using the retained DOM, including unknown fields and
64-bit integers. They never serialize through Qt JSON or legacy step snapshots.
Deletion undo therefore restores all incident edges, including unavailable ones,
and disconnect undo restores the original edge identity and opaque metadata.
Commands validate a candidate before pushing to the stack; failed edits and
no-ops preserve the redo branch, clean marker, revision and active projection.

Semantic snapshots exclude `workspace`. Undo/redo restores the semantic document
with the current workspace, recompiles its projection, increments a monotonic
revision and emits `changed`. Workspace edits use a separate signal and leave
the semantic revision and projection intact. The undo stack's clean marker covers
semantic edits only; future file dirty tracking must account for workspace edits.
Replacing a document prepares the candidate first, then clears old history and
increments the revision. Registrations must remain immutable, as for the session.

Whole-document snapshots are a correctness-first implementation; history memory
limits and compaction remain future work. Canvas integration must replace or
adapt the vendor commands that retain only connection endpoints. The demo editor
does not yet use this command layer, and no UI Save/Open behavior is claimed.
