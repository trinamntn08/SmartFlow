# Local development

The primary application is now native C++/Qt: follow [native development](NATIVE_DEVELOPMENT.md).
The commands below maintain the preserved TypeScript prototype for a future extension.

## Prerequisites

- Node.js 24; `.node-version` and `.nvmrc` record the tested local version.
- npm 11; the root `packageManager` field records the bootstrap version.
- Git.

Existing installations are sufficient. No global package installation or engine build is needed. Version files document the environment; they do not install Node automatically.

## Install and run

From this repository:

```sh
npm ci
npm run dev
```

Visit http://127.0.0.1:5173. Stop the server with Ctrl+C. The port is strict: if another app owns it, Vite fails instead of silently changing the URL. Both dev and preview servers bind to loopback.

On Windows PowerShell, use `npm.cmd` if the shell rejects `npm.ps1`.

## Validation

```sh
npm run check
```

This runs Prettier, strict TypeScript, Node behavior tests, and a Vite production build. Output is in `apps/web/dist`, which is ignored by Git. Use `npm run preview` to serve it on port 4173.

The web workspace checks browser source with `apps/web/tsconfig.json` (DOM and Vite client types) and Vite tooling with `apps/web/tsconfig.node.json` (Node types). Keep browser source under `src` and include additional Node tooling files in the tooling configuration so each environment is checked separately.

Run `npm test` for persistence behavior tests using Node 24's built-in TypeScript support and test runner. There is no graph runtime yet. Add graph validation, parameter invalidation, and cancellation tests as those features arrive.

## Editor

Open `smartflow.code-workspace` or the repository folder. VS Code tasks cover install, dev, check, and build. Prettier is recommended for formatting. Recommendations do not automatically install extensions.

## Dependencies and workspaces

Use npm and the committed lockfile. Add application packages with `npm install <package> --workspace @smartflow/web`. Add shared development tools at the root. Packages under `packages/*` and `extensions/*` become npm workspaces once they have their own manifests.

Use package exports for shared code instead of reaching into another package's internal files. The core and SDK must remain independent of React and concrete domain packages.

## Environment and services

No environment variables, API keys, account login, database, desktop shell, or external processing service are required by the starter. Add an `.env.example` when a real configuration requirement appears. Keep local `.env` files out of Git. Never put secrets in client-side variables.

## References

- [Vite setup](https://vite.dev/guide/)
- [npm workspaces](https://docs.npmjs.com/cli/v11/using-npm/workspaces/)
