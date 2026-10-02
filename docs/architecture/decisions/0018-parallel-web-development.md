# 0018: Develop the browser workspace alongside native desktop

Date: 2026-10-02. Status: accepted by user direction for W1 planning and audit.

## Context

The native desktop preview is available and packaged interaction acceptance remains
pending. The user requested a plan for web implementation alongside desktop work.
The browser repository contains a scaffold and provisional persistence/SDK contracts,
but no graph editor, runtime or domain implementations.

## Decision

Activate a separately verified web development track under
[the web implementation plan](../../WEB_IMPLEMENTATION_PLAN.md). This updates
0003's deferral of web work; C++/Qt remains primary and 0009's first Windows delivery
and acceptance requirements remain in force.

Share persisted contracts and compatibility fixtures across languages. Implement
browser UI/execution separately, with bundled data and required scene-3d extensions
using public platform contracts. Begin with browser-local execution in a worker
and project file import/download. Use a separate versioned browser workspace
namespace while preserving native state. Resolve transport corruption risks before
shipping editable native-file import.

W1 delivers documentation and audit evidence only. Later checkpoints require
their own implementation and verification; this decision does not assert delivery
of a runtime/plugin system or select a deployment service.

## Alternatives

Keeping web fully deferred would prevent the requested parallel progress. Replacing
Qt with a browser shell would reopen desktop implementation without a demonstrated
need. A native bridge or WebAssembly executor could reuse more computation but
adds deployment and integration work before the simple browser workflows need it.

## Consequences and verification

There will be separate UI and domain implementations to maintain. Contract changes
must include shared fixtures and native/browser checks. Browser acceptance remains
independent from packaged desktop acceptance. Initial compatibility is bounded by
implemented node versions and transport rules, not universal execution parity.

[The audit](../web-compatibility-audit-2026-10-02.md) found duplicate-key, numeric and
limit differences; W1b addresses these before W2 graph editing. Components remain
preserved unsupported content until their implementation checkpoint.
