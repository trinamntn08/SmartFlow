# 0009: First user test release is the Windows desktop app

Date: 2026-10-01. Status: accepted by explicit user direction.

## Decision

Deliver the first version for user testing as the native C++17/Qt6 Windows
desktop app. This makes the delivery target explicit beyond decision 0003's
implementation direction. Browser delivery is deferred; the preserved web
prototype and its checks cannot stand in for testing the desktop application.

Provide a local CMake install folder containing the executable, Qt plugins/DLLs
and MSVC runtime. Test startup without developer Qt environment variables, then
exercise the actual desktop editor, execution, undo and native file dialogs.
Keep automated native behavior and offscreen startup checks alongside this.

## Boundaries and acceptance

The included 3D extension remains required, the platform stays domain-independent,
and the independent non-3D extension remains part of N5 acceptance. This decision
does not mark workspace persistence, reusable components or full N5 complete.
The local folder is a development preview, not a public installer or a claim of
compatibility with untested Windows machines. Public distribution, signing and
other operating systems remain future work.

Until viewer configuration is persisted, opening a project chooses the last
registered terminal node as the default pinned output. This keeps the loaded
scene visible without importing scene types into workspace code. Unknown content
is preserved and unsupported graphs still block execution.

See [desktop testing](../../DESKTOP_TESTING.md) for commands and acceptance checks.
