# 0011: Independent native data workflow

Status: accepted for N5h, 2026-10-01.

## Decision

Add `extensions/data/native` using existing public native contribution contracts.
Sample table, Filter rows and Summary share a versioned table result type with
label/value rows. The extension supplies its own factory and read-only table
viewer. Numeric controls use the current generic inspector without any editor
or runtime domain branches. Published outputs are value copies.

The composition root selects the data preset with `--data`. Keep scene-3d as
the default and required bundled capability. Also build `smartflow-data.exe`
from the shared entry point with a compile-time composition choice. This
validation executable and `data_tests` do not link scene-3d or scene-math,
proving independence beyond simply omitting scene nodes from a demo graph.

No SDK or schema-envelope change is needed. Project files retain explicit
package/type/version identities, graph parameters and opaque workspace state.
The data viewer persists row selection through N5g's hooks. Missing data nodes
use the existing unavailable placeholders and execution diagnostics.

## Scope and limits

This is a six-row bundled sample and one numeric filter, not external table
import or a general query engine. Summary emits count, total and mean; the
empty-table mean is defined as zero for this preview. Results are transient,
not serialized assets. Row selection is by index, not a durable record identity.
The source-level contract is for trusted bundled extensions, not a binary
plugin ABI. Reusable graph components remain a separate future checkpoint.
