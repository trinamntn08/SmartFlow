# W3b: Execution controls and table outputs

## Objective and scope

Deliver explicit worker execution and output inspection in the retained browser
editor. Opening a project still does not execute it.

## Current state

Run/Cancel controls, canvas node status and output selection are implemented.
Semantic edits mark old results obsolete; workspace edits retain valid results.
Table rows and pinned node/port choices use the browser workspace namespace and
survive download/reopen. Native workspace content remains unchanged. Worker
progress is batched at 32 ms intervals to avoid a render per message; the canvas
renders visible nodes and the textual status summary displays at most 100 entries.
Cancellation/replacement clears pending progress as well as terminating work.

## Verification

`npm.cmd run check` passed: formatting, type checks, 59 Node tests and production
build, including a separate bundled module-worker asset. Six isolated Chromium
interaction tests passed, including Run/obsolete output, data save/reopen and a
3000-node worker run with editable search and cancellation discarding outputs.
The stress test establishes available controls, not a quantitative latency target;
large graph import/layout can still take several seconds. The data-output
screenshot was inspected for readable graph, inspector and table layout.
Native sources did not change in W3b; native data/startup checks passed in W3a.

## Decisions and open questions

The app uses the worker contract from [decision 0021](../../architecture/decisions/0021-browser-worker-execution.md).
Scene execution/viewer and components are still pending. No connected user browser
or packaged desktop interaction acceptance is claimed.

## Next steps

W4: primitive scene extension, interactive scene viewer and saved camera workspace.
Then W5 component execution/authoring and W6 production/file exchange acceptance.
