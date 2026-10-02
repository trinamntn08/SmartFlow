# W3a: Worker runtime and independent data extension

## Objective and scope

Continue the approved web plan, with a separate verified commit per checkpoint.
Native desktop priority and its pending manual acceptance remain unchanged.

## Current state

Runtime now validates exact contracts, typed DAG edges, required inputs, parameter
controls and browser capabilities. Execution clones consumer inputs and outputs,
reports node states, and discards failed/cancelled outputs. The web worker composes
bundled extensions; its client rejects stale run/revision/graph replies and
terminates superseded work. Data Sample/Filter/Summary executes independently of
scene registration. Shared numerical fixtures are consumed by native and browser
tests. Scene nodes still report unavailable browser execution.

## Verification

`npm.cmd run check`: passed (59 Node behavior/transport tests, type checks,
formatting and production editor build). `npm.cmd run test:browser`: four Chromium
tests passed, including real worker data execution and progress. Native Release
CMake build passed; all four relevant data/startup CTest checks passed after the shared fixture
test correction. The first native fixture attempt used the project-schema reader;
it was corrected to the generic strict JSON transport.

## Decisions and open questions

See [decision 0021](../../architecture/decisions/0021-browser-worker-execution.md).
The worker is tested through Vite development serving; production worker delivery
is verified when the execution controls reference it in W3b. No connected user
browser or Windows desktop automation was used.

## Next steps

W3b: explicit Run/Cancel, node status, table outputs, obsolete-state labeling and
reopen execution. Then complete W4 scene capability, W5 components and W6 delivery.
