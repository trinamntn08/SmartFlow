# Checkpoint N5f: editor file actions

## Objective and scope

Continue N5e with Open/Save/Save As and dirty tracking. Workspace persistence
and the independent non-3D extension remain separate unfinished N5 steps.

## Current state

- WorkspaceWindow exposes File actions and standard shortcuts, a filename/dirty
  title, and Save/Discard/Cancel prompts before replacing or closing dirty content.
- Dialog-free openProject/saveProject methods use ProjectFile atomic I/O. Failed
  reads, empty-graph files and failed writes preserve document, path and baseline.
- Dirty state compares retained JSON with the last successful save/open snapshot,
  covering semantic undo/redo and retained workspace edits without tracking results.
  The initial preset is clean and untitled. Live canvas/viewer changes are not yet
  persisted and are explicitly described as such in the interface.
- Open selects the first graph, retains all other graphs and opaque content,
  clears history, selection and pinned output, resets canvas identity/layout caches
  and navigation, and uses existing revision invalidation for execution.
- Workspace tests exercise inspector edit/save/reopen and recomputation, dirty
  undo/redo, failure isolation, and unknown-content Save As preservation.

## Verification

- Release CMake build passed. Used the documented direct cmake_autogen workaround
  for workspace, workspace tests and scene tests after nested moc spawn failures.
- CTest: all six suites passed with process permission. The initial restricted
  run failed in temporary-file operations in three suites; rerun passed unchanged.
- `npm.cmd run check`: formatting, TypeScript, all nine persistence tests and
  production build passed with process permission. `git diff --check` passed.
- Offscreen startup and screenshot passed with the optional Segoe UI smoke font;
  inspected build/native/n5f-smoke.png, including the File menu and persistence hint.
- Native file dialogs and Save/Discard/Cancel interaction were not manually tested
  on a real desktop. No GPU interaction verification is claimed.

## Decisions and open questions

No package boundary or schema change. Dirty tracking uses full JSON comparison
for correctness; large-document performance is unmeasured. Files without graphs
are valid codec content but cannot currently open in the editor. Multi-graph
selection UI remains future work. Camera state is still transient.

## Next steps

1. Persist canvas positions, selection/navigation, pinned output and viewer camera
   through explicit workspace hooks, preserving unknown workspace fields.
2. Test scene edit/save/reopen with restored viewer state and failure isolation.
3. Add the separate non-3D extension workflow; numeric fixtures do not satisfy it.
4. Complete full N5 only after its acceptance checks. Continue to commit verified
   checkpoints separately. No delegation or old-repository changes are authorized.
