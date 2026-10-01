# Checkpoint N5e: retained-document editor integration

## Objective and scope

Continue N5d (`4bf3436`) by making the running editor use retained documents and
their undo commands. N5e completes model/canvas/inspector integration and missing
node display. Save/Open actions, workspace persistence and the independent
non-3D extension remain pending; full N5 is incomplete.

## Current state

- GraphProject owns DocumentHistory. Node/connection/parameter edits operate on
  retained JSON, and semantic undo restores opaque metadata and exact identities.
  Legacy binary node capture/restore and ProjectCommands.h were removed.
- DocumentSession supplies detached inspector copies even when diagnostics block
  execution. Mutating an inspection copy cannot change retained data or execution.
  GraphProject refreshes its inspection cache after every semantic change.
- WorkspaceScene routes toolbar/context creation, Delete action, connection and
  disconnection through document history. Multi-selection deletion is one atomic
  candidate edit. Inspector Apply pushes only the document command. Initial preset
  setup clears history. Existing movement commands share the history stack without
  changing the semantic revision or invalidating results.
- PipelineCanvas synchronizes incrementally, suppressing edit callbacks. Stable
  canvas IDs and positions survive undo; generated display envelopes alone reach
  QtNodes. Unknown types display zero-port package/type placeholders. Unresolved
  edges remain retained; hidden edges still occupy inputs. Visible-edge deletion
  uses its exact retained edge ID, even beside an unresolved duplicate producer.
- ExecutionController publishes retained diagnostics without submitting unsupported
  graphs. Replacement and semantic undo continue to invalidate pending results
  through the existing revision/generation checks.
- A small isolated vendor adaptation adds virtual scene command-routing methods
  and an undo-stack accessor. Default canvas behavior is preserved; semantics live
  in WorkspaceScene. Original licenses/provenance remain unchanged. See
  [patch notes](../../../native/vendor/qtnodes/SMARTFLOW_PATCHES.md).
- Workspace tests now include opaque node/edge restoration through the Delete
  action, editor-model disk round trips, hidden-edge occupancy, inspection-copy
  isolation and replacement during execution. Existing scene/workspace tests now
  exercise the retained command path instead of mutating legacy projections.

## Verification

- `cmake --build build/native --config Release --parallel 8`: passed. Restricted
  nested moc launches required the documented direct `cmake -E cmake_autogen`
  workaround for changed QtNodes, document-history, workspace and test targets.
  An initial compile error accessing QtNodes' private override of `newNodeId` was
  corrected by calling its public AbstractGraphModel interface.
- `ctest --test-dir build/native -C Release --output-on-failure`: all six suites
  passed. After final placeholder placement/inspector wrapping, rebuilt and reran
  the affected `native_workspace|native_scene` suites successfully. Workspace has
  eight behavior cases plus setup/cleanup.
- `npm.cmd run check`: formatting, typechecks, nine persistence tests and build
  passed with process permission. The first restricted test launch hit `spawn
EPERM`; no source changes were made to bypass it.
- Offscreen `smartflow.exe --smoke-test -platform offscreen --smoke-font
C:/Windows/Fonts/segoeui.ttf --screenshot build/native/n5e-smoke.png`: passed.
  Inspected the scene screenshot and the workspace test's
  `build/native/native/n5e-unavailable-smoke.png` placeholder render.
  The latter initially had missing offscreen font glyphs; reran the workspace
  suite with `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf` using its optional
  font setup. No machine-specific font path is bundled in the test.
- `git diff --check`: passed. No real desktop/GPU interaction or file-dialog
  verification is claimed; application Save/Open dialogs do not exist yet.

## Decisions and open questions

See [decision 0008](../../architecture/decisions/0008-retained-editor-commands.md).
Inspector pointers expire at semantic edits. Canvas identity/position caches are
transient and currently survive model-level replacement; Open must apply the
loaded workspace state explicitly. Whole-document history and inspection copies
remain correctness-first, without large-graph performance guarantees.

## Next steps

1. Add Open/Save/Save As and file dirty tracking. Use ProjectFile atomic writes;
   failed reads/writes must leave active document, path and dirty state intact.
2. Add explicit canvas/viewer workspace hooks for positions, selection, pinned
   outputs and camera. Keep unknown workspace content, semantic undo and transient
   execution results separate. Reset/restore transient canvas caches on Open.
3. Test actual UI edit/save/reopen, workspace round trips, failures and unavailable
   extension content. Preserve the current generation/revision invalidation.
4. Add the independent non-3D extension through the same contracts; the numeric
   fixture still does not satisfy that acceptance requirement.
5. Build/CTest/npm checks, inspect UI renders, document and commit each completed
   step. Full N5 remains incomplete until all acceptance checks pass.

Environment remains PowerShell, Qt 6.11.1 MSVC2022, `build/native`, Node 24/npm 11.
Use `npm.cmd`; Qt bin is `C:/Qt/6.11.1/msvc2022_64/bin`. No agent delegation is
authorized. No old repository changes, new dependency downloads or package loader.
