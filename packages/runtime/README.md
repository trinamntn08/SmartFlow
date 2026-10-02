# Runtime

Status: local browser preview runtime (W3a), with a private package manifest.
The working native execution adapter is separately owned by `native/app/pipeline`.

Own graph validation, scheduling, cancellation, execution status, and result management through core and SDK contracts. Do not import concrete domains or UI components.

Typed DAG validation and deterministic execution use immutable SDK registration.
The host supplies cloning, cancellation, yielding and status publication. Failed
or cancelled runs return no outputs. Run identities reject superseded completion.
See [decision 0021](../../docs/architecture/decisions/0021-browser-worker-execution.md).
This is not a production runtime or dynamic plugin loader.
