import { test, expect } from '@playwright/test';
import { readFileSync } from 'node:fs';
for (const example of ['data', 'scene', 'components']) {
  test(`process Gantt supports ${example}, selection and transient timings`, async ({ page }) => {
    page.on('dialog', (dialog) => dialog.accept());
    await page.goto('/');
    await page.getByLabel('Open example').selectOption(example);
    await page.getByRole('button', { name: 'Run graph' }).click();
    await expect(page.getByLabel('Execution state')).toHaveText('completed');
    const table = page.getByRole('table', { name: 'Process timings' });
    await expect(table).toBeVisible();
    const rows = table.locator('tbody tr');
    expect(await rows.count()).toBeGreaterThan(0);
    await expect(rows.first()).toContainText('completed');
    await expect(rows.first().getByRole('img')).toBeVisible();
    await rows.first().getByRole('button').click();
    const pending = page.waitForEvent('download');
    await page.getByRole('button', { name: 'Download project' }).click();
    const download = await pending;
    const file = JSON.parse(readFileSync((await download.path())!, 'utf8'));
    const graphId = file.project.graphs[0].id;
    expect(file.workspace['smartflow.web-editor@1'][graphId].selection).toEqual([
      await rows.first().getByRole('button').textContent(),
    ]);
    expect(file.timings).toBeUndefined();
    expect(file.project.graphs[0].nodes.every((node: object) => !('timings' in node))).toBe(true);
    await page
      .getByLabel('Process Gantt', { exact: true })
      .screenshot({ path: test.info().outputPath(`gantt-${example}.png`) });
  });
}
