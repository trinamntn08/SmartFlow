import { test, expect } from '@playwright/test';
import { readFileSync } from 'node:fs';
test('scene orbit, framing, deliberate source edit, undo and camera reopen', async ({ page }) => {
  const errors: string[] = [];
  page.on('pageerror', (error) => errors.push(error.message));
  page.on('dialog', (dialog) => dialog.accept());
  await page.goto('/');
  await page.getByLabel('Open example').selectOption('scene');
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByLabel('Execution state')).toHaveText('completed');
  const canvas = page.getByLabel('3D scene preview');
  await expect(canvas).toBeVisible();
  await page.getByRole('button', { name: 'Frame scene' }).click();
  const box = (await canvas.boundingBox())!;
  await canvas.hover();
  await page.mouse.wheel(0, -100);
  await page.mouse.move(box.x + box.width / 2, box.y + box.height / 2);
  await page.mouse.down();
  await page.mouse.move(box.x + box.width / 2 + 45, box.y + box.height / 2 + 10, { steps: 5 });
  await page.mouse.up();
  await canvas.click({ position: { x: box.width / 2, y: box.height / 2 } });
  await page.getByRole('button', { name: 'Increase source size' }).click();
  await expect(page.getByLabel('Execution state')).toHaveText('obsolete');
  await page.getByRole('button', { name: 'Undo', exact: true }).click();
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByLabel('Execution state')).toHaveText('completed');
  const pending = page.waitForEvent('download');
  await page.getByRole('button', { name: 'Download project' }).click();
  const download = await pending;
  const saved = readFileSync((await download.path())!);
  const file = JSON.parse(saved.toString());
  const original = JSON.parse(
    readFileSync(new URL('../../../examples/scene.smartflow', import.meta.url), 'utf8'),
  );
  expect(file.workspace['smartflow.native-editor@1']).toEqual(
    original.workspace['smartflow.native-editor@1'],
  );
  expect(file.project.graphs[0].nodes[0].parameters.size).toBe(3);
  const state =
    file.workspace['smartflow.web-editor@1'].graph.viewers['smartflow.scene-3d.viewer@1'];
  expect(state.yaw).not.toBe(35);
  await page
    .getByLabel('Project file')
    .setInputFiles({ name: 'scene-copy.smartflow', mimeType: 'application/json', buffer: saved });
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(canvas).toBeVisible();
  const next = page.waitForEvent('download');
  await page.getByRole('button', { name: 'Download project' }).click();
  const reopened = JSON.parse(readFileSync((await (await next).path())!).toString());
  expect(
    reopened.workspace['smartflow.web-editor@1'].graph.viewers['smartflow.scene-3d.viewer@1'],
  ).toEqual(state);
  expect(errors).toEqual([]);
  await page.screenshot({ path: test.info().outputPath('scene-viewer.png'), fullPage: true });
});
