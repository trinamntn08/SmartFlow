# 0015: Component editing and explicit update scope

Date: 2026-10-02
Status: proposed; documentation scope only, no implementation delivered.

## Context

The N6f library supports creation, insertion, exchange and removal, but cannot
edit definitions. Collapsed instances execute complete saved snapshots. Changing
a library entry must not silently change a saved graph's behavior.

The current `GraphComponent` constructor accepts only `schemaVersion: 1` and
`version: 1`. Catalog insertion compares ID/version and rejects different content
under that pair. Incrementing `version` for an ordinary edit would currently make
the definition unsupported. These fields cannot serve as editable revision
counters without a separate compatibility design.

## Proposed scope

### First checkpoint: edit a copy

Start with a library action that opens a supported definition in an isolated
editing draft. Saving creates a new definition with a fresh ID and both existing
version fields set to 1. The source entry and all instance snapshots remain
unchanged. The author can choose a new title and edit body nodes, connections,
defaults and exposed interfaces. Preserve opaque fields on retained objects;
explicit removal of an object removes that object's fields with it.

Use the ordinary graph command/validation contracts for the draft, rather than
mutating the main project while editing. Save validates the complete definition
and adds it to the catalog through one semantic history command. Cancel discards
the draft. Guard publication against a stale destination document revision.
Unavailable or malformed definitions stay retained but cannot open in this first
editor. Export/removal remain available under their existing rules.

This phase needs no persisted lineage or revision fields. The new ID identifies
an immutable saved definition. Repeated import of identical content remains a
no-op and conflicting content under an existing ID/version still rejects.

### Later checkpoint: explicit instance replacement

Only after edit-a-copy is verified, add an explicit action for one selected
collapsed instance to adopt a chosen supported definition. Keep its visible node
ID and replace its snapshot in one undoable command. Never update on Open,
catalog import, library editing or execution. Expanded copies have no retained
instance boundary and are outside this action's scope.

The first replacement command should require the same exposed input, output and
control IDs. Validate registered port types, connected edges, control parameter
types/bounds, required inputs and the expanded acyclic graph before publication.
Carry existing control values by exposed ID; an incompatible value rejects the
operation instead of resetting it. Body endpoints may change if the public
interface and current graph remain valid. Preserve unrelated node metadata,
unknown project content, other graphs and workspace/viewer selections.

Present the selected source/target definitions and changed defaults/body before
the user applies the replacement. A rejected or cancelled operation must retain
the document, undo/redo branch and results. A successful replacement invalidates
old execution results through existing revision handling. Undo restores the exact
old snapshot and control values, retaining current workspace state.

Interface additions/removals, remapping, bulk updates, automatic migrations,
nested components and exposed input fan-out remain outside these checkpoints.
An immutable-ID scheme can support explicit replacement without claiming a
general revision or package-migration system.

## Alternatives

Editing content under its current ID/version breaks the existing immutable
catalog contract and creates ambiguous imports. Using `version: 2` as an edit
counter changes the current supported contract. Adding a revision/lineage field
needs a separate catalog identity and interoperability policy. Automatic updates
undermine snapshot reproduction. Defer these alternatives until a concrete need
justifies a new file-contract decision.

## Consequences and verification

This proposal establishes implementation boundaries, not new executable behavior.
Existing schema-v1 files, exports and snapshots need no migration for edit-a-copy.

For the editing checkpoint, verify independent data and required scene definitions,
opaque-field retention, interface/body validation, cancellation, stale drafts,
one-command catalog undo/redo, import conflicts and save/reopen/export. Existing
instances must execute identically after saving the edited copy.

For explicit replacement, additionally verify connected instances, multiple
outputs, preserved overrides and workspace aliases, incompatible interfaces and
values, newly unavailable registrations, cycles, stale revisions, exact undo/redo
and save/reopen execution. Confirm other instances retain their original snapshots.
Run native build/CTest, offscreen startup and web checks for each implementation
checkpoint, then inspect the native dialogs and record packaged desktop results
separately. Full N5 acceptance remains pending manual testing.
