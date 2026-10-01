# Agent guide

## Start here

Read `README.md`, `docs/ROADMAP.md`, and the documentation for the area being changed. Read `docs/product/SmartFlowProposal.md` for product decisions and `docs/architecture/README.md` before changing package boundaries. Inspect current code and Git status; do not assume roadmap items exist.

## Product constraints

- Keep 3D scene workflows as a required product capability, delivered through the included `scene-3d` extension.
- Keep the platform domain-independent. Do not require a scene, mesh, camera, or material in every project.
- Keep the audience and final application field open. Validate extensions using a separate non-3D workflow.
- Start with a native C++/Qt application, reusing audited source from the old repository. TypeScript/browser work is retained for a future extension, not the primary implementation.
- The first version delivered for user testing is the Windows native desktop app. Validate the packaged executable, desktop interaction and save/reopen workflow; a browser preview or passing web checks does not satisfy desktop acceptance.
- Never delete or modify the old repository. Copy reviewed sources and licenses into SmartFlow; do not reference the old checkout at build/runtime or copy machine-specific configuration.

## Ownership

- `native/app`: C++/Qt application composition and native adapters.
- `native/vendor`: copied legacy dependencies, preserved licenses and provenance; isolate local changes.
- `native/tests`: native behavior and application checks.
- `apps/web`: interface, panels, browser integration, and composition of packages.
- `packages/core`: serializable graph/project model; no React, DOM, renderer, or domain imports.
- `packages/extension-sdk`: public contracts; no imports from concrete extensions or application code.
- `packages/runtime`: graph validation and execution via contracts; no concrete domain imports.
- `extensions/*`: domain types, nodes, viewers, integrations; use public platform interfaces.
- `docs`: proposal, decisions, roadmap, development guide, and agent handoffs.

The TypeScript core and SDK are preserved prototypes. Runtime and extension directories remain reserved. Do not claim a production runtime or plugin system exists.

## Commands

Native build and test instructions are in `docs/NATIVE_DEVELOPMENT.md`. Run CMake builds and CTest for native changes. Record a checkpoint for each migration step. Use Node.js 24 and npm 11 for the preserved future web prototype:

- `npm ci`: restore the lockfile installation.
- `npm run dev`: start the web app on loopback port 5173.
- `npm run typecheck`: TypeScript checks.
- `npm run build`: production build.
- `npm run format:check`: formatting checks.
- `npm run format`: apply formatting.
- `npm run check`: formatting, type checking, and build.

On PowerShell, `npm.cmd` avoids execution-policy issues with `npm.ps1`. Keep `package-lock.json` in sync with dependency changes. Install tools in this repository, not globally.

## Implementation agreements

- Use C++17 and Qt6 for native work, descriptive names and small modules. Preserve vendor formatting. Use strict TypeScript and Prettier for the future web prototype.
- Separate stored project state, workspace/view state, and transient execution results.
- Keep graph semantics independent of the node-canvas library and scene rendering library.
- Use stable identifiers and explicit versioning for persisted node and package contracts.
- Preserve unknown extension content when loading projects; report unsupported execution instead of dropping nodes.
- Route viewer edits through explicit project commands so undo and graph reproduction stay coherent.
- Keep long work off the UI thread. Treat browser, local, and remote capabilities explicitly.
- Add dependencies for concrete needs; explain substantial architecture changes in a decision record.
- Never store secrets or large private assets in Git. Browser environment values are not secret storage.

## Verification and handoff

Commit each completed, verified implementation step separately before starting the next step. Include its checkpoint documentation in the commit and report the commit hash. Do not leave completed checkpoints bundled in an uncommitted working tree.

Run relevant native checks and `npm run check` before finishing code changes. Add meaningful behavior tests for migrated functionality. For native UI changes, run the offscreen startup check and inspect rendering where available; state any interactive verification limitations. Browser inspection applies to web UI changes.

Keep edits within the task, preserve unrelated work, and report actual verification and limitations. Update `docs/ROADMAP.md` when a milestone changes. For work spanning sessions, use `docs/agents/HANDOFF_TEMPLATE.md` and save the handoff under `docs/agents/handoffs/` with a descriptive name.

Use the user's current instructions to resolve scope. Do not infer a final framework, market, cloud provider, or desktop requirement from a prototype. Complete routine authorized work without adding approval gates.
