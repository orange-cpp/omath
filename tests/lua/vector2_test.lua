local a = omath.Vec2.new(1, 2)
local b = omath.Vec2.new(10, 20)

-- Operators
local c = a + b
local d = a - b
local e = a * 2.0
local f = -a
print("a + b = " .. tostring(c))
print("a - b = " .. tostring(d))
print("a * 2 = " .. tostring(e))
print("-a    = " .. tostring(f))
print("a == Vec2(1,2): " .. tostring(a == omath.Vec2.new(1, 2)))
print("a < b: " .. tostring(a < b))

-- Field access + mutation
print("c.x = " .. c.x .. ", c.y = " .. c.y)
c.x = 99
print("c.x after mutation = " .. c.x)

-- Methods
print("a:length()           = " .. a:length())
print("a:length_sqr()       = " .. a:length_sqr())
print("a:normalized()       = " .. tostring(a:normalized()))
print("a:dot(b)             = " .. a:dot(b))
print("a:distance_to(b)     = " .. a:distance_to(b))
print("a:distance_to_sqr(b) = " .. a:distance_to_sqr(b))
print("a:sum()              = " .. a:sum())
print("a:abs()              = " .. tostring(a:abs()))
