# Local development

Updated: 2026-10-02. The primary app is native C++17/Qt6 on Windows. The TypeScript
workspace has a retained browser editor (W2b) on the parallel web track, maintained by its
own checks. See [the web implementation plan](WEB_IMPLEMENTATION_PLAN.md).

## Native application

Use Visual Studio 2022 C++ tools, CMake 3.21 or newer and a matching Qt6 MSVC kit.
Read [native development](NATIVE_DEVELOPMENT.md) for build/test details and
[VS Code CMake Tools setup](NATIVE_DEVELOPMENT.md#run-from-vs-code-with-cmake-tools)
for presets and launch configurations. The local ignored `CMakeUserPresets.json`
is prepared on this machine; another developer should copy the documented example
and set their own Qt root.

```powershell
cmake --preset windows-local
cmake --build --preset windows-local-release
ctest --preset windows-local-release-tests
```

Run `smartflow` for 3D or `smartflow-data` for the independent table workflow.
Use [desktop testing](DESKTOP_TESTING.md) for the packaged runtime folder and
manual acceptance. The development build needs Qt runtime discovery; the installed
test folder includes it. No Node.js, browser server, API key, old checkout or
external service is required to run the desktop app.

## Web prototype and parallel implementation

Use Node.js 24 and npm 11. `.node-version`, `.nvmrc` and the root `packageManager`
record the tested versions; they do not install tools. Install dependencies in this
repository, not globally.

```sh
npm ci
npm run dev
```

Visit http://127.0.0.1:5173. Vite uses a strict loopback port and fails if occupied.
Use `npm.cmd` on PowerShell when execution policy blocks `npm.ps1`. The browser
workspace implements graph editing and strict file actions. Execution/scene rendering
are pending. See [the browser guide](WEB_DEVELOPMENT.md).

```sh
npm run check
npm run preview
```

`check` runs Prettier, strict TypeScript, persistence/command/SDK/adapter tests and a Vite
production build. Output is in ignored `apps/web/dist`; preview binds to loopback
port 4173. `npm test` runs behavior tests via Node 24's TypeScript support.
`npm run test:browser:install` installs isolated Chromium inside the repository cache;
`npm run test:browser` checks real editor interactions and file download/reopen.
There is no TypeScript graph runtime; native execution exists separately.

The web workspace separates browser/DOM source in `apps/web/tsconfig.json` from
Node/Vite tooling in `tsconfig.node.json`. Keep environments explicit.

## Editor and verification

Open the repository folder or `smartflow.code-workspace`. CMake Tools handles
native configure/build/test/run; Run and Debug supplies native launch choices.
The existing VS Code tasks still maintain the web prototype. C/C++, CMake Tools
and Prettier are recommended; recommendations do not install extensions.

Run appropriate native build/CTest checks for native changes and `npm.cmd run
check` before finishing implementation. For documentation-only changes, check
formatting and local links. Native offscreen/rendering checks and packaged startup
are useful evidence, but full desktop acceptance remains pending the user's manual
interaction results. Browser inspection applies only to web UI changes.

## Dependencies, boundaries and configuration

Use npm's committed lockfile for the future prototype. Add application dependencies
with `npm install <package> --workspace @smartflow/web`; shared tools belong at the
root. Reserved runtime/extension directories become npm workspaces only after
manifests and implementations are added. Core/SDK never import concrete domains.

Native sources use CMake and audited vendor provenance/licenses. Keep local Qt paths
in ignored user presets, not shared configuration. No global environment change is
required. Never commit secrets or private assets; future browser environment values
cannot store secrets. No external account, database or processing service is used.

See [architecture](architecture/README.md), [roadmap](ROADMAP.md) and
[contributing](../CONTRIBUTING.md).
