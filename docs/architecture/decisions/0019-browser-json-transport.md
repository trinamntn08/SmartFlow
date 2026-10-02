# 0019: Strict browser JSON transport and numeric rejection

Date: 2026-10-02. Status: implemented in W1b.

## Context

The [W1 audit](../web-compatibility-audit-2026-10-02.md) demonstrated duplicate-key
loss, missing resource limits and rounded native 64-bit integers in TypeScript.
Editable import depends on resolving these transport gaps.

## Decision

Keep schema-v1 and native numeric storage unchanged. The browser supports a
narrower numeric subset: finite IEEE doubles whose integer-valued representation
is within ±9007199254740991. Reject larger integer-valued numbers, including
exponent/decimal spellings, before constructing the document. Reject negative
zero because JSON.stringify changes its sign. Positive floating underflow follows
native double behavior; exact decimal arithmetic and original token spellings
are not preserved.

Core preflights JSON grammar, duplicate decoded keys, paired Unicode surrogates,
128-container nesting and the 16 MiB UTF-8 byte limit. Export checks retained
values before serialization and checks resulting byte size. Identifier whitespace
follows Qt's leading-BOM decoding and Unicode separator/TAB-through-CR/NEXT LINE rules. The web adapter
decodes original bytes with fatal UTF-8 validation and accepts one leading BOM,
as native does. It checks original byte size before decoding. Core stays
independent of DOM, filesystem APIs and concrete extensions.

Rejection raises an error and returns no replacement document. Neither parser
writes the source file. W2 file actions must surface errors and retain the current
editor document on failed import; no import UI exists in W1b.

## Consequences and verification

Some valid native files cannot open in the browser. A later lossless numeric model
requires a follow-up decision. Standalone component validation remains W5 work.

[Shared fixtures](../../../tests/fixtures/transport-v1.json) record native/browser
acceptance separately for numeric differences. Both suites check malformed
transport, Unicode, nesting and exact import/export size boundaries. All shipped
projects round-trip semantically through the browser codec. These checks establish
the supported transport subset, not native save → browser edit/save → native
reopen acceptance, which remains a W2/W6 gate.
