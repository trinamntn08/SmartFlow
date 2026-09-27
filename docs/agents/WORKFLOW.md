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

For a new extension, begin with a concrete graph, its data types, and viewer behavior. Extend the SDK only where that example needs a public contract. Verify that the platform does not import the new domain.

## Useful first task

> Read AGENTS.md, the product proposal, architecture notes, and roadmap. Specify the minimal graph and extension contracts for a primitive-transform-material-scene-viewer workflow. Keep the core domain-independent and identify how a table/text extension would use the same contracts. Propose a bounded implementation slice and acceptance tests before expanding the API.

## Handoffs

Use [the handoff template](HANDOFF_TEMPLATE.md) for work that spans sessions. Put durable decisions in architecture records and completed milestone status in the roadmap. Avoid treating conversation history as the only record of a decision.
