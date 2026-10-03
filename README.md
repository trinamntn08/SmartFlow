# SmartFlow

**Build workflows with nodes. Run them. Explore the results.**

SmartFlow is a visual workspace for connecting operations into interactive graphs.
Adjust parameters, inspect outputs, and turn useful groups of nodes into reusable
components.

The primary application is a **Windows desktop app built with C++17 and Qt6**.
It includes a 3D scene extension and a separate data workflow, so projects can work
with tables without requiring any 3D content. The platform is designed to support
additional domains; its final audience and application field remain open.

> **Development preview:** graph editing, execution, project saving, and viewers
> are implemented. The user's initial manual testing is OK so far.
> A public installer and production plugin system are future work.

## What you can do

- **Build graphs** with typed connections, editable parameters, and undo/redo.
  Numeric parameters can be edited directly inside nodes; see
  [inline controls](docs/INLINE_NODE_CONTROLS.md).
- **Run and inspect** workflows with background execution, cancellation,
  sequential or parallel scheduling, live progress, and timing analysis.
- **Explore results** in an interactive primitive 3D viewer or a table viewer.
- **Reuse components** with exposed ports and controls, a component library,
  and standalone import/export.
- **Use a prepared tool** through its exposed component controls and result viewer,
  then switch back to Build to edit the graph. See [Use mode](docs/USE_MODE.md).
- **Save your workspace** with project files that retain graph settings,
  canvas positions, viewer state, and panel layouts.
- **Arrange panels** by splitting, resizing, and swapping workspace regions.

## Example workflows

| Workflow           | Included operations                                 | Result                                       |
| ------------------ | --------------------------------------------------- | -------------------------------------------- |
| 3D scene           | Cube → Transform → Material → Scene                 | Interactive primitive scene preview          |
| Data               | Sample table → Filter rows → Summary                | Filtered rows and summary values             |
| Reusable component | Wrap filtering and summary operations into one node | A component with exposed inputs and controls |

Try the [example projects](examples/README.md), or read about the
[3D extension](extensions/scene-3d/README.md),
[data extension](extensions/data/README.md), and
[graph components](docs/GRAPH_COMPONENTS.md).

## Get started on Windows

### Build and run the desktop app

Install Visual Studio 2022 C++ tools, CMake 3.21 or newer, and a matching Qt6 MSVC
kit with Widgets, OpenGLWidgets, and Test. From the repository root, run the
following in PowerShell, replacing the Qt path with your installation:

```powershell
cmake -S . -B build/native -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/msvc2022_64
cmake --build build/native --config Release --parallel 8
cmake --install build/native --config Release --prefix "$PWD/build/desktop-test"
.\build\desktop-test\bin\smartflow.exe
```

The install step creates a local folder containing the app, runtime dependencies,
and examples. Keep that folder together when running it. Launch
`build/desktop-test/bin/smartflow-data.exe` for the independent table workflow.
The installed app runs without Node.js or a browser server.

For IDE setup and native tests, see [native development](docs/NATIVE_DEVELOPMENT.md).
For the save/reopen test workflow and current limitations, see
[desktop testing](docs/DESKTOP_TESTING.md).

### Browser preview

A parallel React/TypeScript preview supports graph editing, worker execution,
table and primitive scene viewers, components, and project import/download.
It remains a local preview with provisional extension contracts.

With Node.js 24 and npm 11 installed:

```sh
npm ci
npm run dev
```

Open <http://127.0.0.1:5173>. In Windows PowerShell, use `npm.cmd` if execution
policy blocks `npm`. See [web development](docs/WEB_DEVELOPMENT.md) for supported
workflows and checks.

## Documentation

- [Documentation guide](docs/README.md) — entry point for all guides.
- [Roadmap](docs/ROADMAP.md) — current status, verification, and planned work.
- [Product proposal](docs/product/SmartFlowProposal.md) — goals and open decisions.
- [Architecture](docs/architecture/README.md) — module boundaries and contracts.
- [Contributing](CONTRIBUTING.md) — development workflow and contribution status.

## Repository layout

| Directory     | Contents                                                         |
| ------------- | ---------------------------------------------------------------- |
| `native/`     | Desktop application, native tests, and copied dependencies       |
| `apps/web/`   | Browser preview                                                  |
| `packages/`   | TypeScript graph model, extension contracts, and preview runtime |
| `extensions/` | Included 3D scene and data workflows                             |
| `examples/`   | Sample projects and a reusable component definition              |
| `docs/`       | Guides, roadmap, architecture decisions, and development history |

## License

A project distribution license has not yet been selected. Copied dependencies
retain their own licenses and [provenance records](native/vendor/README.md).
