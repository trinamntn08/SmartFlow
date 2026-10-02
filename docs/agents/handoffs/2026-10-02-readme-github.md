# Task handoff: GitHub README refresh

## Objective and scope

Make the root README a clear introduction for GitHub visitors. This is a
documentation-only change; application behavior and milestone status are unchanged.

## Current state

- Replaced the checkpoint-heavy overview with a short product introduction,
  available capabilities, and example workflows.
- Added a Windows build/install/run quick start using the documented native commands.
- Kept the browser preview secondary and linked detailed development and testing guides.
- Summarized repository layout and retained the preview, pending desktop acceptance,
  production plugin, and license status.

## Verification

- `node_modules/.bin/prettier.cmd --check README.md docs/agents/handoffs/2026-10-02-readme-github.md`: passed.
- PowerShell validation of all 13 relative README links: all targets exist.
- `git diff --check`: passed; reviewed the Markdown structure and final diff.
- Native quick-start commands match the existing development/testing guides;
  they were not rerun. No application code changed, so native and web behavior
  suites were not rerun. GitHub rendering was not inspected live.

## Decisions and open questions

No architecture or product decisions changed. Native Windows remains primary,
included 3D support remains required, and the independent data workflow remains visible.
Detailed checkpoint counts and session-specific package status belong in the roadmap
and testing guides rather than the repository landing page.

## Next steps

No remaining work for this documentation task. Desktop acceptance and product
milestones remain tracked in the existing roadmap and testing guides.
