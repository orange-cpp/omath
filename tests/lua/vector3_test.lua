local a = omath.Vec3.new(1, 0, 0)
local b = omath.Vec3.new(0, 1, 0)

-- Operators
local c = a + b
local d = a - b
local e = a * 2.0
local f = -a
print("a + b = " .. tostring(c))
print("a - b = " .. tostring(d))
print("a * 2 = " .. tostring(e))
print("-a    = " .. tostring(f))
print("a == Vec3(1,2,3): " .. tostring(a == omath.Vec3.new(1, 2, 3)))
print("a < b: " .. tostring(a < b))

-- Field access + mutation
print("c.x = " .. c.x .. ", c.y = " .. c.y .. ", c.z = " .. c.z)
c.x = 99
print("c.x after mutation = " .. c.x)

-- Methods
print("a:length()       = " .. a:length())
print("a:length_2d()    = " .. a:length_2d())
print("a:length_sqr()   = " .. a:length_sqr())
print("a:normalized()   = " .. tostring(a:normalized()))
print("a:dot(b)         = " .. a:dot(b))
print("a:cross(b)       = " .. tostring(a:cross(b)))
print("a:distance_to(b) = " .. a:distance_to(b))
print("a:distance_to_sqr(b) = " .. a:distance_to_sqr(b))
print("a:abs()          = " .. tostring(a:abs()))
print("a:sum()          = " .. a:sum())
print("a:sum_2d()       = " .. a:sum_2d())
print("a:point_to_same_direction(b) = " .. tostring(a:point_to_same_direction(b)))
print("a:is_perpendicular(b) = " .. tostring(a:is_perpendicular(b)))

-- angle_between
local angle, err = a:angle_between(b)
if angle then
    print("angle_between = " .. angle .. " degrees")
else
    print("angle_between error: " .. err)
end

-- Zero vector edge case
local zero = omath.Vec3.new(0, 0, 0)
local ang2, err2 = zero:angle_between(a)
if ang2 then
    print("zero angle = " .. ang2)
else
    print("zero angle error: " .. err2)
end

-- as_table
local t = a:as_table()
print("as_table: x=" .. t.x .. " y=" .. t.y .. " z=" .. t.z)
