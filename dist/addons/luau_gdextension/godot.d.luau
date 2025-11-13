declare class Vector2
    x: number
    y: number
end
declare function Vector2(x: number, y: number): Vector2

declare class Vector2i
    x: number
    y: number
end
declare Vector2i: ((x: number, y: number) -> Vector2i) & ((vec: Vector2) -> Vector2i)

declare class Rect2
    position: Vector2
    size: Vector2
end
declare Rect2: ((position: Vector2, size: Vector2) -> Rect2) & ((x: number, y: number, width: number, height: number) -> Rect2)

declare class Rect2i
    position: Vector2i
    size: Vector2i
end
declare Rect2i: ((position: Vector2i, size: Vector2i) -> Rect2i) & ((x: number, y: number, width: number, height: number) -> Rect2i)

declare Vector3: ((x: number, y: number, z: number) -> vector)

declare class Vector3i
    x: number
    y: number
    z: number
end
declare Vector3i: ((x: number, y: number, z: number) -> Vector3i) & ((vec: vector) -> Vector3i)

declare class Transform2D
    origin: Vector2
    x: Vector2
    y: Vector2
end
declare Transform2D: ((x: Vector2, y: Vector2, origin: Vector2) -> Transform2D) & ((rotation: number, scale: Vector2, skew: number, position: Vector2) -> Transform2D) & ((rotation: number, position: Vector2) -> Transform2D)

declare class Vector4
    x: number
    y: number
    z: number
    w: number
end
declare function Vector4(x: number, y: number, z: number, w: number): Vector4

declare class Vector4i
    x: number
    y: number
    z: number
    w: number
end
declare Vector4i: ((x: number, y: number, z: number, w: number) -> Vector4i) & ((vec: Vector4) -> Vector4i)

declare class Plane
    d: number
    normal: vector
    x: number
    y: number
    z: number
end
declare Plane: ((a: number, b: number, c: number, d: number) -> Plane) & ((normal: vector, d: number) -> Plane) & ((normal: vector, point: vector) -> Plane) & ((point1: vector, point2: vector, point3: vector) -> Plane) & ((normal: vector) -> Plane)

declare class Quaternion
    x: number
    y: number
    z: number
    w: number
end
declare Quaternion: ((basis: Basis) -> Quaternion) & ((x: number, y: number, z: number, w: number) -> Quaternion) & ((axis: vector, angle: number) -> Quaternion) & ((arc_from: vector, arc_to: vector) -> Quaternion)

declare class AABB
    position: vector
    size: vector
end
declare AABB: ((position: vector, size: vector) -> AABB)

declare class Basis
    x: vector
    y: vector
    z: vector
end
declare Basis: ((quaternion: Quaternion) -> Basis) & ((x: vector, y: vector, z: vector) -> Basis) & ((axis: vector, angle: number) -> Basis)

declare class Transform3D
    basis: Basis
    origin: vector
end
declare Transform3D: ((projection: Projection) -> Transform3D) & ((basis: Basis, origin: vector) -> Transform3D) & ((x_axis: vector, y_axis: vector, z_axis: vector, origin: vector) -> Transform3D)

declare class Projection
    w: Vector4
    x: Vector4
    y: Vector4
    z: Vector4
end
declare Projection: ((transform: Transform3D) -> Projection) & ((x_axis: Vector4, y_axis: Vector4, z_axis: Vector4, w_axis: Vector4) -> Projection)

declare class Color
    a: number
    a8: number
    b: number
    b8: number
    g: number
    g8: number
    h: number
    ok_hsl_h: number
    ok_hsl_l: number
    ok_hsl_s: number
    r: number
    r8: number
    s: number
    v: number
end
declare Color: ((code: string, alpha: number?) -> Color) & ((base: Color, alpha: number) -> Color) & ((r: number, g: number, b: number, a: number?) -> Color)

declare class RID
end
declare RID: (() -> RID)

declare class Signal
end
declare Signal: ((object: unknown, signal: string) -> Signal)

declare PackedByteArray: ((array: {[number]: number}?) -> buffer)

declare class PackedInt32Array
end
declare PackedInt32Array: ((array: {[number]: number}?) -> PackedInt32Array)

declare class PackedInt64Array
end
declare PackedInt64Array: ((array: {[number]: number}?) -> PackedInt64Array)

declare class PackedFloat32Array
end
declare PackedFloat32Array: ((array: {[number]: number}?) -> PackedFloat32Array)

declare class PackedFloat64Array
end
declare PackedFloat64Array: ((array: {[number]: number}?) -> PackedFloat64Array)

declare class PackedStringArray
end
declare PackedStringArray: ((array: {[number]: string}?) -> PackedStringArray)

declare class PackedVector2Array
end
declare PackedVector2Array: ((array: {[number]: Vector2}?) -> PackedVector2Array)

declare class PackedVector3Array
end
declare PackedVector3Array: ((array: {[number]: vector}?) -> PackedVector3Array)

declare class PackedColorArray
end
declare PackedColorArray: ((array: {[number]: Color}?) -> PackedColorArray)

declare class PackedVector4Array
end
declare PackedVector4Array: ((array: {[number]: Vector4}?) -> PackedVector4Array)
