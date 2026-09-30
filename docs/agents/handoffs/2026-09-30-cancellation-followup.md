# Native cancellation follow-up discovered during N5a checks

## Objective and scope

Resolve the pipeline cancellation failure discovered while verifying the native
project-file checkpoint. This fix is independent of the new file codec.

## Current state

The copied `ParrallelProgress::addChildStep` supplies a poll callback that reads
the parent's cached `shouldStop()` flag, not the parent's live poll callback.
The executor checked the atomic cancellation request between nodes but did not
refresh the parent while a delegate was running. A cooperative delegate could
therefore keep running despite polling its supplied progress object.

The application execution adapter now attaches a child poll callback that polls
the root progress first. No vendor source changes. The root remains alive for
the complete parallel-progress lifetime. Nested delegate progress polling reaches
the same bridge through its parent.

The cancellation regression now verifies that the delegate actually observed
cancellation. Previously its five-second watchdog and the caller's five-second
wait could either time out or produce an apparent pass without proving cooperative
cancellation. The caller now waits beyond the watchdog and asserts observation,
so a watchdog-only exit cannot pass.

## Verification

- `cmake --build build/native --config Release --parallel 8`: passed.
- `ctest --test-dir build/native -C Release --output-on-failure`: all five current
  working-tree suites passed, including the pending N5a project tests.
- Pipeline report: 22 entries including setup/cleanup, zero failures. The suite
  duration dropped from about 14 seconds to 9 seconds after the polling bridge;
  correctness is asserted by delegate observation, not elapsed-time assumptions.
- `ctest --test-dir build/native -C Release -R '^native_pipeline$'
--repeat until-fail:3 --output-on-failure`: all three repetitions passed.
- `npm.cmd run check`: passed formatting, workspace type checks, persistence tests
  and production build. No web or native UI code is changed by this fix.

## Decisions and open questions

Cancellation remains cooperative: delegates must poll and return. This change
does not forcibly stop delegates or change the published-result discard policy.

## Next steps

Resume the N5a file-codec checkpoint after verifying this fix. See the separate
`2026-09-30-native-checkpoint-n5a.md` handoff for native persistence integration.
