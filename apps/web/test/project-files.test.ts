import assert from 'node:assert/strict';
import test from 'node:test';
import { readFileSync } from 'node:fs';
import { createProject, parseProject, serializeProject } from '@smartflow/core';
import { parseProjectBytes } from '../src/project-files.ts';

test('byte transport rejects invalid UTF-8 and accepts one leading BOM', () => {
  const input = Buffer.from(serializeProject(createProject('bytes')));
  assert.deepEqual(
    parseProjectBytes(Buffer.concat([Buffer.from([0xef, 0xbb, 0xbf]), input])),
    parseProject(input.toString()),
  );
  const invalid = Buffer.concat([
    Buffer.from(input.toString().replace('"workspace": {}', '"workspace": {"bad": "').slice(0, -2)),
    Buffer.from([0xc3, 0x28]),
    Buffer.from('"}}'),
  ]);
  // Replacement decoding would produce valid JSON; fatal decoding must reject it.
  assert.doesNotThrow(() => parseProject(invalid.toString()));
  assert.throws(() => parseProjectBytes(invalid), /encoded data/);
  assert.throws(() =>
    parseProjectBytes(Buffer.concat([Buffer.from([0xef, 0xbb, 0xbf, 0xef, 0xbb, 0xbf]), input])),
  );
});

test('byte transport uses shared exact size boundaries', () => {
  const fixtures = JSON.parse(
    readFileSync(new URL('../../../tests/fixtures/transport-v1.json', import.meta.url), 'utf8'),
  ) as { sizes: { bytes: number; accepted: boolean }[] };
  const prefix = serializeProject(createProject('é😀'));
  for (const { bytes, accepted } of fixtures.sizes) {
    const input = Buffer.from(prefix + ' '.repeat(bytes - Buffer.byteLength(prefix)));
    if (accepted) assert.doesNotThrow(() => parseProjectBytes(input));
    else assert.throws(() => parseProjectBytes(input), /16 MiB/);
  }
});

test('byte adapter reopens each shipped native workflow without changing its content', () => {
  for (const name of ['scene', 'data', 'components']) {
    const input = readFileSync(new URL(`../../../examples/${name}.smartflow`, import.meta.url));
    assert.deepEqual(
      parseProject(serializeProject(parseProjectBytes(input))),
      JSON.parse(input.toString()),
    );
  }
});
