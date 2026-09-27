# 0001: Independent browser development scaffold

Status: accepted for bootstrap; replaceable after prototyping.

## Context

The product must retain 3D scene workflows, extend into other fields, and may ship as a standalone web app. The final audience and execution environment are undecided.

## Decision

Use a new repository, npm workspaces, and a minimal React/TypeScript/Vite browser starter. Use existing Node.js 24 and npm 11, exact dependency versions, and a lockfile. Reserve clear locations for core, SDK, runtime, and domain packages.

Keep domain implementation, a node-canvas library, a renderer, desktop shell, and remote services out of the bootstrap. Implement the public extension contract alongside a real 3D slice, then validate a non-3D package.

## Alternatives

- Documentation-only repository: avoids a framework choice but provides no runnable environment to verify.
- Extract the existing engine module: couples the new product to its original application and native dependencies too early.
- Desktop-first shell: may suit later native workloads, but is not required to validate the browser interaction.

## Consequences

The workspace runs locally without external services. The frontend choice is provisional, while domain independence and required 3D support are product constraints. Reserved package directories contain documentation until their behavior is implemented.
