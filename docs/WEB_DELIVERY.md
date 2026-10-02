# Local browser preview delivery

Updated: 2026-10-02. The approved W1–W6 browser scope is implemented. Native
C++17/Qt6 remains primary, and packaged Windows manual acceptance is still pending.

## Build and launch

From the repository root, with Node.js 24/npm 11:

```powershell
npm.cmd ci
npm.cmd run check
npm.cmd run preview --workspace @smartflow/web -- --host 127.0.0.1 --port 4173 --strictPort
```

Open http://127.0.0.1:4173. The production files are in `apps/web/dist`, including
the separate execution-worker asset. Vite preview is for local build verification;
for hosted delivery, copy the entire directory to a static HTTP/HTTPS server,
retaining the asset paths and JavaScript module MIME types. The build currently
targets the server root. A subpath requires rebuilding with the appropriate Vite
`base` and repeating acceptance on that path. No public provider or deployment is
selected. These instructions follow [Vite's static delivery guide](https://vite.dev/guide/static-deploy.html).

Use [the browser workflow guide](WEB_DEVELOPMENT.md) for editing, executing,
components and workspace controls. Projects open without execution. Files are
imported/downloaded locally; save the downloaded replacement to keep work.
There are no accounts, uploads, external API keys or remote executor.

## Supported and tested preview environment

| Environment                                 | Evidence                                                         |
| ------------------------------------------- | ---------------------------------------------------------------- |
| Isolated Chromium 153.0.8010.12 on Windows  | Production editor/worker interaction and native file exchange    |
| 1440 × 900 desktop viewport, scale factor 1 | Editor, tables, components, history, files and cancellation      |
| 1280 × 800 desktop viewport, scale factor 2 | Scene painting, picking, camera, deliberate edit/undo and reopen |

This is the tested preview support set. Installed user browser profiles, other
browser versions/engines, mobile layouts and packaged native desktop interactions
have not been accepted in this run. The connected computer-use browser was unavailable.

## Reproduce acceptance

```powershell
npm.cmd run test:browser:install
npm.cmd run build
npm.cmd run test:browser:production
```

The suite starts a fresh loopback production preview on strict port 4173. It tests
both domains, component authoring, output selection, save/reopen, dirty errors,
responsive controls and cancellation during a 3000-node run. The development-only
raw worker-entry probe is intentionally skipped; production worker execution is
checked through the actual Run graph controls. Run `npm.cmd run test:browser`
for that additional development worker probe on port 5173.

For actual native save → browser edit/download → fresh native window reopen,
build the test-only native adapter and supply local runtime paths:

```powershell
cmake --build --preset windows-local-release
$env:SMARTFLOW_NATIVE_EXCHANGE = (Resolve-Path build/native/native/Release/file_exchange_check.exe).Path
$env:SMARTFLOW_QT_ROOT = '<your Qt MSVC kit directory>'
npm.cmd run test:browser:production
ctest --preset windows-local-release-tests --output-on-failure
```

Native presets are local; see [native development](NATIVE_DEVELOPMENT.md). Without
the adapter path, four file-exchange tests explicitly skip. W6 supplied it and
executed all four: data, scene, collapsed components and unsupported content. The
test-only adapter uses `WorkspaceWindow` Open/Save and the real executor in fresh
offscreen processes. It is not installed with the desktop app and does not count
as manual desktop acceptance. Reports/downloads/screenshots live under ignored
`build/web-tests`; browser binaries are repository-local in `.cache/playwright`.

## Current limits

Strict browser transport rejects unsafe integer-valued numbers and negative zero;
see [decision 0019](architecture/decisions/0019-browser-json-transport.md). Unknown
extension content is retained and explicitly blocks unsupported execution.
Scene rendering is a 64-object opaque primitive painter, with fixed lighting and
depth-sorted faces. Imported assets, intersecting-surface accuracy, transparency,
textures and production GPU rendering remain outside scope. Asset paths are
references, not browser file permissions. Large graph import/layout has no latency
guarantee. Nested components, migrations, worker pools, arbitrary plugin loading,
recovery storage and cloud services remain future work.
