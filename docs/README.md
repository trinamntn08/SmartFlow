# Documentation guide

Updated: 2026-10-02. SmartFlow is a native Windows C++/Qt development preview,
implemented through N6f and VS Code CMake Tools setup. Full packaged desktop
interaction acceptance is pending the user's manual results.

## Current guides

- [Roadmap and verification status](ROADMAP.md): delivered checkpoints and open work.
- [Product proposal](product/SmartFlowProposal.md): requirements and open audience/domain decisions, with current implementation status.
- [Architecture](architecture/README.md): current native ownership, state and execution contracts; deferred TypeScript work.
- [Local development](DEVELOPMENT.md): native entry point and preserved web commands.
- [Native development / VS Code](NATIVE_DEVELOPMENT.md): build, presets, launch and test details.
- [Desktop testing](DESKTOP_TESTING.md): packaged manual acceptance and result reporting.
- [Graph components](GRAPH_COMPONENTS.md): authoring, snapshots, controls/ports, import/export and removal.
- [Examples](../examples/README.md): scene, data, collapsed projects and standalone definition.
- [Scene extension](../extensions/scene-3d/README.md) and [data extension](../extensions/data/README.md): implemented domain scope and limits.
- [Agent workflow](agents/WORKFLOW.md) and [contributing](../CONTRIBUTING.md): instructions, verification and handoff practice.

## History and provenance

[Decision records](architecture/decisions/README.md) and
[dated handoffs](agents/handoffs/README.md) preserve original scope/verification;
older pending statements are superseded by current guides. The
[legacy source audit](architecture/legacy-reuse-analysis.md) is historical evidence,
not the active browser-first recommendation. [Vendor provenance](../native/vendor/README.md)
and per-module audits/licenses record copied sources and isolated patches.

The TypeScript [core](../packages/core/README.md) and
[SDK](../packages/extension-sdk/README.md) are preserved prototypes; their
[runtime](../packages/runtime/README.md) is reserved. They do not replace the
working native execution adapter or establish a production plugin system.
