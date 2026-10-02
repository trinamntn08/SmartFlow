# Runtime

Status: reserved TypeScript runtime; no implementation or package manifest.
The working native execution adapter is separately owned by `native/app/pipeline`.

Own graph validation, scheduling, cancellation, execution status, and result management through core and SDK contracts. Do not import concrete domains or UI components.

Add a private package manifest and behavior tests when implementation begins. TypeScript execution remains deferred; see the [current architecture](../../docs/architecture/README.md)
and [roadmap](../../docs/ROADMAP.md) before choosing a future browser slice.
