import assert from 'node:assert/strict';
import test from 'node:test';
import { readFileSync } from 'node:fs';
import { createProject, parseProject, serializeProject } from '../src/index.ts';

const fixtures = JSON.parse(
  readFileSync(new URL('../../../tests/fixtures/transport-v1.json', import.meta.url), 'utf8'),
) as {
  cases: { name: string; json: string; native: boolean; browser: boolean }[];
  names: { id: string; accepted: boolean }[];
  depths: { arrays: number; accepted: boolean }[];
  sizes: { bytes: number; accepted: boolean }[];
};
const envelope = (payload: string) =>
  `{"format":"smartflow","schemaVersion":1,"project":{"id":"transport","graphs":[],"packages":[],"assets":[]},"workspace":{},"opaque":${payload}}`;

for (const fixture of fixtures.cases) {
  test(`shared transport: ${fixture.name}`, () => {
    const input = envelope(fixture.json);
    if (!fixture.browser) assert.throws(() => parseProject(input));
    else {
      const document = parseProject(input);
      assert.deepEqual(parseProject(serializeProject(document)), JSON.parse(input));
    }
  });
}
test('shared container depth boundaries apply on import and export', () => {
  for (const { arrays, accepted } of fixtures.depths) {
    const input = envelope('['.repeat(arrays) + '0' + ']'.repeat(arrays));
    if (accepted)
      assert.deepEqual(parseProject(serializeProject(parseProject(input))), JSON.parse(input));
    else {
      assert.throws(() => parseProject(input), /nesting/);
      assert.throws(() => serializeProject(JSON.parse(input)), /nesting/);
    }
  }
});
test('project identities match native Unicode whitespace rules', () => {
  for (const { id, accepted } of fixtures.names) {
    if (accepted) assert.equal(parseProject(serializeProject(createProject(id))).project.id, id);
    else assert.throws(() => createProject(id));
  }
});
test('shared UTF-8 size boundaries include multibyte text', () => {
  for (const { bytes, accepted } of fixtures.sizes) {
    const prefix = envelope('"é😀"');
    const input = prefix + ' '.repeat(bytes - Buffer.byteLength(prefix));
    assert.equal(Buffer.byteLength(input), bytes);
    if (accepted) assert.doesNotThrow(() => parseProject(input));
    else assert.throws(() => parseProject(input), /16 MiB/);
    const file = createProject('export');
    file.opaque = 'é😀';
    file.opaque += 'x'.repeat(bytes - Buffer.byteLength(serializeProject(file)));
    if (accepted) assert.equal(Buffer.byteLength(serializeProject(file)), bytes);
    else assert.throws(() => serializeProject(file), /16 MiB/);
  }
  const file = createProject('size');
  file.opaque = 'é'.repeat(8 * 1024 * 1024);
  assert.throws(() => serializeProject(file), /16 MiB/);
});
test('unsafe in-memory integers and Unicode reject before export', () => {
  for (const value of [9007199254740992, 1e100, '\ud800']) {
    const file = createProject('invalid');
    file.opaque = value;
    assert.throws(() => serializeProject(file));
  }
});
test('all shipped projects preserve opaque component and native workspace fields', () => {
  for (const name of ['scene', 'data', 'components']) {
    const input = readFileSync(new URL(`../../../examples/${name}.smartflow`, import.meta.url));
    assert.deepEqual(
      parseProject(serializeProject(parseProject(input.toString()))),
      JSON.parse(input.toString()),
    );
  }
});
