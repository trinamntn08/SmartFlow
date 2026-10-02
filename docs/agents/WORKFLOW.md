# Working with AI coding agents

## Open the correct repository

Open this repository as its own project/workspace in your agent or editor. A conversation opened in the old engine repository may retain that repository's context and filesystem permissions.

Use `AGENTS.md` as the shared instruction source. Codex discovers repository instructions through this file; see [official AGENTS.md guidance](https://learn.chatgpt.com/docs/agent-configuration/agents-md). `CLAUDE.md` and `.github/copilot-instructions.md` point other assistants to the same guide. For other tools, explicitly ask them to read `AGENTS.md`.

This setup provides development context. It does not install an AI SDK into the product, select a model, modify global agent permissions, or require an API key. Existing agent installations and authentication remain separate from this repository.

## Start a task

1. Read `AGENTS.md` and `docs/ROADMAP.md`.
2. Inspect the working tree and relevant code.
3. State the intended behavior and how completion will be verified.
4. Implement a bounded slice, respecting package ownership.
5. Run relevant checks and report results and remaining limitations.
6. Commit the completed step with its checkpoint documentation before starting the next step, and report the commit hash.

For a new extension, begin with a concrete graph, its data types, and viewer behavior. Extend the SDK only where that example needs a public contract. Verify that the platform does not import the new domain.

## Current continuation context

Read [the current roadmap](../ROADMAP.md) and the
[handoff index](handoffs/README.md) before choosing work. Native implementation is
complete through N7, with VS Code launch setup. Do not reimplement delivered graph,
file or component features based on old handoff "next steps".

The user has deferred automated desktop interaction and will test manually. Keep
full N5 acceptance pending their results; continue unrelated authorized work
without repeatedly retrying native computer-use. Definition editing copies are
implemented; explicit replacement is delivered under decision 0015. Nested components and
asset distribution require separate scoped follow-up decisions.

## Handoffs

Use [the handoff template](HANDOFF_TEMPLATE.md) for work that spans sessions. Put durable decisions in architecture records and completed milestone status in the roadmap. Avoid treating conversation history as the only record of a decision. Dated
handoffs are historical snapshots; current guides and explicit user instructions
supersede their pending items.
