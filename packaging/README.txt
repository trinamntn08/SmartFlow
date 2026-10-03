SmartFlow - first Windows desktop test version

Launch bin/smartflow.exe for the 3D scene workflow.
Launch bin/smartflow-data.exe for the independent table workflow.
Keep this whole folder together; its Qt and compiler runtimes are included.
Node.js, a browser server and the old source checkout are not needed.

Each region has a widget selector: choose Graph, Node library, Inspector,
Execution results or Result viewer. H splits left/right, V splits top/bottom,
and X closes a region. Drag dividers to resize. Choosing an already visible
widget swaps its region. Layout > Reset widget layout restores defaults.
The arrangement and divider sizes are saved with your project.

Open the matching examples with File > Open:
  examples/scene.smartflow       - use smartflow.exe
  examples/data.smartflow        - use smartflow-data.exe
  examples/components.smartflow  - use smartflow-data.exe
  examples/scene-interaction-tool.smartflow - use smartflow.exe (Use mode)

In the scene viewer choose Orbit/Front/Right/Top. Middle or Shift-drag pans;
wheel zooms at the cursor. Select an object, choose Move/Rotate Y/Scale and drag.
Move has X/Y/Z controls and colored axis handles. Ctrl snaps; Escape cancels.
Release commits one undoable graph edit. A shared Transform edits all meshes
it drives. Use-mode edits require exposed component controls.
See docs/SCENE_INTERACTION.md for supported behavior and module boundaries.

You can also edit numeric fields directly inside graph nodes: type and Enter
or leave the field; Up/Down steps apply immediately. Escape cancels unfinished
typing. Inline edits share Undo/Redo and execution with the inspector and viewers.
See docs/INLINE_NODE_CONTROLS.md.
Each numeric row has a slider: drag to change the graph and viewer live.
The drag is one undoable edit. Escape restores the start. Precise typing remains.
Scroll over a focused field or slider for proportional adjustments; Shift is finer.
Selecting a node also provides its inspector with an Apply button.
Run, Undo and Redo should update the results. Save As to a separate file, close the app,
relaunch and Open it to check restoration of the graph, layout and pinned viewer.

Components > Component library supports import/export, removal, and Edit a copy.
Save copy adds a new definition; existing instances keep their snapshots.
Select one collapsed instance and use Components > Update selected instance
to explicitly adopt a compatible definition. Review both bodies and Apply;
current controls and connections remain. Undo restores the previous snapshot.

The full procedure and limitations are in docs/DESKTOP_TESTING.md.
Automated Windows startup checks are recorded in verification/startup-results.json
when scripts/Test-DesktopPackage.ps1 has been run from the source repository.
Manual edit/undo/save/relaunch/Open acceptance is still awaiting user results.

This is a local development preview. The 3D viewer displays bounded primitives;
asset import, nested components, automatic migrations and a public installer
remain future work.
