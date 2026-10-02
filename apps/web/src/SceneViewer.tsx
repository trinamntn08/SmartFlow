import { useEffect, useRef, useState } from 'react';
import type { JsonObject } from '@smartflow/core';
import { bounds, type SceneValue, type Vector } from '@smartflow/scene-3d';
const dot = (a: Vector, b: Vector) => a.reduce((sum, value, axis) => sum + value * b[axis]!, 0);
const sub = (a: Vector, b: Vector): Vector => [a[0] - b[0], a[1] - b[1], a[2] - b[2]];
const cross = (a: Vector, b: Vector): Vector => [
  a[1] * b[2] - a[2] * b[1],
  a[2] * b[0] - a[0] * b[2],
  a[0] * b[1] - a[1] * b[0],
];
const number = (state: JsonObject, key: string, fallback: number, low: number, high: number) =>
  typeof state[key] === 'number' &&
  Number.isFinite(state[key]) &&
  state[key] >= low &&
  state[key] <= high
    ? state[key]
    : fallback;
export function SceneViewer({
  scene,
  state,
  changed,
  editSource,
  editable,
}: {
  scene: SceneValue;
  state: JsonObject;
  changed(state: JsonObject): void;
  editSource(id: string): void;
  editable: boolean;
}) {
  const canvas = useRef<HTMLCanvasElement>(null);
  const [size, setSize] = useState({ width: 800, height: 300 });
  const drag = useRef<
    { x: number; y: number; yaw: number; pitch: number; moved: boolean } | undefined
  >(undefined);
  const picks = useRef<{ path: Path2D; object: number }[]>([]);
  const yaw = number(state, 'yaw', 35, -1e6, 1e6),
    pitch = number(state, 'pitch', 25, -85, 85),
    span = number(state, 'span', 6, 0.1, 1e6),
    selection = Math.trunc(number(state, 'selection', -1, -1, 63));
  const target =
    Array.isArray(state.target) &&
    state.target.length === 3 &&
    state.target.every(
      (value) => typeof value === 'number' && Number.isFinite(value) && Math.abs(value) <= 1e6,
    )
      ? (state.target as Vector)
      : ([0, 0, 0] as Vector);
  useEffect(() => {
    const element = canvas.current;
    if (!element) return;
    const wheel = (event: WheelEvent) => {
      event.preventDefault();
      changed({
        ...state,
        span: Math.max(0.1, Math.min(1e6, span * Math.exp(event.deltaY * 0.001))),
      });
    };
    element.addEventListener('wheel', wheel, { passive: false });
    return () => element.removeEventListener('wheel', wheel);
  }, [state, span, changed]);
  useEffect(() => {
    if (!canvas.current) return;
    const observer = new ResizeObserver((entries) => {
      const rect = entries[0]?.contentRect;
      if (rect) setSize({ width: Math.max(1, rect.width), height: Math.max(1, rect.height) });
    });
    observer.observe(canvas.current);
    return () => observer.disconnect();
  }, []);
  useEffect(() => {
    const context = canvas.current?.getContext('2d');
    if (!context || !canvas.current) return;
    const ratio = devicePixelRatio || 1;
    canvas.current.width = Math.round(size.width * ratio);
    canvas.current.height = Math.round(size.height * ratio);
    context.scale(ratio, ratio);
    context.fillStyle = '#1b2029';
    context.fillRect(0, 0, size.width, size.height);
    const y = (yaw * Math.PI) / 180,
      p = (pitch * Math.PI) / 180;
    const forward: Vector = [Math.sin(y) * Math.cos(p), Math.sin(p), Math.cos(y) * Math.cos(p)],
      right: Vector = [Math.cos(y), 0, -Math.sin(y)],
      up = cross(forward, right);
    const pixels = Math.min(size.width, size.height - 30) / span;
    const project = (v: Vector) => {
      const relative = sub(v, target);
      return [
        size.width / 2 + dot(relative, right) * pixels,
        size.height / 2 - dot(relative, up) * pixels,
      ] as const;
    };
    context.strokeStyle = '#303945';
    context.lineWidth = 1;
    for (let index = -5; index <= 5; index++)
      for (const [a, b] of [
        [
          [index, -1, -5],
          [index, -1, 5],
        ],
        [
          [-5, -1, index],
          [5, -1, index],
        ],
      ] as [Vector, Vector][]) {
        context.beginPath();
        context.moveTo(...project(a));
        context.lineTo(...project(b));
        context.stroke();
      }
    const faces: { path: Path2D; color: string; depth: number; object: number }[] = [];
    scene.objects.forEach((object, index) =>
      object.triangles.forEach((face) => {
        const [a, b, c] = face.map((vertex) => object.vertices[vertex]!) as [
            Vector,
            Vector,
            Vector,
          ],
          normal = cross(sub(b, a), sub(c, a)),
          length = Math.hypot(...normal);
        if (length < 1e-7 || dot(normal, forward) <= 0) return;
        const light =
          0.28 +
          0.72 *
            Math.max(
              0,
              dot(normal, [-1 / Math.sqrt(14), 2 / Math.sqrt(14), 3 / Math.sqrt(14)]) / length,
            );
        const path = new Path2D();
        path.moveTo(...project(a));
        path.lineTo(...project(b));
        path.lineTo(...project(c));
        path.closePath();
        const center = a.map((value, axis) => (value + b[axis]! + c[axis]!) / 3) as Vector;
        faces.push({
          path,
          color: `rgb(${object.color.map((value) => Math.round(value * light * 255)).join(',')})`,
          depth: dot(sub(center, target), forward),
          object: index,
        });
      }),
    );
    faces.sort((a, b) => a.depth - b.depth);
    picks.current = [];
    for (const face of faces) {
      context.fillStyle = face.color;
      context.fill(face.path);
      context.strokeStyle = face.object === selection ? '#ffcf5e' : face.color;
      context.lineWidth = face.object === selection ? 2 : 1;
      context.stroke(face.path);
      picks.current.push({ path: face.path, object: face.object });
    }
  }, [scene, size, yaw, pitch, span, selection, JSON.stringify(target)]);
  const frame = () => {
    const box = bounds(scene),
      target = box.minimum.map((value, axis) => (value + box.maximum[axis]!) / 2),
      diagonal = Math.hypot(...box.maximum.map((value, axis) => value - box.minimum[axis]!));
    changed({ ...state, target, span: Math.max(0.1, diagonal * 1.4) });
  };
  return (
    <div className="scene-viewer">
      <div className="scene-tools">
        <span>{scene.objects.length} object(s)</span>
        <button onClick={frame}>Frame scene</button>
        <button
          disabled={!editable || selection < 0 || !scene.objects[selection]}
          onClick={() => editSource(scene.objects[selection]!.id)}
        >
          Increase source size
        </button>
        <span>Drag to orbit · Wheel to zoom · Click to select</span>
      </div>
      <canvas
        ref={canvas}
        aria-label="3D scene preview"
        onDoubleClick={frame}
        onPointerDown={(event) => {
          event.currentTarget.setPointerCapture(event.pointerId);
          drag.current = { x: event.clientX, y: event.clientY, yaw, pitch, moved: false };
        }}
        onPointerMove={(event) => {
          const start = drag.current;
          if (!start) return;
          const dx = event.clientX - start.x,
            dy = event.clientY - start.y;
          if (Math.hypot(dx, dy) > 3) start.moved = true;
          if (start.moved)
            changed({
              ...state,
              yaw: Math.max(-1e6, Math.min(1e6, start.yaw + dx * 0.5)),
              pitch: Math.max(-85, Math.min(85, start.pitch + dy * 0.5)),
            });
        }}
        onPointerUp={(event) => {
          const start = drag.current;
          drag.current = undefined;
          if (!start || start.moved) return;
          const rect = event.currentTarget.getBoundingClientRect(),
            context = event.currentTarget.getContext('2d');
          const hit = [...picks.current]
            .reverse()
            .find((face) =>
              context?.isPointInPath(
                face.path,
                (event.clientX - rect.left) * devicePixelRatio,
                (event.clientY - rect.top) * devicePixelRatio,
              ),
            );
          changed({ ...state, selection: hit?.object ?? -1 });
        }}
        onPointerCancel={() => {
          drag.current = undefined;
        }}
      />
      <p className="small muted">
        Primitive preview · depth-sorted opaque faces · bounds{' '}
        {bounds(scene)
          .minimum.map((value) => value.toFixed(2))
          .join(', ')}{' '}
        to{' '}
        {bounds(scene)
          .maximum.map((value) => value.toFixed(2))
          .join(', ')}
      </p>
    </div>
  );
}
