# SmartFlow

An independent, extensible visual workspace for interactive node graphs and their results.
3D scene workflows are a required capability. Other fields extend the same platform through packages; the final audience and application domain remain open.

## Current state

SmartFlow starts as a native C++/Qt application based on audited source copied from the old studio engine. N1 provides the copied QtNodes canvas. N2 adds the pipeline foundation and background execution adapter. N3 connects the workspace, inspector and undo/redo. N4 adds the included scene-3d extension: Cube, Transform, Material, Scene and Merge nodes with an interactive primitive preview. N5a–N5e add the native project-file codec, retained-document editor commands and undo, and unavailable-node placeholders. Editor Save/Open actions and viewer-state persistence remain pending. The TypeScript browser starter, persistence prototype and SDK contracts are retained for a future extension.

## Start locally

Follow [native development](docs/NATIVE_DEVELOPMENT.md) to build and run the C++/Qt application. The old repository is preserved and is not required by the new build.

### Future web prototype

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

| Path                     | Responsibility                                        | State               |
| ------------------------ | ----------------------------------------------------- | ------------------- |
| `native/app`             | Native C++/Qt shell and adapters                      | Migration preview   |
| `native/vendor`          | Copied QtNodes and pipeline foundation, with licenses | Native dependencies |
| `native/tests`           | Canvas, pipeline, workspace and scene behavior tests  | N1–N4 checks        |
| `apps/web`               | Browser interface and application composition         | Development starter |
| `packages/core`          | Domain-independent project and graph model            | Initial persistence |
| `packages/extension-sdk` | Public extension contracts                            | Initial contracts   |
| `packages/runtime`       | Validation, scheduling, execution state               | Reserved            |
| `extensions/scene-3d`    | Included native 3D types, nodes, and viewer           | Primitive preview   |
| `extensions/data`        | Independent non-3D validation package                 | Reserved            |
| `docs`                   | Product, decisions, development, and agent context    | Available           |

Reserved folders contain ownership notes, not working libraries. Add their package manifests when implementation starts. The root npm workspace configuration already covers these locations.

## Repository status

The Git repository has a configured GitHub remote; no deployment configuration is included. Packages are private and no distribution license has been selected. Copied dependencies retain their own licenses. The original checkout is never a build/runtime dependency.
