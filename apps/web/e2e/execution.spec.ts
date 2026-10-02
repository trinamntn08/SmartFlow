import { test, expect } from '@playwright/test';
import { readFileSync } from 'node:fs';
test('editing controls remain responsive while a worker runs; cancellation clears outputs', async ({
  page,
}) => {
  test.setTimeout(60000);
  await page.goto('/');
  const file = JSON.parse(
    readFileSync(new URL('../../../examples/data.smartflow', import.meta.url), 'utf8'),
  );
  const graph = file.project.graphs[0];
  graph.nodes = Array.from({ length: 3000 }, (_, index) => ({
    ...graph.nodes[0],
    id: `sample-${index}`,
  }));
  graph.connections = [];
  await page.getByLabel('Project file').setInputFiles({
    name: 'many.smartflow',
    mimeType: 'application/json',
    buffer: Buffer.from(JSON.stringify(file)),
  });
  await page.getByRole('button', { name: 'Run graph' }).click();
  await page.getByLabel('Search nodes').fill('summary');
  await expect(page.getByLabel('Search nodes')).toHaveValue('summary');
  await page.getByRole('button', { name: 'Cancel run' }).click();
  await expect(page.getByLabel('Execution state')).toHaveText('cancelled');
  await expect(page.getByRole('table', { name: 'Table output' })).toHaveCount(0);
});
test('explicit run, obsolete results, output selection and save/reopen table state', async ({
  page,
}) => {
  const errors: string[] = [];
  page.on('pageerror', (error) => errors.push(error.message));
  page.on('dialog', (dialog) => dialog.accept());
  await page.goto('/');
  await expect(page.getByLabel('Execution state')).toHaveText('idle');
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByLabel('Execution state')).toHaveText('completed');
  await expect(page.getByRole('table', { name: 'Table output' })).toContainText('79');
  await page.getByRole('row').filter({ hasText: 'Total' }).click();
  await page.getByLabel('Minimum', { exact: true }).fill('40');
  await page.getByLabel('Minimum', { exact: true }).press('Enter');
  await expect(page.getByLabel('Execution state')).toHaveText('obsolete');
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByRole('table', { name: 'Table output' })).toContainText('48');
  await page.getByLabel('Viewed output').selectOption(JSON.stringify(['sample', 'out']));
  await expect(page.getByRole('table', { name: 'Table output' })).toContainText('Alpha');
  const pending = page.waitForEvent('download');
  await page.getByRole('button', { name: 'Download project' }).click();
  const download = await pending;
  const saved = readFileSync((await download.path())!);
  await page
    .getByLabel('Project file')
    .setInputFiles({ name: 'roundtrip.smartflow', mimeType: 'application/json', buffer: saved });
  await expect(page.getByLabel('Execution state')).toHaveText('obsolete');
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByLabel('Viewed output')).toHaveValue(JSON.stringify(['sample', 'out']));
  await expect(page.getByRole('table', { name: 'Table output' })).toContainText('Alpha');
  expect(errors).toEqual([]);
  await page.screenshot({ path: test.info().outputPath('data-output.png'), fullPage: true });
});
