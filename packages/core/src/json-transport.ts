/** Native-compatible transport limits, measured before constructing a JSON document. */
export const MAXIMUM_FILE_BYTES = 16 * 1024 * 1024;
export const MAXIMUM_NESTING = 128;

export function assertUnicode(text: string): void {
  for (let index = 0; index < text.length; index++) {
    const code = text.charCodeAt(index);
    if (code >= 0xd800 && code <= 0xdbff) {
      const next = text.charCodeAt(++index);
      if (!(next >= 0xdc00 && next <= 0xdfff))
        throw new Error('JSON contains an unpaired Unicode surrogate');
    } else if (code >= 0xdc00 && code <= 0xdfff) {
      throw new Error('JSON contains an unpaired Unicode surrogate');
    }
  }
}

export function assertNumber(number: number): void {
  if (!Number.isFinite(number)) throw new Error('Project contains a non-JSON number');
  if (Object.is(number, -0))
    throw new Error('JSON negative zero cannot be preserved by browser serialization');
  if (Number.isInteger(number) && !Number.isSafeInteger(number))
    throw new Error('JSON number is outside the browser safe integer range');
}

export function assertFileSize(text: string): void {
  let bytes = 0;
  for (let index = 0; index < text.length; index++) {
    const code = text.charCodeAt(index);
    if (code >= 0xd800 && code <= 0xdbff) {
      bytes += 4;
      index++;
    } else bytes += code < 0x80 ? 1 : code < 0x800 ? 2 : 3;
    if (bytes > MAXIMUM_FILE_BYTES) throw new Error('JSON exceeds 16 MiB file limit');
  }
}

/** Reject duplicate decoded keys and unsupported values before JSON.parse can lose them. */
export function parseJson(text: string): unknown {
  assertUnicode(text);
  assertFileSize(text);
  // Native nlohmann transport accepts one leading UTF-8 BOM.
  if (text.startsWith('\uFEFF')) text = text.slice(1);
  let position = 0;
  const invalid = (): never => {
    throw new Error(`Invalid JSON at character ${position}`);
  };
  const whitespace = () => {
    while (position < text.length) {
      const code = text.charCodeAt(position);
      if (code !== 32 && code !== 9 && code !== 13 && code !== 10) break;
      position++;
    }
  };
  const string = (): string => {
    const start = position++;
    while (position < text.length) {
      const character = text[position++];
      if (character === '\\') position++;
      else if (character === '"') {
        const decoded: unknown = JSON.parse(text.slice(start, position));
        if (typeof decoded !== 'string') invalid();
        assertUnicode(decoded as string);
        return decoded as string;
      }
    }
    return invalid();
  };
  const value = (depth: number): void => {
    whitespace();
    const character = text[position];
    if (character === '{' || character === '[') {
      if (depth >= MAXIMUM_NESTING) throw new Error('JSON exceeds nesting limit');
      position++;
      whitespace();
      const end = character === '{' ? '}' : ']';
      const keys = new Set<string>();
      if (text[position] === end) {
        position++;
        return;
      }
      for (;;) {
        if (character === '{') {
          if (text[position] !== '"') invalid();
          const key = string();
          if (keys.has(key)) throw new Error(`Duplicate JSON key: ${key}`);
          keys.add(key);
          whitespace();
          if (text[position++] !== ':') invalid();
        }
        value(depth + 1);
        whitespace();
        if (text[position] === end) {
          position++;
          return;
        }
        if (text[position++] !== ',') invalid();
        whitespace();
      }
    }
    if (character === '"') {
      string();
      return;
    }
    for (const literal of ['true', 'false', 'null']) {
      if (text.startsWith(literal, position)) {
        position += literal.length;
        return;
      }
    }
    const token = /^-?(?:0|[1-9]\d*)(?:\.\d+)?(?:[eE][+-]?\d+)?/.exec(text.slice(position))?.[0];
    if (!token) invalid();
    assertNumber(Number(token));
    position += token!.length;
  };
  value(0);
  whitespace();
  if (position !== text.length) invalid();
  return JSON.parse(text);
}
