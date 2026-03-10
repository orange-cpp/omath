local function approx(a, b, eps) return math.abs(a - b) < (eps or 1e-5) end

function Triangle_Constructor_default()
    local t = omath.Triangle.new()
    assert(t.vertex1.x == 0 and t.vertex1.y == 0 and t.vertex1.z == 0)
    assert(t.vertex2.x == 0 and t.vertex2.y == 0 and t.vertex2.z == 0)
    assert(t.vertex3.x == 0 and t.vertex3.y == 0 and t.vertex3.z == 0)
end

function Triangle_Constructor_vertices()
    local v1 = omath.Vec3.new(1, 0, 0)
    local v2 = omath.Vec3.new(0, 1, 0)
    local v3 = omath.Vec3.new(0, 0, 1)
    local t = omath.Triangle.new(v1, v2, v3)
    assert(t.vertex1.x == 1 and t.vertex1.y == 0 and t.vertex1.z == 0)
    assert(t.vertex2.x == 0 and t.vertex2.y == 1 and t.vertex2.z == 0)
    assert(t.vertex3.x == 0 and t.vertex3.y == 0 and t.vertex3.z == 1)
end

function Triangle_Vertex_mutation()
    local t = omath.Triangle.new()
    t.vertex1 = omath.Vec3.new(5, 6, 7)
    assert(t.vertex1.x == 5 and t.vertex1.y == 6 and t.vertex1.z == 7)
end

-- Right triangle: v1=(0,3,0), v2=(0,0,0), v3=(4,0,0) — sides 3, 4, hypot 5
function Triangle_SideALength()
    local t = omath.Triangle.new(omath.Vec3.new(0, 3, 0), omath.Vec3.new(0, 0, 0), omath.Vec3.new(4, 0, 0))
    assert(approx(t:side_a_length(), 3.0))
end

function Triangle_SideBLength()
    local t = omath.Triangle.new(omath.Vec3.new(0, 3, 0), omath.Vec3.new(0, 0, 0), omath.Vec3.new(4, 0, 0))
    assert(approx(t:side_b_length(), 4.0))
end

function Triangle_Hypot()
    local t = omath.Triangle.new(omath.Vec3.new(0, 3, 0), omath.Vec3.new(0, 0, 0), omath.Vec3.new(4, 0, 0))
    assert(approx(t:hypot(), 5.0))
end

function Triangle_SideAVector()
    local t = omath.Triangle.new(omath.Vec3.new(0, 3, 0), omath.Vec3.new(0, 0, 0), omath.Vec3.new(4, 0, 0))
    local a = t:side_a_vector()
    assert(approx(a.x, 0) and approx(a.y, 3) and approx(a.z, 0))
end

function Triangle_SideBVector()
    local t = omath.Triangle.new(omath.Vec3.new(0, 3, 0), omath.Vec3.new(0, 0, 0), omath.Vec3.new(4, 0, 0))
    local b = t:side_b_vector()
    assert(approx(b.x, 4) and approx(b.y, 0) and approx(b.z, 0))
end

function Triangle_IsRectangular_true()
    local t = omath.Triangle.new(omath.Vec3.new(0, 3, 0), omath.Vec3.new(0, 0, 0), omath.Vec3.new(4, 0, 0))
    assert(t:is_rectangular() == true)
end

function Triangle_IsRectangular_false()
    -- equilateral-ish triangle, not rectangular
    local t = omath.Triangle.new(omath.Vec3.new(0, 1, 0), omath.Vec3.new(-1, 0, 0), omath.Vec3.new(1, 0, 0))
    assert(t:is_rectangular() == false)
end

function Triangle_MidPoint()
    local t = omath.Triangle.new(omath.Vec3.new(3, 0, 0), omath.Vec3.new(0, 3, 0), omath.Vec3.new(0, 0, 3))
    local m = t:mid_point()
    assert(approx(m.x, 1.0) and approx(m.y, 1.0) and approx(m.z, 1.0))
end

function Triangle_CalculateNormal()
    -- flat triangle in XY plane — normal should be (0, 0, 1)
    local t = omath.Triangle.new(omath.Vec3.new(0, 1, 0), omath.Vec3.new(0, 0, 0), omath.Vec3.new(1, 0, 0))
    local n = t:calculate_normal()
    assert(approx(n.x, 0) and approx(n.y, 0) and approx(n.z, 1))
end

function Triangle_ToString()
    local t = omath.Triangle.new(omath.Vec3.new(1, 0, 0), omath.Vec3.new(0, 1, 0), omath.Vec3.new(0, 0, 1))
    local s = tostring(t)
    assert(s == "Triangle((1, 0, 0), (0, 1, 0), (0, 0, 1))")
end
