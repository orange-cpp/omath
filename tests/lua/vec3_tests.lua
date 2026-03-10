local function approx(a, b, eps) return math.abs(a - b) < (eps or 1e-5) end

function Vec3_Constructor_default()
    local v = omath.Vec3.new()
    assert(v.x == 0 and v.y == 0 and v.z == 0)
end

function Vec3_Constructor_xyz()
    local v = omath.Vec3.new(1, 2, 3)
    assert(v.x == 1 and v.y == 2 and v.z == 3)
end

function Vec3_Field_mutation()
    local v = omath.Vec3.new(1, 2, 3)
    v.x = 9; v.y = 8; v.z = 7
    assert(v.x == 9 and v.y == 8 and v.z == 7)
end

function Vec3_Addition()
    local c = omath.Vec3.new(1, 2, 3) + omath.Vec3.new(4, 5, 6)
    assert(c.x == 5 and c.y == 7 and c.z == 9)
end

function Vec3_Subtraction()
    local c = omath.Vec3.new(4, 5, 6) - omath.Vec3.new(1, 2, 3)
    assert(c.x == 3 and c.y == 3 and c.z == 3)
end

function Vec3_UnaryMinus()
    local b = -omath.Vec3.new(1, 2, 3)
    assert(b.x == -1 and b.y == -2 and b.z == -3)
end

function Vec3_Multiplication_scalar()
    local b = omath.Vec3.new(1, 2, 3) * 2.0
    assert(b.x == 2 and b.y == 4 and b.z == 6)
end

function Vec3_Multiplication_scalar_reversed()
    local b = 2.0 * omath.Vec3.new(1, 2, 3)
    assert(b.x == 2 and b.y == 4 and b.z == 6)
end

function Vec3_Multiplication_vec()
    local c = omath.Vec3.new(2, 3, 4) * omath.Vec3.new(2, 2, 2)
    assert(c.x == 4 and c.y == 6 and c.z == 8)
end

function Vec3_Division_scalar()
    local b = omath.Vec3.new(2, 4, 6) / 2.0
    assert(b.x == 1 and b.y == 2 and b.z == 3)
end

function Vec3_Division_vec()
    local c = omath.Vec3.new(4, 6, 8) / omath.Vec3.new(2, 2, 2)
    assert(c.x == 2 and c.y == 3 and c.z == 4)
end

function Vec3_EqualTo_true()
    assert(omath.Vec3.new(1, 2, 3) == omath.Vec3.new(1, 2, 3))
end

function Vec3_EqualTo_false()
    assert(not (omath.Vec3.new(1, 2, 3) == omath.Vec3.new(9, 9, 9)))
end

function Vec3_LessThan()
    assert(omath.Vec3.new(1, 0, 0) < omath.Vec3.new(3, 4, 0))
end

function Vec3_LessThanOrEqual()
    -- (0,3,4) and (0,4,3) both have length 5
    assert(omath.Vec3.new(0, 3, 4) <= omath.Vec3.new(0, 4, 3))
end

function Vec3_ToString()
    assert(tostring(omath.Vec3.new(1, 2, 3)) == "Vec3(1, 2, 3)")
end

function Vec3_Length()
    assert(approx(omath.Vec3.new(1, 2, 2):length(), 3.0))
end

function Vec3_Length2d()
    assert(approx(omath.Vec3.new(3, 4, 99):length_2d(), 5.0))
end

function Vec3_LengthSqr()
    assert(omath.Vec3.new(1, 2, 2):length_sqr() == 9.0)
end

function Vec3_Normalized()
    local n = omath.Vec3.new(3, 0, 0):normalized()
    assert(approx(n.x, 1.0) and approx(n.y, 0.0) and approx(n.z, 0.0))
end

function Vec3_Dot_perpendicular()
    assert(omath.Vec3.new(1, 0, 0):dot(omath.Vec3.new(0, 1, 0)) == 0.0)
end

function Vec3_Dot_parallel()
    local a = omath.Vec3.new(1, 2, 3)
    assert(a:dot(a) == 14.0)
end

function Vec3_Cross()
    local c = omath.Vec3.new(1, 0, 0):cross(omath.Vec3.new(0, 1, 0))
    assert(approx(c.x, 0) and approx(c.y, 0) and approx(c.z, 1))
end

function Vec3_DistanceTo()
    assert(approx(omath.Vec3.new(0, 0, 0):distance_to(omath.Vec3.new(1, 2, 2)), 3.0))
end

function Vec3_DistanceToSqr()
    assert(omath.Vec3.new(0, 0, 0):distance_to_sqr(omath.Vec3.new(1, 2, 2)) == 9.0)
end

function Vec3_Sum()
    assert(omath.Vec3.new(1, 2, 3):sum() == 6.0)
end

function Vec3_Sum2d()
    assert(omath.Vec3.new(1, 2, 3):sum_2d() == 3.0)
end

function Vec3_Abs()
    local a = omath.Vec3.new(-1, -2, -3):abs()
    assert(a.x == 1 and a.y == 2 and a.z == 3)
end

function Vec3_PointToSameDirection_true()
    assert(omath.Vec3.new(1, 1, 0):point_to_same_direction(omath.Vec3.new(2, 2, 0)) == true)
end

function Vec3_PointToSameDirection_false()
    assert(omath.Vec3.new(1, 0, 0):point_to_same_direction(omath.Vec3.new(-1, 0, 0)) == false)
end

function Vec3_IsPerpendicular_true()
    assert(omath.Vec3.new(1, 0, 0):is_perpendicular(omath.Vec3.new(0, 1, 0)) == true)
end

function Vec3_IsPerpendicular_false()
    local a = omath.Vec3.new(1, 0, 0)
    assert(a:is_perpendicular(a) == false)
end

function Vec3_AngleBetween_90deg()
    local angle, err = omath.Vec3.new(1, 0, 0):angle_between(omath.Vec3.new(0, 1, 0))
    assert(angle ~= nil, err)
    assert(math.abs(angle - 90.0) < 1e-3)
end

function Vec3_AngleBetween_zero_vector_error()
    local angle, err = omath.Vec3.new(0, 0, 0):angle_between(omath.Vec3.new(1, 0, 0))
    assert(angle == nil and err ~= nil)
end

function Vec3_AsTable()
    local t = omath.Vec3.new(1, 2, 3):as_table()
    assert(t.x == 1 and t.y == 2 and t.z == 3)
end
