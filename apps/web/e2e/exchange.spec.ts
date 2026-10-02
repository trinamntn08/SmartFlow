import { test, expect } from '@playwright/test';
import { execFileSync } from 'node:child_process';
import { readFileSync, writeFileSync, existsSync } from 'node:fs';
import { resolve, delimiter } from 'node:path';
const adapter = process.env.SMARTFLOW_NATIVE_EXCHANGE;
const qt = process.env.SMARTFLOW_QT_ROOT;
function native(
  domain: string,
  input: string,
  output: string,
  report: string,
  allowFailure = false,
) {
  execFileSync(
    adapter!,
    [
      '--domain',
      domain,
      '--input',
      input,
      '--output',
      output,
      '--report',
      report,
      ...(allowFailure ? ['--allow-failure'] : []),
      '-platform',
      'offscreen',
    ],
    {
      env: {
        ...process.env,
        PATH: qt ? `${resolve(qt, 'bin')}${delimiter}${process.env.PATH ?? ''}` : process.env.PATH,
      },
      timeout: 25000,
      stdio: 'pipe',
    },
  );
  return JSON.parse(readFileSync(report, 'utf8'));
}
for (const domain of ['data', 'scene', 'components'])
  test(`native save → production browser edit/download → fresh native reopen: ${domain}`, async ({
    page,
  }) => {
    test.skip(
      !adapter || !existsSync(adapter),
      'Set SMARTFLOW_NATIVE_EXCHANGE to the built test-only adapter and SMARTFLOW_QT_ROOT to the Qt kit.',
    );
    const original = resolve(`examples/${domain}.smartflow`),
      saved = test.info().outputPath('native.smartflow'),
      nativeReport = test.info().outputPath('native-report.json');
    native(domain === 'scene' ? 'scene' : 'data', original, saved, nativeReport);
    const first = JSON.parse(readFileSync(saved, 'utf8'));
    await page.goto('/');
    page.on('dialog', (dialog) => dialog.accept());
    await page.getByLabel('Project file').setInputFiles(saved);
    const label = domain === 'scene' ? 'size' : domain === 'components' ? 'minimum' : 'Minimum';
    const input = page.locator('aside.inspector').getByLabel(label, { exact: true });
    await input.fill(domain === 'scene' ? '4' : '40');
    await input.press('Enter');
    await page.getByRole('button', { name: 'Run graph' }).click();
    await expect(page.getByLabel('Execution state')).toHaveText('completed');
    const pending = page.waitForEvent('download');
    await page.getByRole('button', { name: 'Download project' }).click();
    const bytes = readFileSync((await (await pending).path())!),
      browserFile = test.info().outputPath('browser.smartflow');
    writeFileSync(browserFile, bytes);
    const browser = JSON.parse(bytes.toString());
    expect(browser.workspace['smartflow.native-editor@1']).toEqual(
      first.workspace['smartflow.native-editor@1'],
    );
    const reopened = test.info().outputPath('native-reopened.smartflow'),
      report = test.info().outputPath('reopened-report.json');
    const result = native(domain === 'scene' ? 'scene' : 'data', browserFile, reopened, report);
    expect(result.succeeded).toBe(true);
    const final = JSON.parse(readFileSync(reopened, 'utf8'));
    expect(final.project).toEqual(browser.project);
    expect(final.workspace['smartflow.web-editor@1']).toEqual(
      browser.workspace['smartflow.web-editor@1'],
    );
    if (domain === 'scene') {
      const output = Object.values(result.outputs.scene)[0] as { objects: { maximum: number[] }[] };
      expect(output.objects[0]!.maximum[1]).toBeCloseTo(2, 5);
    } else {
      const node = domain === 'components' ? 'summary-tool' : 'summary';
      const output = (result.outputs[node].summary ?? Object.values(result.outputs[node])[0]) as {
        rows: { value: number }[];
      };
      expect(output.rows[1]!.value).toBe(48);
    }
  });
test('native/browser/native exchange retains unsupported nodes, inactive graphs, assets and opaque fields', async ({
  page,
}) => {
  test.skip(!adapter || !existsSync(adapter), 'Native exchange adapter not configured.');
  const source = test.info().outputPath('unsupported-source.smartflow');
  const file = JSON.parse(readFileSync(resolve('examples/data.smartflow'), 'utf8'));
  file.project.graphs[0].nodes.push({
    id: 'future',
    packageId: 'future.package',
    typeId: 'unknown',
    version: 2,
    parameters: { future: { keep: true } },
    opaque: [1, 'keep'],
  });
  file.project.packages.push({ id: 'future.package', version: '9' });
  file.project.graphs.push({ id: 'inactive', nodes: [], connections: [], future: 'keep' });
  file.project.assets = [{ id: 'asset', uri: 'private-local-reference.obj', future: 'keep' }];
  file.opaque = { retain: true };
  writeFileSync(source, JSON.stringify(file));
  const saved = test.info().outputPath('native-unsupported.smartflow');
  native('data', source, saved, test.info().outputPath('native-unsupported-report.json'), true);
  await page.goto('/');
  await page.getByLabel('Project file').setInputFiles(saved);
  await page.locator('aside.inspector').getByLabel('Minimum', { exact: true }).fill('40');
  await page.locator('aside.inspector').getByLabel('Minimum', { exact: true }).press('Enter');
  await page.getByRole('button', { name: 'Run graph' }).click();
  await expect(page.getByLabel('Execution state')).toHaveText('failed');
  const pending = page.waitForEvent('download');
  await page.getByRole('button', { name: 'Download project' }).click();
  const bytes = readFileSync((await (await pending).path())!),
    browserFile = test.info().outputPath('browser-unsupported.smartflow');
  writeFileSync(browserFile, bytes);
  const browser = JSON.parse(bytes.toString());
  const reopened = test.info().outputPath('native-unsupported-reopened.smartflow');
  const result = native(
    'data',
    browserFile,
    reopened,
    test.info().outputPath('unsupported-reopened-report.json'),
    true,
  );
  expect(result.succeeded).toBe(false);
  const final = JSON.parse(readFileSync(reopened, 'utf8'));
  expect(final.project).toEqual(browser.project);
  expect(final.opaque).toEqual(file.opaque);
  expect(final.project.graphs[1]).toEqual(file.project.graphs[1]);
  expect(final.project.assets).toEqual(file.project.assets);
  expect(final.project.graphs[0].nodes.at(-1)).toEqual(file.project.graphs[0].nodes.at(-1));
});
