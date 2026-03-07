local a = omath.Vec4.new(1, 2, 3, 4)
local b = omath.Vec4.new(10, 20, 30, 40)

-- Operators
local c = a + b
local d = a - b
local e = a * 2.0
local f = -a
print("a + b = " .. tostring(c))
print("a - b = " .. tostring(d))
print("a * 2 = " .. tostring(e))
print("-a    = " .. tostring(f))
print("a == Vec4(1,2,3,4): " .. tostring(a == omath.Vec4.new(1, 2, 3, 4)))
print("a < b: " .. tostring(a < b))

-- Field access + mutation
print("c.x=" .. c.x .. " c.y=" .. c.y .. " c.z=" .. c.z .. " c.w=" .. c.w)
c.w = 99
print("c.w after mutation = " .. c.w)

-- Methods
print("a:length()     = " .. a:length())
print("a:length_sqr() = " .. a:length_sqr())
print("a:dot(b)       = " .. a:dot(b))
print("a:sum()        = " .. a:sum())
print("a:abs()        = " .. tostring(a:abs()))

-- clamp
local clamped = omath.Vec4.new(5, -3, 10, 1)
clamped:clamp(0, 7)
print("clamp([5,-3,10,1], 0, 7).x=" .. clamped.x .. " .y=" .. clamped.y .. " .z=" .. clamped.z)
