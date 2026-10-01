# SmartFlow local canvas changes

Original source and license provenance is recorded in [the vendor audit](../README.md).
The original copy manifest remains unchanged.

- N1: `inc/QtNodes/Globals.h` adapts export macros for the independent CMake build.
- N5e: `BasicGraphicsScene.hpp/.cpp` adds virtual `createNode`, `deleteSelected`,
  `connectNodes`, `disconnectNodes` and makes `undoStack` virtual. Defaults invoke
  the existing canvas commands. `DataFlowGraphicsScene.cpp`, `GraphicsView.cpp`
  and `NodeConnectionInteraction.cpp` route their corresponding user actions
  through those methods. Movement commands use the virtual undo-stack accessor.
  All document semantics live in the application-side WorkspaceScene adapter.

This is an application integration hook, not a change to the canvas file format,
node model, command implementations or license. Git records the isolated diff.
See [decision 0008](../../../docs/architecture/decisions/0008-retained-editor-commands.md).
