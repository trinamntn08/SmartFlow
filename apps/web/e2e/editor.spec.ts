import { test, expect, type Page } from '@playwright/test';
import { readFile } from 'node:fs/promises';
import { resolve } from 'node:path';
import { parseProject } from '@smartflow/core';

async function exported(page: Page, path: string) {
  const pending = page.waitForEvent('download');
  await page.getByRole('button', { name: 'Download project', exact: true }).click();
  const download = await pending;
  await download.saveAs(path);
  return parseProject(await readFile(path, 'utf8'));
}
const pageErrors = new WeakMap<Page, string[]>();
test.beforeEach(async ({ page }) => {
  const errors: string[] = [];
  pageErrors.set(page, errors);
  page.on('pageerror', (error) => errors.push(error.message));
  page.on('dialog', (dialog) => {
    void dialog.accept();
  });
  await page.goto('/');
  await expect(page.locator('.graph-title')).toContainText('3 nodes');
});
test.afterEach(async ({ page }) => {
  expect(pageErrors.get(page)).toEqual([]);
});
test('inspector edits undo and redo; downloaded native example reopens with opaque workspace intact', async ({
  page,
  browser,
}, info) => {
  const errors: string[] = [];
  page.on('pageerror', (error) => errors.push(error.message));
  await page.getByRole('spinbutton', { name: 'Minimum', exact: true }).fill('40');
  await page.getByRole('spinbutton', { name: 'Minimum', exact: true }).press('Enter');
  await expect(page.getByText('Unsaved changes', { exact: true })).toBeVisible();
  await page.getByRole('button', { name: 'Undo', exact: true }).click();
  await expect(page.getByRole('spinbutton', { name: 'Minimum', exact: true })).toHaveValue('30');
  await page.getByRole('button', { name: 'Redo', exact: true }).click();
  await expect(page.getByRole('spinbutton', { name: 'Minimum', exact: true })).toHaveValue('40');
  const path = info.outputPath('edited.smartflow');
  const output = await exported(page, path);
  const original = parseProject(await readFile('examples/data.smartflow', 'utf8'));
  expect(output.workspace['smartflow.native-editor@1']).toEqual(
    original.workspace['smartflow.native-editor@1'],
  );
  expect(output.project.graphs[0]!.nodes[1]!.parameters.minimum).toBe(40);
  await page.screenshot({ path: info.outputPath('editor.png') });
  const fresh = await browser.newPage();
  await fresh.goto('/');
  await fresh.getByLabel('Project file', { exact: true }).setInputFiles(path);
  await expect(fresh.getByRole('spinbutton', { name: 'Minimum', exact: true })).toHaveValue('40');
  await fresh.close();
  expect(errors).toEqual([]);
});
test('new graph adds nodes, connects ports, deletes and restores exact connections', async ({
  page,
}, info) => {
  await page.getByRole('button', { name: 'New project', exact: true }).click();
  await page.locator('.library-node').filter({ hasText: 'Sample table' }).click();
  await page.locator('.library-node').filter({ hasText: 'Filter rows' }).click();
  const source = page
    .locator('.react-flow__node')
    .filter({ hasText: 'Sample table' })
    .locator('.source');
  const target = page
    .locator('.react-flow__node')
    .filter({ hasText: 'Filter rows' })
    .locator('.target');
  await source.dragTo(target);
  await expect(page.locator('.graph-title')).toContainText('1 connections');
  const connected = await exported(page, info.outputPath('connected.smartflow'));
  await page.getByRole('button', { name: 'Delete selection', exact: true }).click();
  await expect(page.locator('.graph-title')).toContainText('1 nodes');
  await page.getByRole('button', { name: 'Undo', exact: true }).click();
  const restored = await exported(page, info.outputPath('restored.smartflow'));
  expect(restored.project).toEqual(connected.project);
});
test('unknown nodes survive delete/undo/download and failed strict import leaves current project intact', async ({
  page,
}, info) => {
  const input = resolve('tests/fixtures/project-v1.json');
  await page.getByLabel('Project file', { exact: true }).setInputFiles(input);
  await expect(page.locator('.unsupported')).toHaveCount(1);
  const original = parseProject(await readFile(input, 'utf8'));
  await page.locator('.unsupported').click();
  await page.getByRole('button', { name: 'Delete selection', exact: true }).click();
  await page.getByRole('button', { name: 'Undo', exact: true }).click();
  const output = await exported(page, info.outputPath('unknown.smartflow'));
  expect(output.project).toEqual(original.project);
  const malformed = Buffer.from(
    (await readFile(input, 'utf8')).replace(
      '"schemaVersion": 1',
      '"schemaVersion": 2, "schemaVersion": 1',
    ),
  );
  await page
    .getByLabel('Project file', { exact: true })
    .setInputFiles({ name: 'bad.smartflow', mimeType: 'application/json', buffer: malformed });
  await expect(page.getByRole('alert')).toContainText('Duplicate JSON key');
  expect((await exported(page, info.outputPath('after-error.smartflow'))).project).toEqual(
    original.project,
  );
});
