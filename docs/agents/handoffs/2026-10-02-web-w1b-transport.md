# Task handoff: W1b strict browser project transport

## Objective and scope

Continue the approved web plan after W1. Resolve transport corruption before
editable import. Preserve native priority and schema-v1; no native application
behavior or browser UI changes.

## Current state

Core rejects duplicate decoded keys, invalid Unicode, depth/byte limit violations,
nonfinite and unsafe integer-valued numbers, and negative zero. Export checks
values/depth before JSON.stringify and UTF-8 size afterward. Identifier whitespace
agrees with native Qt rules, including one leading BOM in identifier decoding.
The web adapter strictly decodes original file bytes
and accepts one leading BOM without hiding its size. The web workspace declares
its core dependency and owns byte-adapter tests. Shared TypeScript configuration
permits source .ts imports under noEmit.

Shared fixtures are consumed by native_project and TypeScript. They record
intentional numeric differences, exact 16 MiB import/export limits, 128-container
nesting, Unicode and malformed JSON. The three shipped projects retain opaque
component/native workspace content.

## Verification

- `cmake --build --preset windows-local-release`: passed. Restricted nested moc
  spawning required direct `cmake -E cmake_autogen build/native/native/CMakeFiles/project_tests_autogen.dir/AutogenInfo.json Release`
  with process permissions before rebuilding.
- `ctest --preset windows-local-release-tests`: all 24 entries passed, including
  offscreen startup checks. Final identity/export cases checked again with
  `ctest --preset windows-local-release-tests -R native_project`.
- `npm.cmd run check`: formatting, TypeScript, 38 core tests, three web byte
  transport tests and production build passed.
- `git diff --check`: passed.

No browser UI changed, so browser interaction was not exercised. Native packaged
manual acceptance remains pending; the desktop package is unchanged. Transport
tests do not establish editor-level cross-application file exchange.

## Decisions and open questions

[Decision 0019](../../architecture/decisions/0019-browser-json-transport.md)
specifies numeric rejection and strict UTF-8 behavior. Native retains 64-bit
integers; the browser rejects unsupported values without writing the source.
Ordinary finite-double precision applies. Standalone components remain preserved
project content rather than supported browser execution or file APIs.

## Next steps

W2a: retained commands/history and extension lookup, with exact undo/redo,
opaque-content retention and workspace isolation. W2b will connect byte transport
to import/download actions and display failed-import errors. Desktop manual
results and demonstrated fixes proceed independently.
