# SmartFlow

An independent, extensible visual workspace for interactive node graphs and their results.
3D scene workflows are a required capability. Other fields extend the same platform through packages; the final audience and application domain remain open.

## Current state

Updated: 2026-10-02. SmartFlow is a native C++17/Qt6 Windows development preview
with graph editing, background execution/cancellation, inspector controls,
undo/redo, atomic project Save/Open and workspace/viewer restoration.
Selectable widget regions support H/V splits, swapping, resizing and saved layouts.
See [workspace widgets](docs/WORKSPACE_WIDGETS.md).
Sequential/parallel execution controls and live node/component progress are
available, with managed node-internal work sharing the thread budget; see
[execution](docs/EXECUTION.md).

The included 3D extension provides Cube/Transform/Material/Scene/Merge and a
bounded interactive primitive viewer. The separate data app provides sample,
filter and summary nodes with a table viewer, without scene dependencies.
Reusable components support collapsed snapshots, exposed ports/controls,
standalone import/export, undoable library removal and isolated editing copies
and explicit compatible instance updates (N6a through N6h).

Release build, 24 native CTest entries, packaged startup and web checks passed.
**Full desktop interaction acceptance remains pending the user's manual results**;
automated computer-use is deferred. The TypeScript browser starter, persistence
and SDK contracts are the foundation for the parallel web track, not the
delivered desktop app. W2b adds a browser graph editor with strict import/download,
retained undo/redo and data/scene metadata; browser execution/rendering remain
pending. See [the web guide](docs/WEB_DEVELOPMENT.md) and [web plan](docs/WEB_IMPLEMENTATION_PLAN.md).
See [the roadmap](docs/ROADMAP.md) for checkpoints and remaining work.

The refreshed `build/desktop-test` folder is ready for first manual testing.
See [the version notes and result sheet](docs/TEST_VERSION.md).

## Start locally

Use **Components** to create, insert, import/export and remove saved definitions.
Instances insert as one node by default, with optional expanded copies. See
[graph components](docs/GRAPH_COMPONENTS.md).

To build and run in VS Code, use the [CMake Tools setup](docs/NATIVE_DEVELOPMENT.md#run-from-vs-code-with-cmake-tools).

The first version for user testing is the **Windows desktop app**, built with C++/Qt.
Follow [desktop testing](docs/DESKTOP_TESTING.md) to build a local test folder and
launch `build/desktop-test/bin/smartflow.exe` directly. Qt and compiler runtime
dependencies are copied into that folder; Node.js and a browser server are not
required to run it. Try the [scene, data and component examples](examples/README.md). This is a development preview with the limitations listed in
the testing guide. The old repository is not required by the new build.

### Parallel web development

Use Node.js 24 (the tested version is in `.node-version`) and npm 11.

```sh
npm ci
npm run dev
```

Open http://127.0.0.1:5173. In Windows PowerShell, use `npm.cmd` if script execution policy prevents `npm` from running.

```sh
npm run check
npm run format
npm run preview
```

`check` verifies formatting, TypeScript, persistence tests, and a production build. `preview` serves an existing build on http://127.0.0.1:4173. No API keys, database, native engine, or remote service are required.

## Read first

- [Product proposal](docs/product/SmartFlowProposal.md)
- [Architecture](docs/architecture/README.md)
- [Roadmap and current status](docs/ROADMAP.md)
- [Local development](docs/DEVELOPMENT.md)
- [Agent instructions](AGENTS.md)
- [Agent workflow](docs/agents/WORKFLOW.md)
- [Contributing](CONTRIBUTING.md)

## Layout

| Path                     | Responsibility                                             | State               |
| ------------------------ | ---------------------------------------------------------- | ------------------- |
| `native/app`             | Native C++/Qt shell and adapters                           | Migration preview   |
| `native/vendor`          | Copied QtNodes and pipeline foundation, with licenses      | Native dependencies |
| `native/tests`           | Graph, execution, file, component, panel and viewer checks | 24 CTest entries    |
| `apps/web`               | Browser interface and application composition              | Retained editor     |
| `packages/core`          | Domain-independent project and graph model                 | Persistence/history |
| `packages/extension-sdk` | Public extension contracts                                 | Initial contracts   |
| `packages/runtime`       | Validation, scheduling, execution state                    | Browser preview     |
| `extensions/scene-3d`    | Included native 3D types, nodes, and viewer                | Primitive preview   |
| `extensions/data`        | Independent native table/filter/summary workflow           | Validation preview  |
| `docs`                   | Product, decisions, development, and agent context         | Available           |

Reserved folders contain ownership notes, not working libraries. Add their package manifests when implementation starts. The root npm workspace configuration already covers these locations.

## Repository status

The Git repository has a configured GitHub remote; no deployment configuration is included. Packages are private and no distribution license has been selected. Copied dependencies retain their own licenses. The original checkout is never a build/runtime dependency.
