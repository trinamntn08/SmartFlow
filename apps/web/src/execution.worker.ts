import { ExtensionRegistry } from '@smartflow/extension-sdk';
import { runGraph, type WorkerRequest, type WorkerReply } from '@smartflow/runtime';
import { dataExtension } from '@smartflow/data';
import { sceneExtension } from '@smartflow/scene-3d';
const registry = new ExtensionRegistry([dataExtension, sceneExtension]);
const scope = globalThis as unknown as {
  onmessage: ((event: MessageEvent<WorkerRequest>) => void) | null;
  postMessage(reply: WorkerReply): void;
};
let active: number | undefined;
let cancelled = false;
scope.onmessage = async (event) => {
  const request = event.data;
  if (request.kind === 'cancel') {
    if (active === request.runId) cancelled = true;
    return;
  }
  if (active !== undefined) return;
  active = request.identity.runId;
  cancelled = false;
  const result = await runGraph(request.file, request.identity.graphId, registry, {
    clone: (value) => structuredClone(value),
    cancelled: () => cancelled,
    yield: () => new Promise((resolve) => setTimeout(resolve, 0)),
    onStatus: (nodeId, state, timing, elapsedMs) =>
      scope.postMessage({
        kind: 'status',
        identity: request.identity,
        nodeId,
        state,
        timing: timing ?? {},
        elapsedMs: elapsedMs ?? 0,
      }),
  });
  scope.postMessage({ kind: 'result', identity: request.identity, result });
  active = undefined;
};
