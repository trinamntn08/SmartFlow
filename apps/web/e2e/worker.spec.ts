import { test, expect } from '@playwright/test';
import { readFileSync } from 'node:fs';
const file = JSON.parse(
  readFileSync(new URL('../../../examples/data.smartflow', import.meta.url), 'utf8'),
);
test('real browser worker executes independent data and reports progress', async ({ page }) => {
  await page.goto('/');
  const result = await page.evaluate(async (file) => {
    const worker = new Worker('/src/execution.worker.ts', { type: 'module' });
    const states: string[] = [];
    return await new Promise<{ total: number; states: string[] }>((resolve, reject) => {
      const timeout = setTimeout(() => {
        worker.terminate();
        reject(new Error('Worker timed out'));
      }, 10000);
      worker.onerror = (event) => {
        clearTimeout(timeout);
        worker.terminate();
        reject(new Error(event.message));
      };
      worker.onmessage = (event) => {
        const message = event.data;
        if (message.kind === 'status') states.push(message.state);
        else {
          clearTimeout(timeout);
          worker.terminate();
          resolve({ total: message.result.outputs.summary.out.rows[1].value, states });
        }
      };
      worker.postMessage({
        kind: 'run',
        identity: { runId: 1, revision: 0, graphId: 'graph' },
        file,
      });
    });
  }, file);
  expect(result.total).toBe(79);
  expect(result.states.filter((state) => state === 'completed')).toHaveLength(3);
});
