# Data extension

Status: initial native implementation, checkpoint N5h.

The independent non-3D validation package implements Sample table ? Filter rows
? Summary, with a read-only two-column table viewer. It uses the same native
StepDelegate, member factory, node registration and OutputViewer contracts as
the bundled scene extension. No scene model or scene registry is imported.

Run the packaged `smartflow.exe --data` or `smartflow-data.exe`. The latter
links only the workspace and data extension, excluding scene and scene-math
libraries. The primary application still includes the required 3D workflow.

The six sample labels have values 12, 25, 7, 48, 31 and 19. Sample table supplies
a multiplier; Filter rows keeps values greater than or equal to its minimum.
The default minimum 20 yields count 3, total 104 and mean 34.66666667. Minimum 30
yields count 2, total 79 and mean 39.5. Empty input yields count, total and mean 0.
The sample is a bundled fixture, not an external file import.

Select Filter rows, edit minimum and Apply. Parameter edits and undo/recompute
use platform commands. Pin a source/filter node to inspect its rows. Viewer
row selection is saved workspace state, without graph edits or execution.
Node identities are `smartflow.data/{sample,filter,summary}` at version 1;
output type is `smartflow.data.table@1`. Computed rows are never project assets.

This bounded validation slice has one label and one numeric value per row,
at most 1000 input rows, and finite values within 1,000,000. It does not provide
CSV import, arbitrary column schemas, sorting, editable tables or a plugin loader.
Tests live in `native/tests/DataTests.cpp`, which does not link scene libraries.
No TypeScript package/runtime is introduced; browser integration remains deferred.
