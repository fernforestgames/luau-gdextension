print("Starting Fibonacci sequence...")

local function fibonacci(n)
    if n <= 1 then return n end
    return fibonacci(n - 1) + fibonacci(n - 2)
end

for i = 1, 4 do
    print("fibonacci(" .. i .. ") = " .. fibonacci(i))
end

print("Done with Fibonacci sequence.")

print("\n--- Testing Math Types ---")

-- Test Vector2
local v1 = Vector2(3, 4)
local v2 = Vector2(1, 2)
print("v1 =", tostring(v1))
print("v2 =", tostring(v2))
print("v1 + v2 =", tostring(v1 + v2))
print("v1 - v2 =", tostring(v1 - v2))
print("v1 * 2 =", tostring(v1 * 2))
print("v1 / 2 =", tostring(v1 / 2))
print("-v1 =", tostring(-v1))
print("v1 == v2:", v1 == v2)
print("v1.x =", v1.x, ", v1.y =", v1.y)

-- Test property modification
v1.x = 10
v1.y = 20
print("After modification, v1 =", tostring(v1))

-- Test Vector3 (uses Luau's native vector type for performance)
local v3 = Vector3(1, 2, 3)
local v4 = Vector3(4, 5, 6)
print("\nv3 =", tostring(v3))
print("v4 =", tostring(v4))
print("v3 + v4 =", tostring(v3 + v4))
print("v3 - v4 =", tostring(v3 - v4))
print("v3 * 3 =", tostring(v3 * 3))
print("v3 / 2 =", tostring(v3 / 2))
print("-v3 =", tostring(-v3))
print("v3 == v4:", v3 == v4)
print("v3.x =", v3.x, ", v3.y =", v3.y, ", v3.z =", v3.z)

-- Test Luau's built-in vector library functions
print("\n-- Vector library functions:")
print("vector.magnitude(v3) =", vector.magnitude(v3))
print("vector.normalize(v3) =", tostring(vector.normalize(v3)))
print("vector.dot(v3, v4) =", vector.dot(v3, v4))
print("vector.cross(v3, v4) =", tostring(vector.cross(v3, v4)))
print("vector.floor(Vector3(1.7, 2.3, 3.9)) =", tostring(vector.floor(Vector3(1.7, 2.3, 3.9))))
print("vector.ceil(Vector3(1.2, 2.5, 3.1)) =", tostring(vector.ceil(Vector3(1.2, 2.5, 3.1))))
print("vector.abs(Vector3(-1, -2, 3)) =", tostring(vector.abs(Vector3(-1, -2, 3))))
print("vector.min(v3, v4) =", tostring(vector.min(v3, v4)))
print("vector.max(v3, v4) =", tostring(vector.max(v3, v4)))

-- Test Color
local c1 = Color(1.0, 0.5, 0.0, 1.0)
local c2 = Color(0.0, 0.5, 1.0, 1.0)
print("\nc1 =", tostring(c1))
print("c2 =", tostring(c2))
print("c1 + c2 =", tostring(c1 + c2))
print("c1 * 0.5 =", tostring(c1 * 0.5))
print("c1.r =", c1.r, ", c1.a =", c1.a)

-- Test Vector2i
local vi1 = Vector2i(10, 20)
local vi2 = Vector2i(5, 3)
print("\nvi1 =", tostring(vi1))
print("vi2 =", tostring(vi2))
print("vi1 + vi2 =", tostring(vi1 + vi2))
print("vi1 * 2 =", tostring(vi1 * 2))

print("\n--- Math Types Tests Complete ---")
