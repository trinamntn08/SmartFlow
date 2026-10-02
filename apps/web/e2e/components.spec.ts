import { test, expect } from '@playwright/test';
test('collapsed component controls execute named outputs and survive undo/reopen', async ({
  page,
}) => {
  const errors: string[] = [];
  page.on('pageerror', (error) => errors.push(error.message));
  page.on('dialog', (dialog) => dialog.accept());
  await page.goto('/');
  await page.getByLabel('Open example').selectOption('components');
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByLabel('Execution state')).toHaveText('completed');
  await page.getByLabel('Viewed output').selectOption(JSON.stringify(['summary-tool', 'summary']));
  await expect(page.getByRole('table', { name: 'Table output' })).toContainText('79');
  await page.getByLabel('minimum', { exact: true }).fill('40');
  await page.getByLabel('minimum', { exact: true }).press('Enter');
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByRole('table', { name: 'Table output' })).toContainText('48');
  await page.getByRole('button', { name: 'Undo', exact: true }).click();
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByRole('table', { name: 'Table output' })).toContainText('79');
  expect(errors).toEqual([]);
});
