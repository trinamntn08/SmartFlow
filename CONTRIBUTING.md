# Contributing

Start with [AGENTS.md](AGENTS.md), [the current roadmap](docs/ROADMAP.md) and
[local development](docs/DEVELOPMENT.md). Native C++/Qt is primary; TypeScript
packages are a deferred prototype.

1. Inspect the current branch and working tree.
2. Keep each change focused on a concrete behavior or setup improvement.
3. Respect the package boundaries in [architecture](docs/architecture/README.md).
4. Add behavior tests alongside new graph, runtime, persistence, or extension logic.
5. Run native build/CTest checks and `npm run check`. Check native startup and rendering for native UI work; browser inspection applies to web UI work.
6. Update the roadmap and public documentation when behavior changes. Preserve
   historical handoffs as dated records, and distinguish them from current guides.
7. Commit each verified implementation step with its checkpoint documentation.

Record durable decisions using [the decision template](docs/architecture/decisions/TEMPLATE.md). Describe the problem, chosen behavior, alternatives, and verification. Do not treat exploratory choices as settled product requirements.

No public license or contribution agreement has been selected. Resolve these before opening the project to external contributions.

Desktop interaction acceptance is pending the user's manual tests. Do not infer
acceptance from build-tree launch, passing offscreen checks or packaged startup.
Record concrete manual results before changing that status. No native automation
retry is required while the user has deferred it.
