# SmartFlow

An independent, extensible visual workspace for interactive node graphs and their results.
3D scene workflows are a required capability. Other fields extend the same platform through packages; the final audience and application domain remain open.

## Current state

This repository contains the product proposal, architecture boundaries, agent guidance, and a runnable browser development starter. The graph editor, execution runtime, and domain packages are planned, not implemented. The project is named SmartFlow.

## Start locally

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

`check` verifies formatting, TypeScript, and a production build. `preview` serves an existing build on http://127.0.0.1:4173. No API keys, database, native engine, or remote service are required.

## Read first

- [Product proposal](docs/product/SmartFlowProposal.md)
- [Architecture](docs/architecture/README.md)
- [Roadmap and current status](docs/ROADMAP.md)
- [Local development](docs/DEVELOPMENT.md)
- [Agent instructions](AGENTS.md)
- [Agent workflow](docs/agents/WORKFLOW.md)
- [Contributing](CONTRIBUTING.md)

## Layout

| Path                     | Responsibility                                     | State               |
| ------------------------ | -------------------------------------------------- | ------------------- |
| `apps/web`               | Browser interface and application composition      | Development starter |
| `packages/core`          | Domain-independent project and graph model         | Reserved            |
| `packages/extension-sdk` | Public extension contracts                         | Reserved            |
| `packages/runtime`       | Validation, scheduling, execution state            | Reserved            |
| `extensions/scene-3d`    | Included 3D types, nodes, and viewer               | Reserved            |
| `extensions/data`        | Independent non-3D validation package              | Reserved            |
| `docs`                   | Product, decisions, development, and agent context | Available           |

Reserved folders contain ownership notes, not working libraries. Add their package manifests when implementation starts. The root npm workspace configuration already covers these locations.

## Repository status

Local Git repository; no remote or deployment is configured. Packages are private and no distribution license has been selected. Decide licensing before publishing. The original engine is not a dependency.
