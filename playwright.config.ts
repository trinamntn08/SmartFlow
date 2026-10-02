import { defineConfig } from '@playwright/test';
const production = process.env.SMARTFLOW_WEB_PRODUCTION === '1';
const npm = process.platform === 'win32' ? 'npm.cmd' : 'npm';
const baseURL = `http://127.0.0.1:${production ? 4173 : 5173}`;
export default defineConfig({
  testDir: './apps/web/e2e',
  outputDir: './build/web-tests/results',
  workers: 1,
  timeout: 30000,
  reporter: [['list'], ['json', { outputFile: 'build/web-tests/report.json' }]],
  use: {
    baseURL,
    viewport: { width: 1440, height: 900 },
    screenshot: 'only-on-failure',
    trace: 'retain-on-failure',
    acceptDownloads: true,
  },
  projects: [{ name: 'chromium', use: { browserName: 'chromium' } }],
  webServer: {
    command: production
      ? `${npm} run preview --workspace @smartflow/web -- --host 127.0.0.1 --port 4173 --strictPort`
      : `${npm} run dev`,
    url: baseURL,
    reuseExistingServer: !production && !process.env.CI,
  },
});
