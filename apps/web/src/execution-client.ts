import {
  RunGate,
  type NodeState,
  type RunIdentity,
  type RunResult,
  type WorkerReply,
  type WorkerRequest,
} from '@smartflow/runtime';
import type { ProjectFile } from '@smartflow/core';
export interface WorkerPort {
  post(message: WorkerRequest): void;
  subscribe(reply: (message: WorkerReply) => void, error: (message: string) => void): void;
  terminate(): void;
}
export function createExecutionWorker(): WorkerPort {
  const worker = new Worker(new URL('./execution.worker.ts', import.meta.url), { type: 'module' });
  return {
    post: (message) => worker.postMessage(message),
    subscribe: (reply, error) => {
      worker.onmessage = (event) => reply(event.data as WorkerReply);
      worker.onerror = (event) => error(event.message);
    },
    terminate: () => worker.terminate(),
  };
}
export interface ExecutionState {
  phase: 'idle' | 'running' | 'completed' | 'failed' | 'cancelled' | 'obsolete';
  statuses: Record<string, NodeState>;
  result?: RunResult;
  identity?: RunIdentity;
  error?: string;
}
export class ExecutionClient {
  #gate = new RunGate();
  #worker: WorkerPort | undefined;
  #revision = 0;
  #graphId = '';
  #state: ExecutionState = { phase: 'idle', statuses: {} };
  #factory: () => WorkerPort;
  #publish: (state: ExecutionState) => void;
  #pending: Record<string, NodeState> = {};
  #timer: ReturnType<typeof setTimeout> | undefined;
  #clearProgress(): void {
    if (this.#timer !== undefined) clearTimeout(this.#timer);
    this.#timer = undefined;
    this.#pending = {};
  }
  constructor(factory: () => WorkerPort, publish: (state: ExecutionState) => void) {
    this.#factory = factory;
    this.#publish = publish;
  }
  #update(state: ExecutionState): void {
    this.#state = state;
    this.#publish({ ...state, statuses: { ...state.statuses } });
  }
  invalidate(revision: number, graphId: string): void {
    if (revision === this.#revision && graphId === this.#graphId) return;
    this.#clearProgress();
    this.#revision = revision;
    this.#graphId = graphId;
    this.#gate.invalidate();
    this.#worker?.terminate();
    this.#worker = undefined;
    if (this.#state.phase !== 'idle') this.#update({ ...this.#state, phase: 'obsolete' });
  }
  run(file: ProjectFile, revision: number, graphId: string): void {
    this.#clearProgress();
    this.#worker?.terminate();
    this.#revision = revision;
    this.#graphId = graphId;
    const identity = this.#gate.begin(revision, graphId);
    this.#update({ phase: 'running', statuses: {}, identity });
    try {
      const worker = this.#factory();
      this.#worker = worker;
      worker.subscribe(
        (reply) => {
          if (!this.#gate.accepts(reply.identity, this.#revision, this.#graphId)) return;
          if (reply.kind === 'status') {
            Object.defineProperty(this.#pending, reply.nodeId, {
              value: reply.state,
              enumerable: true,
              configurable: true,
              writable: true,
            });
            if (this.#timer === undefined)
              this.#timer = setTimeout(() => {
                const pending = this.#pending;
                this.#pending = {};
                this.#timer = undefined;
                if (this.#gate.accepts(identity, this.#revision, this.#graphId))
                  this.#update({
                    ...this.#state,
                    statuses: { ...this.#state.statuses, ...pending },
                  });
              }, 32);
          } else {
            this.#clearProgress();
            this.#gate.invalidate();
            worker.terminate();
            this.#worker = undefined;
            this.#update({
              phase: reply.result.state,
              statuses: reply.result.statuses,
              result: reply.result,
              identity,
            });
          }
        },
        (error) => {
          if (!this.#gate.accepts(identity, this.#revision, this.#graphId)) return;
          this.#clearProgress();
          this.#gate.invalidate();
          worker.terminate();
          this.#worker = undefined;
          this.#update({ phase: 'failed', statuses: {}, identity, error });
        },
      );
      worker.post({ kind: 'run', identity, file });
    } catch (error) {
      this.#gate.invalidate();
      this.#worker?.terminate();
      this.#worker = undefined;
      this.#update({
        phase: 'failed',
        statuses: {},
        identity,
        error: error instanceof Error ? error.message : String(error),
      });
    }
  }
  cancel(): void {
    this.#clearProgress();
    this.#gate.invalidate();
    this.#worker?.terminate();
    this.#worker = undefined;
    this.#update({ phase: 'cancelled', statuses: {} });
  }
  dispose(): void {
    this.#clearProgress();
    this.#gate.invalidate();
    this.#worker?.terminate();
    this.#worker = undefined;
  }
}
