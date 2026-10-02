import { test, expect } from '@playwright/test';
import { readFileSync } from 'node:fs';
test.beforeEach(async ({ page }) => {
  page.on('dialog', (dialog) => dialog.accept());
  await page.goto('/');
});
test('extract, standalone export/import and bound component insertion', async ({ page }) => {
  await page.getByRole('button', { name: 'Create component', exact: true }).click();
  const dialog = page.getByRole('dialog', { name: 'Component editor' });
  await dialog.getByLabel('Component title').fill('Filter tool');
  await dialog.getByLabel('Name inputs filter:in').fill('table');
  await dialog.getByLabel('Name outputs filter:out').fill('rows');
  await dialog.getByLabel('Name controls filter:minimum').fill('minimum');
  await dialog.getByRole('button', { name: 'Save component', exact: true }).click();
  await expect(dialog).toHaveCount(0);
  await page.getByLabel('Component library').selectOption('0');
  const pending = page.waitForEvent('download');
  await page.getByRole('button', { name: 'Export component', exact: true }).click();
  const bytes = readFileSync((await (await pending).path())!);
  await page
    .getByLabel('Source table')
    .selectOption(JSON.stringify({ nodeId: 'sample', portId: 'out' }));
  await page.getByRole('button', { name: 'Insert component', exact: true }).click();
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByLabel('Execution state')).toHaveText('completed');
  await page.getByRole('button', { name: 'Remove component', exact: true }).click();
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByLabel('Execution state')).toHaveText('completed');
  await page.getByLabel('Component file').setInputFiles({
    name: 'filter.smartflow-component',
    mimeType: 'application/json',
    buffer: bytes,
  });
  await page.getByLabel('Component library').selectOption('0');
  await expect(page.getByLabel('Component library')).toContainText('Filter tool');
});
test('isolated copy editing, explicit compatible replacement and exact undo', async ({ page }) => {
  await page.getByLabel('Open example').selectOption('components');
  await page.getByLabel('Component library').selectOption('0');
  await page.getByRole('button', { name: 'Edit copy', exact: true }).click();
  const dialog = page.getByRole('dialog', { name: 'Component editor' });
  await dialog.getByLabel('Component title').fill('Edited tool');
  await dialog.locator('.react-flow__node[data-id="filter"]').click();
  await dialog.getByLabel('Minimum', { exact: true }).fill('10');
  await dialog.getByLabel('Minimum', { exact: true }).press('Enter');
  await dialog.getByRole('button', { name: 'Save component', exact: true }).click();
  await expect(dialog).toHaveCount(0);
  await page.getByLabel('Component library').selectOption('1');
  await page.getByText('Update selected instance', { exact: true }).click();
  await page.getByRole('button', { name: 'Apply compatible update' }).click();
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByLabel('Execution state')).toHaveText('completed');
  await expect(page.getByRole('table', { name: 'Table output' })).toContainText('79');
  await page.getByRole('button', { name: 'Undo', exact: true }).click();
  const pending = page.waitForEvent('download');
  await page.getByRole('button', { name: 'Download project' }).click();
  const file = JSON.parse(readFileSync((await (await pending).path())!).toString());
  const instance = file.project.graphs[0].nodes[1];
  expect(instance.component.id).toBe('filtered-summary');
  expect(instance.parameters.minimum).toBe(30);
  expect(file.project.components[1].graph.nodes[0].parameters.minimum).toBe(10);
  await page.screenshot({ path: test.info().outputPath('component-library.png'), fullPage: true });
});
test('workspace panel choices restore independently of execution and project history', async ({
  page,
}) => {
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByLabel('Execution state')).toHaveText('completed');
  await page.getByText('Workspace layout', { exact: true }).click();
  await page.locator('.layout-controls').getByLabel('Inspector', { exact: true }).uncheck();
  await expect(page.locator('aside.inspector')).toBeHidden();
  await expect(page.getByLabel('Execution state')).toHaveText('completed');
  await expect(page.getByRole('button', { name: 'Undo', exact: true })).toBeDisabled();
  const pending = page.waitForEvent('download');
  await page.getByRole('button', { name: 'Download project' }).click();
  const bytes = readFileSync((await (await pending).path())!);
  await page
    .getByLabel('Project file')
    .setInputFiles({ name: 'layout.smartflow', mimeType: 'application/json', buffer: bytes });
  await expect(page.locator('aside.inspector')).toBeHidden();
  await page.getByRole('button', { name: 'Reset layout' }).click();
  await expect(page.locator('aside.inspector')).toBeVisible();
});
