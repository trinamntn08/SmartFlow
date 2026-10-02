import { spawn } from 'node:child_process';
import { resolve } from 'node:path';
const args = process.argv.slice(2);
const production = args[0] === '--production';
if (production) args.shift();
const child = spawn(process.execPath, [resolve('node_modules/@playwright/test/cli.js'), ...args], {
  stdio: 'inherit',
  env: {
    ...process.env,
    ...(production ? { SMARTFLOW_WEB_PRODUCTION: '1' } : {}),
    PLAYWRIGHT_BROWSERS_PATH: resolve('.cache/playwright'),
  },
});
child.on('error', (error) => {
  console.error(error.message);
  process.exitCode = 1;
});
child.on('exit', (code) => {
  process.exitCode = code ?? 1;
});
