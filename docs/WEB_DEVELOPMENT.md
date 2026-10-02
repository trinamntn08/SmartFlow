# Browser workspace development

Updated: 2026-10-02. W3b data execution and table outputs are implemented alongside
the native Windows app. W4 scene execution/rendering and W5 component authoring are implemented.

## Run and edit

Use Node.js 24/npm 11 and repository-local dependencies:

```powershell
npm.cmd ci
npm.cmd run dev
```

Open http://127.0.0.1:5173. The initial data example opens without execution.
Use the library to add nodes; connect named outputs to compatible inputs. Select
a node and edit its parameters in Inspector, then press Enter or leave the field
to apply. Delete selection removes nodes and incident edges in one command.
Use Undo/Redo or Ctrl+Z/Ctrl+Shift+Z (Ctrl+Y also works). Input fields retain normal
text-editing shortcuts. Dragging/navigation changes workspace rather than semantic
history. Examples include a scene and a collapsed reusable component workflow.

Run graph executes in a worker. Cancel run terminates work and discards partial
outputs. Node states appear on the canvas. Choose a named output in the Output
panel; table row selection and the pinned output are saved with browser workspace.
Parameter/graph edits mark previous results obsolete until another run completes.

The scene example runs Cube/Transform/Material/Scene; Merge accepts two scenes.
Drag the 3D preview to orbit, wheel to zoom, and use Frame scene to center it.
Click an object to select it. Increase source size is an explicit undoable cube
parameter edit. Camera and object selection are workspace edits. This primitive
preview uses fixed lighting/depth-sorted faces, with at most 64 boxes; it does not
render imported assets, transparency or intersecting surfaces accurately.

## Process time analysis

The **Process Gantt** panel shows per-node ready-queue and execution spans on a
shared millisecond timeline. Click a name to select its graph node; larger runs
use 100-row pages. Timings remain transient; component rows measure the whole
component invocation. See [Gantt analysis](GANTT_ANALYSIS.md).

## Components and layout

Create component opens an isolated draft of the current selection. Choose public
input/output/control names, keeping required graph boundaries exposed, then Save
component. Import/Export component exchanges standalone definitions. Choose matching
input sources and Insert component; each instance retains its own snapshot.
Remove component removes the library entry while instances continue to run.

Edit copy opens the body graph with ordinary commands and draft undo/redo. Save
creates a fresh immutable definition. Select a collapsed instance and a replacement
library definition, review its body/defaults, then Apply compatible update. The
public interface must match; current control values carry over. Undo restores the
exact old snapshot. Opening, importing or editing a library never updates instances.

Workspace layout can hide/show the library or inspector and set their widths.
Reset layout restores the browser arrangement. These settings, output choice,
camera and selection restore from downloaded projects independently of graph undo.

New project starts domain-independent. The graph selector switches among retained
graphs. Projects with no graphs can explicitly add one. Unknown nodes/edges are
retained; an unavailable node's existing ports can display but cannot form new
connections. No arbitrary extension code is loaded from a project file.

## Files and portability

Open project reads a local file; Download project prepares a complete replacement
file. Keep that downloaded file to save your edits. The app marks the exported
snapshot saved when it prepares the download, not when an OS file write is proven.
Unsaved replacement asks before discarding. Failed strict imports preserve current
content and show the transport error. There is no automatic recovery storage yet.

The browser rejects unsupported large integer-valued numbers/negative zero;
[decision 0019](architecture/decisions/0019-browser-json-transport.md) documents
the supported subset. Original files remain unchanged. Native viewer/panel JSON,
unknown extension fields, inactive graphs and assets survive export. Browser view
state uses `smartflow.web-editor@1`; Qt panel/viewer payloads are not browser
settings. Local asset references are retained without granting browser file access.

## Verification

```powershell
npm.cmd run check
npm.cmd run test:browser:install
npm.cmd run test:browser
```

Browser test binaries live in ignored `.cache/playwright`; results, downloads,
screenshots and failure traces are under `build/web-tests`. Tests launch isolated
Chromium on loopback, with no existing browser profile. The connected computer-use
browser runtime is currently unavailable on this machine. Initial automated
browser verification uses Chromium 153 on Windows. W6 production checks and
native/browser/native exchange pass for the [documented support set](WEB_DELIVERY.md).

Native packaged desktop acceptance stays separate and remains pending manual
results. Browser tests do not establish native desktop interaction acceptance.
See [the plan](WEB_IMPLEMENTATION_PLAN.md) and [roadmap](ROADMAP.md).
