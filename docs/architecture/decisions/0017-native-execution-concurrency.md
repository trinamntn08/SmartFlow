# 0017: Bounded native execution and owned invocation data

Date: 2026-10-02. Status: implemented through E1-E4.

## Decision

Keep native DAG execution independent of scene/data domains and Qt canvas.
Provide sequential and parallel modes over one dependency plan, with a total
delegate-thread budget of 1..64. Sequential is the default. Parallel schedules
only ready independent nodes, retaining component input/whole-body barriers.
One coordinator owns readiness and results. Workers publish synchronized
completions; a temporary lack of ready work does not end an in-flight run.

Use source-level execution policies for reentrancy, named exclusive resources
and a node-internal thread limit. Unregistered delegate safety serializes by
default. Resources constrain overlap, while graph dependencies specify order.
These are trusted bundled contracts, not a dynamic plugin ABI or sandbox.

Legacy contexts expose mutable inputs. Until a new const input API replaces
them, deep-clone input members per invocation and clone published outputs.
Factories must clone recursively owned data without retaining mutable aliases;
clone errors become node failures rather than sharing unsafe payloads. This
adds memory/copy cost and preserves graph reproducibility when a legacy delegate
mutates or forwards its private input. Scene geometry copies recursively clone
material extensions; table and numeric factories own their copied values.

Keep progress snapshots and execution options transient. Progress uses compiled
step counts and visible component aggregation, delivered on the Qt thread with
the same revision/request rejection as final results. Cancellation remains
cooperative and drains workers before destroying execution data. Internal node
parallelism uses reserved thread capacity and never queues child jobs into an
exhausted graph pool. Unmanaged third-party threads are outside that guarantee.

## Checkpoint status

- E1: policy/options contracts and invocation input/output isolation implemented.
- E2: bounded dependency scheduling, sequential/parallel options, resource locks
  and cancellation drain implemented.
- E3: native controls and live node/component progress implemented; audited
  bundled numeric/data/scene delegates opt into reentrancy.
- E4: managed node-internal parallel work shares reserved per-run capacity;
  nested work is serial, and child workers join before return/exception.

## Consequences

Do not change copied vendor APIs or infer safety from a const delegate method.
Audit bundled delegates before reentrant opt-in. Keep every execution result
separate from persisted graph/workspace state. Selective caching, remote work,
hard process cancellation and arbitrary third-party safety remain future work.
