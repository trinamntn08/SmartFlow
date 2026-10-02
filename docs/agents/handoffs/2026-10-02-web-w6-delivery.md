# W6: Browser acceptance and local delivery

## Objective and scope

Finish the approved W1–W6 browser plan, recording and committing each verified
step separately. Native desktop remains primary; manual packaged acceptance is
independent and still pending the user's results.

## Current state

The production browser preview delivers retained graph/file editing, worker
execution/cancellation/statuses, independent table and primitive scene workflows,
collapsed component execution/authoring and browser workspace restoration.
[Delivery instructions](../../WEB_DELIVERY.md) identify the supported/tested browser
set, build/serve commands, acceptance setup and current limits.

A test-only native executable exercises the actual `WorkspaceWindow` Open/Save
paths and executor in fresh offscreen processes. Production browser tests perform
native save → browser edit/download → fresh native reopen for data, scene,
components and unsupported content. Native result reporting resolves actual public
port mappings rather than generated member identities. Unknown nodes/fields,
inactive graphs, asset references and browser/native workspace separation survive.

The smaller 1280 × 800 scene check uncovered canvas minimum-height overflow over
viewer controls. The canvas now shrinks within its clipped graph region. Rendering,
picking, orbit/zoom, source edit/undo and reopen pass at scale factor 2; the final
scene screenshot was inspected and the overlap is gone.

## Verification

- `npm.cmd run check`: passed formatting, workspace type checks, 68 Node tests
  and production editor/worker build.
- `cmake --build --preset windows-local-release`: passed, including the test-only
  file-exchange executable. Final reporting correction was rebuilt separately.
- `ctest --preset windows-local-release-tests --output-on-failure`: all 24 passed
  (156.35 seconds), including offscreen startup and native behavior checks.
- With local `SMARTFLOW_NATIVE_EXCHANGE` and `SMARTFLOW_QT_ROOT` supplied,
  `npm.cmd run test:browser:production`: 14 passed (37.7 seconds). Only the raw
  development-module worker probe was intentionally skipped. All four native
  exchange tests ran and passed; production worker execution was exercised through
  UI controls. The report was copied to ignored `build/web-tests/production-report.json`.
- `npm.cmd run test:browser -- worker.spec.ts`: additional development worker probe
  verified separately. Browser binaries are isolated repository-local Chromium
  153.0.8010.12, with no user profile or connected computer-use interaction.

Initial checks exposed test-adapter option/mapping mistakes and the smaller-screen
overlap; those were corrected before the successful final verification. No failed
acceptance was relabeled as passing.

## Completed commits

| Step                              | Commit    |
| --------------------------------- | --------- |
| W1 planning/audit                 | `6c93670` |
| W1b transport                     | `0b4318c` |
| W2a commands/registry             | `8edc31f` |
| W2b editor/files                  | `99b4218` |
| W3a worker/data                   | `e9eba51` |
| W3b controls/table viewer         | `ffc8ed3` |
| W4 scene workflow/viewer          | `d3e6bba` |
| W5a component contracts/execution | `a891ca1` |
| W5b authoring/layout              | `647eee7` |

W6 is committed with this handoff; its hash is reported at completion.

## Decisions and open questions

No public deployment/provider was selected or published. Browser support is limited
to the documented isolated Chromium/Windows preview set. Native packaged manual
acceptance remains pending. Numeric transport limits, primitive rendering limits,
large-graph latency, imported assets, nested components, migrations, recovery,
dynamic plugins and remote services remain explicitly outside this completed plan.

## Next steps

No implementation step remains in the approved W1–W6 plan. Future work follows
user feedback and separate scope; collect the user's packaged desktop acceptance
results without claiming browser tests establish them.
