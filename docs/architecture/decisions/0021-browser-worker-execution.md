# 0021: Browser worker execution

Date: 2026-10-02. Status: accepted for the local web preview.

The runtime resolves exact bundled package/node versions through SDK contracts,
validates typed ports, parameter controls and DAG dependencies, then executes a
deterministic order. Core and runtime remain independent of domain extensions.
Every input and published output is cloned by the host; failed or cancelled runs
publish no partial outputs. The SDK context exposes node identity, cancellation
checks and cooperative yielding for long extension work.

The web application composes the registry in a module worker. Each request has a
run ID, semantic revision and graph ID. The client rejects late progress and
completion, and terminates workers on cancellation, replacement and semantic
invalidation. Workspace-only edits do not invalidate results. Termination also
stops an extension that fails to cooperate; this preview does not run arbitrary
downloaded plugins or provide a worker pool.

Use Vite's documented [module worker constructor](https://vite.dev/guide/features.html#web-workers).
[Worker termination](https://developer.mozilla.org/en-US/docs/Web/API/Worker/terminate)
provides immediate cancellation; [structured cloning](https://developer.mozilla.org/en-US/docs/Web/API/Window/structuredClone)
isolates values. Domain values must be cloneable and satisfy registered types.

The bundled data extension implements bounded Sample, Filter and Summary behavior,
with shared native/browser numerical expectations. Scene execution, viewer UI and
component execution follow in separate checkpoints. This remains a preview runtime,
not a production plugin system; native desktop remains the primary delivery.
