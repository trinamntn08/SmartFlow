export type Vector = [number, number, number];
export interface SceneObject {
  id: string;
  vertices: Vector[];
  triangles: [number, number, number][];
  color: Vector;
}
export interface SceneValue {
  objects: SceneObject[];
}
const vector = (value: unknown): value is Vector =>
  Array.isArray(value) &&
  value.length === 3 &&
  value.every(
    (item) => typeof item === 'number' && Number.isFinite(item) && Math.abs(item) <= 100000,
  );
export function isScene(value: unknown): value is SceneValue {
  if (
    !value ||
    typeof value !== 'object' ||
    !('objects' in value) ||
    !Array.isArray(value.objects) ||
    value.objects.length > 64
  )
    return false;
  return value.objects.every((object: unknown) => {
    if (
      !object ||
      typeof object !== 'object' ||
      !('id' in object) ||
      typeof object.id !== 'string' ||
      !('vertices' in object) ||
      !Array.isArray(object.vertices) ||
      object.vertices.length > 36 ||
      !object.vertices.every(vector) ||
      !('triangles' in object) ||
      !Array.isArray(object.triangles) ||
      object.triangles.length > 12 ||
      !('color' in object) ||
      !vector(object.color) ||
      !object.color.every((value) => value >= 0 && value <= 1)
    )
      return false;
    const count = object.vertices.length;
    return object.triangles.every(
      (face: unknown) =>
        Array.isArray(face) &&
        face.length === 3 &&
        face.every((index) => Number.isInteger(index) && index >= 0 && index < count),
    );
  });
}
export function cube(id: string, size: number): SceneValue {
  const h = size / 2;
  return {
    objects: [
      {
        id,
        color: [0.65, 0.68, 0.72],
        vertices: [
          [-h, -h, -h],
          [h, -h, -h],
          [h, h, -h],
          [-h, h, -h],
          [-h, -h, h],
          [h, -h, h],
          [h, h, h],
          [-h, h, h],
        ],
        triangles: [
          [0, 2, 1],
          [0, 3, 2],
          [4, 5, 6],
          [4, 6, 7],
          [0, 1, 5],
          [0, 5, 4],
          [3, 7, 6],
          [3, 6, 2],
          [0, 4, 7],
          [0, 7, 3],
          [1, 2, 6],
          [1, 6, 5],
        ],
      },
    ],
  };
}
export function bounds(scene: SceneValue): { minimum: Vector; maximum: Vector } {
  const vertices = scene.objects.flatMap((object) => object.vertices);
  return {
    minimum: [0, 1, 2].map((axis) =>
      vertices.length ? Math.min(...vertices.map((vertex) => vertex[axis]!)) : 0,
    ) as Vector,
    maximum: [0, 1, 2].map((axis) =>
      vertices.length ? Math.max(...vertices.map((vertex) => vertex[axis]!)) : 0,
    ) as Vector,
  };
}
