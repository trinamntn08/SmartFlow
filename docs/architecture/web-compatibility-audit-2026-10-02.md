# Native/browser compatibility audit

Date: 2026-10-02. Scope: source inspection and TypeScript parse/serialize probes,
not browser editing or native application round-trip acceptance.

Follow-up: [W1b](../agents/handoffs/2026-10-02-web-w1b-transport.md) resolves the
duplicate-key, size/depth and numeric corruption findings with strict transport
and explicit numeric rejection. The observations/probes below retain the original
W1 baseline; rerunning them now produces rejections for the malformed/unsafe cases.

## Evidence and result

Inspected [native codec](../../native/app/project/ProjectFile.cpp), its
[limits](../../native/app/project/ProjectFile.h),
[component validation](../../native/app/project/GraphComponent.cpp),
[standalone component transport](../../native/app/project/ComponentFile.cpp),
[native workspace adapter](../../native/app/workspace/WorkspaceWindow.cpp),
[TypeScript codec](../../packages/core/src/index.ts),
[SDK](../../packages/extension-sdk/src/index.ts) and
[web starter](../../apps/web/src/App.tsx).

The shared fixture and shipped scene/data/component projects pass semantic JSON
round-trip through TypeScript. This includes the component catalog, instance
snapshot and native workspace payload. Arbitrary native documents are **not yet
safe to round-trip**: JavaScript numeric parsing can silently change retained
integers, and transport validation differs.

| Area                              | Current evidence                                                                                                                                              | Required follow-up                                                                                                   |
| --------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| Envelope and identities           | Both require `smartflow`, schema 1, project/workspace objects, identified collections and positive safe node versions                                         | Keep shared fixture tests; expand malformed-field/identity boundary cases                                            |
| Unknown fields and dangling edges | Both codecs retain JSON and do not resolve extensions; current examples survive TypeScript serialization                                                      | Preserve retained objects through commands, not reconstructed UI models                                              |
| Duplicate JSON keys               | Native preflight rejects; TypeScript `JSON.parse` accepts the final value                                                                                     | Reject before conversion in W1b                                                                                      |
| Numeric transport                 | Native retains signed/unsigned 64-bit integer tokens; TypeScript rounds `9007199254740993` to `9007199254740992` in opaque content                            | Lossless transport or explicit rejection before conversion; never silently save rounded data                         |
| File size and nesting             | Native limits UTF-8 files to 16 MiB and container depth to 128; TypeScript has no corresponding explicit limits and accepted 130 nested arrays                | Match byte/depth semantics on import and export, with boundary fixtures                                              |
| Components                        | Native separately validates `smartflow.graph-component` files and instance bodies; TypeScript project parser preserves embedded JSON but has no component API | W5 adds separate contracts/validation and execution; preserve unsupported components meanwhile                       |
| Workspace                         | Native writes `smartflow.native-editor@1`; TypeScript retains workspace opaquely; web has no state adapter                                                    | Separate browser namespace and explicit conversion of supported portable fields                                      |
| Domain IDs and behavior           | Native extensions use `smartflow.data` and `smartflow.scene-3d`; their persisted short node IDs differ from qualified Qt registration IDs                     | Register persisted IDs/versions/ports in browser extensions; compare domain outputs using shared examples            |
| Runtime and SDK                   | Browser runtime is reserved; SDK capabilities and viewer adapter IDs are provisional                                                                          | Worker protocol, result ownership, cancellation/progress, stale-run rejection and UI adapters require implementation |
| Assets and saving                 | Native uses atomic replacement and filesystem references; web has no file UI                                                                                  | Begin with file input/download; retain unresolved asset URIs and explain capability limits                           |

No native codec or schema is changed by this audit. Native strictness is established
here by source inspection; malformed probes below were run against TypeScript only.
String whitespace edge cases and the full accepted numeric grammar still require
cross-language fixtures; this is not an exhaustive parser equivalence proof.

## Reproduce the probes

Run in repository-root PowerShell with Node.js 24:

```powershell
@'
import { readFileSync } from 'node:fs';
import assert from 'node:assert/strict';
import { parseProject, serializeProject } from './packages/core/src/index.ts';
for (const path of ['tests/fixtures/project-v1.json', 'examples/scene.smartflow',
  'examples/data.smartflow', 'examples/components.smartflow']) {
  const text = readFileSync(path, 'utf8');
  assert.deepEqual(JSON.parse(serializeProject(parseProject(text))), JSON.parse(text));
  console.log(`${path}: semantic round-trip passed`);
}
const base = readFileSync('examples/data.smartflow', 'utf8');
for (const [label, text] of [
  ['duplicate keys', base.replace('"schemaVersion": 1',
    '"schemaVersion": 2, "schemaVersion": 1')],
  ['64-bit opaque integer', base.replace('"workspace": {',
    '"opaqueInteger": 9007199254740993, "workspace": {')],
  ['deep opaque array', base.replace('"workspace": {',
    `"deep": ${'['.repeat(130)}0${']'.repeat(130)}, "workspace": {`)],
]) {
  try {
    const output = serializeProject(parseProject(text));
    console.log(`${label}: accepted${label.includes('integer')
      ? `; integer changed=${!output.includes('9007199254740993')}` : ''}`);
  } catch (error) { console.log(`${label}: rejected: ${error.message}`); }
}
try { parseProject(readFileSync('examples/filtered-summary.smartflow-component', 'utf8')); }
catch (error) { console.log(`standalone component through project parser: ${error.message}`); }
'@ | node --input-type=module
```

Observed: four round-trips passed; duplicate keys and deep array accepted; opaque
integer accepted and changed; standalone component rejected as unsupported project
format. After W1b, these observations must be replaced by verified strict transport
behavior. See [the implementation plan](../WEB_IMPLEMENTATION_PLAN.md).
