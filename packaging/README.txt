SmartFlow - first Windows desktop test version

Launch bin/smartflow.exe for the 3D scene workflow.
Launch bin/smartflow-data.exe for the independent table workflow.
Keep this whole folder together; its Qt and compiler runtimes are included.
Node.js, a browser server and the old source checkout are not needed.

Open the matching examples with File > Open:
  examples/scene.smartflow       - use smartflow.exe
  examples/data.smartflow        - use smartflow-data.exe
  examples/components.smartflow  - use smartflow-data.exe

Try selecting a node, editing its inspector value and applying it. Run, Undo
and Redo should update the results. Save As to a separate file, close the app,
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
