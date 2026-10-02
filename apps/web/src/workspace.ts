import {
  cloneJson,
  type JsonObject,
  type JsonValue,
  type ProjectFile,
  type ProjectHistory,
} from '@smartflow/core';
export const WEB_WORKSPACE = 'smartflow.web-editor@1';
export function record(value: JsonValue | undefined): JsonObject {
  return value && typeof value === 'object' && !Array.isArray(value) ? value : {};
}
export interface Point extends JsonObject {
  x: number;
  y: number;
}
export interface GraphWorkspace {
  positions: Record<string, Point>;
  selection: string[];
  viewport?: { x: number; y: number; zoom: number };
  nativeCenter?: { x: number; y: number; scale: number };
  pinned?: string;
  pinnedPort?: string;
  viewers: JsonObject;
}
export function readWorkspace(file: ProjectFile, graphId: string): GraphWorkspace {
  const web = record(file.workspace[WEB_WORKSPACE]);
  const browser = Object.hasOwn(web, graphId) ? record(web[graphId]) : undefined;
  const native = record(record(file.workspace['smartflow.native-editor@1'])[graphId]);
  const state = browser ?? native;
  const positions: Record<string, Point> = Object.create(null) as Record<string, Point>;
  for (const [id, value] of Object.entries(record(state.positions))) {
    const point = record(value);
    if (
      typeof point.x === 'number' &&
      typeof point.y === 'number' &&
      Number.isFinite(point.x) &&
      Number.isFinite(point.y) &&
      Math.abs(point.x) <= 1e6 &&
      Math.abs(point.y) <= 1e6
    )
      positions[id] = { x: point.x, y: point.y };
  }
  const result: GraphWorkspace = {
    positions,
    selection: Array.isArray(state.selection)
      ? state.selection.filter((id): id is string => typeof id === 'string')
      : [],
    viewers: cloneJson(record(browser?.viewers)),
  };
  if (typeof state.pinned === 'string') result.pinned = state.pinned;
  if (typeof state.pinnedPort === 'string') result.pinnedPort = state.pinnedPort;
  const viewport = record(state.viewport);
  if (
    typeof viewport.x === 'number' &&
    typeof viewport.y === 'number' &&
    typeof viewport.zoom === 'number' &&
    [viewport.x, viewport.y, viewport.zoom].every(Number.isFinite) &&
    viewport.zoom >= 0.01 &&
    viewport.zoom <= 2
  )
    result.viewport = { x: viewport.x, y: viewport.y, zoom: viewport.zoom };
  if (!browser) {
    const navigation = record(native.navigation);
    if (
      typeof navigation.x === 'number' &&
      typeof navigation.y === 'number' &&
      typeof navigation.scale === 'number' &&
      [navigation.x, navigation.y, navigation.scale].every(Number.isFinite) &&
      navigation.scale >= 0.01 &&
      navigation.scale <= 2
    )
      result.nativeCenter = { x: navigation.x, y: navigation.y, scale: navigation.scale };
  }
  return result;
}
export function patchWorkspace(history: ProjectHistory, graphId: string, patch: JsonObject): void {
  history.updateWorkspace((workspace) => {
    writeWorkspace({ ...history.snapshot, workspace }, graphId, patch);
  });
}
export function writeWorkspace(file: ProjectFile, graphId: string, patch: JsonObject): void {
  const workspace = file.workspace;
  const raw = workspace[WEB_WORKSPACE];
  if (raw !== undefined && (raw === null || typeof raw !== 'object' || Array.isArray(raw)))
    throw new Error('Unsupported browser workspace content');
  const root = record(raw);
  const previous = Object.hasOwn(root, graphId) ? root[graphId] : undefined;
  if (
    previous !== undefined &&
    (previous === null || typeof previous !== 'object' || Array.isArray(previous))
  )
    throw new Error('Unsupported graph workspace content');
  const initial =
    previous ??
    (() => {
      const state = readWorkspace(file, graphId);
      return {
        positions: state.positions,
        selection: state.selection,
        ...(state.pinned ? { pinned: state.pinned } : {}),
        ...(state.pinnedPort ? { pinnedPort: state.pinnedPort } : {}),
      };
    })();
  Object.defineProperty(root, graphId, {
    value: { ...record(initial as JsonValue), ...cloneJson(patch) },
    enumerable: true,
    configurable: true,
    writable: true,
  });
  workspace[WEB_WORKSPACE] = root;
}
