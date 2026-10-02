import { defineConfig } from '@playwright/test';
export default defineConfig({
  testDir: './apps/web/e2e',
  outputDir: './build/web-tests/results',
  workers: 1,
  timeout: 30000,
  reporter: [['list'], ['json', { outputFile: 'build/web-tests/report.json' }]],
  use: {
    baseURL: 'http://127.0.0.1:5173',
    viewport: { width: 1440, height: 900 },
    screenshot: 'only-on-failure',
    trace: 'retain-on-failure',
    acceptDownloads: true,
  },
  projects: [{ name: 'chromium', use: { browserName: 'chromium' } }],
  webServer: {
    command: 'npm.cmd run dev',
    url: 'http://127.0.0.1:5173',
    reuseExistingServer: !process.env.CI,
  },
});
