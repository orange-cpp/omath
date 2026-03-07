local function approx(a, b) return math.abs(a - b) < 1e-5 end

function Vec2_Constructor_default()
    local v = omath.Vec2.new()
    assert(v.x == 0 and v.y == 0)
end

function Vec2_Constructor_xy()
    local v = omath.Vec2.new(3, 4)
    assert(v.x == 3 and v.y == 4)
end

function Vec2_Field_mutation()
    local v = omath.Vec2.new(1, 2)
    v.x = 9; v.y = 8
    assert(v.x == 9 and v.y == 8)
end

function Vec2_Addition()
    local c = omath.Vec2.new(1, 2) + omath.Vec2.new(3, 4)
    assert(c.x == 4 and c.y == 6)
end

function Vec2_Subtraction()
    local c = omath.Vec2.new(5, 7) - omath.Vec2.new(2, 3)
    assert(c.x == 3 and c.y == 4)
end

function Vec2_UnaryMinus()
    local b = -omath.Vec2.new(1, 2)
    assert(b.x == -1 and b.y == -2)
end

function Vec2_Multiplication_scalar()
    local b = omath.Vec2.new(2, 3) * 2.0
    assert(b.x == 4 and b.y == 6)
end

function Vec2_Multiplication_scalar_reversed()
    local b = 2.0 * omath.Vec2.new(2, 3)
    assert(b.x == 4 and b.y == 6)
end

function Vec2_Division_scalar()
    local b = omath.Vec2.new(4, 6) / 2.0
    assert(b.x == 2 and b.y == 3)
end

function Vec2_EqualTo_true()
    assert(omath.Vec2.new(1, 2) == omath.Vec2.new(1, 2))
end

function Vec2_EqualTo_false()
    assert(not (omath.Vec2.new(1, 2) == omath.Vec2.new(9, 9)))
end

function Vec2_LessThan()
    assert(omath.Vec2.new(1, 0) < omath.Vec2.new(3, 4))
end

function Vec2_LessThanOrEqual()
    -- (3,4) and (4,3) both have length 5
    assert(omath.Vec2.new(3, 4) <= omath.Vec2.new(4, 3))
end

function Vec2_ToString()
    assert(tostring(omath.Vec2.new(1, 2)) == "Vec2(1, 2)")
end

function Vec2_Length()
    assert(approx(omath.Vec2.new(3, 4):length(), 5.0))
end

function Vec2_LengthSqr()
    assert(omath.Vec2.new(3, 4):length_sqr() == 25.0)
end

function Vec2_Normalized()
    local n = omath.Vec2.new(3, 4):normalized()
    assert(approx(n.x, 0.6) and approx(n.y, 0.8))
end

function Vec2_Dot()
    assert(omath.Vec2.new(1, 2):dot(omath.Vec2.new(3, 4)) == 11.0)
end

function Vec2_DistanceTo()
    assert(approx(omath.Vec2.new(0, 0):distance_to(omath.Vec2.new(3, 4)), 5.0))
end

function Vec2_DistanceToSqr()
    assert(omath.Vec2.new(0, 0):distance_to_sqr(omath.Vec2.new(3, 4)) == 25.0)
end

function Vec2_Sum()
    assert(omath.Vec2.new(3, 4):sum() == 7.0)
end

function Vec2_Abs()
    local a = omath.Vec2.new(-3, -4):abs()
    assert(a.x == 3 and a.y == 4)
end
