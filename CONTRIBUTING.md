# Contributing

Start with [AGENTS.md](AGENTS.md) and [local development](docs/DEVELOPMENT.md).

1. Inspect the current branch and working tree.
2. Keep each change focused on a concrete behavior or setup improvement.
3. Respect the package boundaries in [architecture](docs/architecture/README.md).
4. Add behavior tests alongside new graph, runtime, persistence, or extension logic.
5. Run native build/CTest checks and `npm run check`. Check native startup and rendering for native UI work; browser inspection applies to web UI work.
6. Update the roadmap and public documentation when behavior changes.

Record durable decisions using [the decision template](docs/architecture/decisions/TEMPLATE.md). Describe the problem, chosen behavior, alternatives, and verification. Do not treat exploratory choices as settled product requirements.

No public license or contribution agreement has been selected. Resolve these before opening the project to external contributions.
