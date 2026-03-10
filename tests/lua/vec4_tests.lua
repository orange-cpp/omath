local function approx(a, b) return math.abs(a - b) < 1e-5 end

function Vec4_Constructor_default()
    local v = omath.Vec4.new()
    assert(v.x == 0 and v.y == 0 and v.z == 0 and v.w == 0)
end

function Vec4_Constructor_xyzw()
    local v = omath.Vec4.new(1, 2, 3, 4)
    assert(v.x == 1 and v.y == 2 and v.z == 3 and v.w == 4)
end

function Vec4_Field_mutation()
    local v = omath.Vec4.new(1, 2, 3, 4)
    v.w = 99
    assert(v.w == 99)
end

function Vec4_Addition()
    local c = omath.Vec4.new(1, 2, 3, 4) + omath.Vec4.new(4, 3, 2, 1)
    assert(c.x == 5 and c.y == 5 and c.z == 5 and c.w == 5)
end

function Vec4_Subtraction()
    local c = omath.Vec4.new(5, 5, 5, 5) - omath.Vec4.new(1, 2, 3, 4)
    assert(c.x == 4 and c.y == 3 and c.z == 2 and c.w == 1)
end

function Vec4_UnaryMinus()
    local b = -omath.Vec4.new(1, 2, 3, 4)
    assert(b.x == -1 and b.y == -2 and b.z == -3 and b.w == -4)
end

function Vec4_Multiplication_scalar()
    local b = omath.Vec4.new(1, 2, 3, 4) * 2.0
    assert(b.x == 2 and b.y == 4 and b.z == 6 and b.w == 8)
end

function Vec4_Multiplication_scalar_reversed()
    local b = 2.0 * omath.Vec4.new(1, 2, 3, 4)
    assert(b.x == 2 and b.y == 4 and b.z == 6 and b.w == 8)
end

function Vec4_Multiplication_vec()
    local c = omath.Vec4.new(2, 3, 4, 5) * omath.Vec4.new(2, 2, 2, 2)
    assert(c.x == 4 and c.y == 6 and c.z == 8 and c.w == 10)
end

function Vec4_Division_scalar()
    local b = omath.Vec4.new(2, 4, 6, 8) / 2.0
    assert(b.x == 1 and b.y == 2 and b.z == 3 and b.w == 4)
end

function Vec4_Division_vec()
    local c = omath.Vec4.new(4, 6, 8, 10) / omath.Vec4.new(2, 2, 2, 2)
    assert(c.x == 2 and c.y == 3 and c.z == 4 and c.w == 5)
end

function Vec4_EqualTo_true()
    assert(omath.Vec4.new(1, 2, 3, 4) == omath.Vec4.new(1, 2, 3, 4))
end

function Vec4_EqualTo_false()
    assert(not (omath.Vec4.new(1, 2, 3, 4) == omath.Vec4.new(9, 9, 9, 9)))
end

function Vec4_LessThan()
    assert(omath.Vec4.new(1, 0, 0, 0) < omath.Vec4.new(0, 0, 3, 4))
end

function Vec4_LessThanOrEqual()
    -- (0,0,3,4) and (0,0,4,3) both have length 5
    assert(omath.Vec4.new(0, 0, 3, 4) <= omath.Vec4.new(0, 0, 4, 3))
end

function Vec4_ToString()
    assert(tostring(omath.Vec4.new(1, 2, 3, 4)) == "Vec4(1, 2, 3, 4)")
end

function Vec4_Length()
    assert(approx(omath.Vec4.new(0, 0, 3, 4):length(), 5.0))
end

function Vec4_LengthSqr()
    assert(omath.Vec4.new(0, 0, 3, 4):length_sqr() == 25.0)
end

function Vec4_Dot()
    local a = omath.Vec4.new(1, 2, 3, 4)
    assert(a:dot(a) == 30.0)
end

function Vec4_Dot_perpendicular()
    assert(omath.Vec4.new(1, 0, 0, 0):dot(omath.Vec4.new(0, 1, 0, 0)) == 0.0)
end

function Vec4_Sum()
    assert(omath.Vec4.new(1, 2, 3, 4):sum() == 10.0)
end

function Vec4_Abs()
    local a = omath.Vec4.new(-1, -2, -3, -4):abs()
    assert(a.x == 1 and a.y == 2 and a.z == 3 and a.w == 4)
end

function Vec4_Clamp()
    local v = omath.Vec4.new(5, -3, 10, 99)
    v:clamp(0, 7)
    assert(v.x == 5 and v.y == 0 and v.z == 7)
end
