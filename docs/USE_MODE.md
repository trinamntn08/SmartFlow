# Native Build and Use modes

Use mode presents a prepared graph as an interactive tool. Choose **Use** in the
main toolbar to hide the graph, node library and Build panel controls. Choose
**Build** to return to the same canvas, selection and panel layout.

## Try the included tools

After building and installing the desktop test folder:

```powershell
build/desktop-test/bin/smartflow.exe --project "$PWD/build/desktop-test/examples/scene-tool.smartflow"
build/desktop-test/bin/smartflow-data.exe --use --project "$PWD/build/desktop-test/examples/components.smartflow"
```

The scene example opens in Use mode with size, position X, rotation Y and RGB
material controls. Change a value and click **Apply**. The data example exposes
the filter's minimum value; choose **rows** or **summary** in the Output selector.
Minimum 30 gives count 2, total 79 and mean 39.5. Minimum 20 gives count 3,
total 104 and mean approximately 34.667.

**Undo/Redo**, **Run**, **Cancel**, **Live updates**, and the execution scheduling
controls operate the same project and executor in either mode. With Live updates
off, Apply invalidates old results and Run computes the new ones. Diagnostics
remain visible if a component, package or input is unavailable.

## Prepare a tool

1. In Build mode, select the operations that make up the tool.
2. Use **Components > Create from selection** to expose named controls and outputs.
3. Insert the component from the library with **Insert as one node** checked,
   binding any required inputs to sources in the graph.
4. Switch to Use and select the inserted component in the Tool selector.
5. Save As to a new project file to retain parameter values and the chosen mode.

Use mode lists collapsed component instances in the active graph. Ordinary nodes
and component body parameters are edited in Build. Graphs without a collapsed
component show guidance for preparing one. Multiple instances can be selected
individually; execution still runs the entire graph and reports errors anywhere
in it. Library definitions do not need to remain present for snapshot tools to run.

## State and scope

Control edits use existing project commands and semantic history. Switching modes,
choosing a tool/output and navigating its viewer change workspace state. Native
Use state is saved per graph in `workspace["smartflow.native-use@1"]`, independently
of Build layout, selection, pinned output and viewer state. Unknown workspace
fields and opaque viewer data are retained.

The configured extension supplies an independent result viewer for Use mode;
scene and data tools use the same domain-independent workspace implementation.
The first version supports the existing numeric parameter editors and one selected
tool/output viewer at a time. Unsupported control types retain their values and
show an unavailable-editor message. Use mode is a simplified interface within
SmartFlow, not a separate exported executable or access-control boundary. Browser
Use mode, custom forms, file inputs and tool-specific automatic run policies are
future work.
