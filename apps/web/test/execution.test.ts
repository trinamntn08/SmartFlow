import assert from 'node:assert/strict';
import test from 'node:test';
import { readFileSync } from 'node:fs';
import { parseProject } from '@smartflow/core';
import { ExtensionRegistry } from '@smartflow/extension-sdk';
import { dataExtension } from '@smartflow/data';
import { runGraph, type WorkerReply, type WorkerRequest } from '@smartflow/runtime';
import { ExecutionClient, type ExecutionState, type WorkerPort } from '../src/execution-client.ts';
const file = () =>
  parseProject(readFileSync(new URL('../../../examples/data.smartflow', import.meta.url), 'utf8'));
test('independent data workflow matches shared native expectations', async () => {
  const fixture = JSON.parse(
    readFileSync(new URL('../../../tests/fixtures/data-results-v1.json', import.meta.url), 'utf8'),
  );
  for (const expected of fixture.cases) {
    const project = file();
    project.project.graphs[0]!.nodes[0]!.parameters.multiplier = expected.multiplier;
    project.project.graphs[0]!.nodes[1]!.parameters.minimum = expected.minimum;
    const result = await runGraph(project, 'graph', new ExtensionRegistry([dataExtension]), {
      clone: structuredClone,
    });
    assert.equal(result.state, 'completed');
    const rows = (result.outputs.summary!.out as { rows: { value: number }[] }).rows;
    assert.deepEqual(
      rows.map((row) => row.value),
      [expected.count, expected.total, expected.mean],
    );
  }
});
test('client terminates cancelled/superseded workers and ignores all late replies', () => {
  const workers: {
    port: WorkerPort;
    message?: WorkerRequest;
    reply?: (reply: WorkerReply) => void;
    error?: (message: string) => void;
    stopped: boolean;
  }[] = [];
  const states: ExecutionState[] = [];
  const client = new ExecutionClient(
    () => {
      const worker = { stopped: false } as (typeof workers)[number];
      worker.port = {
        post: (message) => {
          worker.message = message;
        },
        subscribe: (reply, error) => {
          worker.reply = reply;
          worker.error = error;
        },
        terminate: () => {
          worker.stopped = true;
        },
      };
      workers.push(worker);
      return worker.port;
    },
    (state) => states.push(state),
  );
  const reply = (index: number) => {
    const worker = workers[index]!;
    const message = worker.message!;
    assert.equal(message.kind, 'run');
    if (message.kind !== 'run') throw new Error();
    worker.reply!({
      kind: 'status',
      identity: message.identity,
      nodeId: 'sample',
      state: 'running',
    });
    worker.error!('late error');
  };
  client.run(file(), 0, 'graph');
  client.run(file(), 0, 'graph');
  assert.ok(workers[0]!.stopped);
  let count = states.length;
  reply(0);
  assert.equal(states.length, count);
  client.invalidate(1, 'graph');
  assert.ok(workers[1]!.stopped);
  count = states.length;
  reply(1);
  assert.equal(states.length, count);
  assert.equal(states.at(-1)!.phase, 'obsolete');
  client.run(file(), 1, 'graph');
  client.cancel();
  assert.ok(workers[2]!.stopped);
  count = states.length;
  reply(2);
  assert.equal(states.length, count);
  assert.equal(states.at(-1)!.phase, 'cancelled');
  client.dispose();
});
