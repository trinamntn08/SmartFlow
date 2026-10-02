import { MAXIMUM_FILE_BYTES, parseProject, type ProjectFile } from '@smartflow/core';

/** Decode original file bytes strictly; invalid UTF-8 must never become replacement characters. */
export function parseProjectBytes(bytes: Uint8Array): ProjectFile {
  if (bytes.byteLength > MAXIMUM_FILE_BYTES) throw new Error('JSON exceeds 16 MiB file limit');
  return parseProject(new TextDecoder('utf-8', { fatal: true, ignoreBOM: true }).decode(bytes));
}
