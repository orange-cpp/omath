local function approx(a, b, eps) return math.abs(a - b) < (eps or 1e-5) end

function Quaternion_Constructor_default()
    local q = omath.Quaternion.new()
    assert(q.x == 0 and q.y == 0 and q.z == 0 and q.w == 1)
end

function Quaternion_Constructor_xyzw()
    local q = omath.Quaternion.new(1, 2, 3, 4)
    assert(q.x == 1 and q.y == 2 and q.z == 3 and q.w == 4)
end

function Quaternion_Field_mutation()
    local q = omath.Quaternion.new()
    q.x = 1; q.y = 2; q.z = 3; q.w = 4
    assert(q.x == 1 and q.y == 2 and q.z == 3 and q.w == 4)
end

function Quaternion_FromAxisAngle_zero_angle()
    local q = omath.Quaternion.from_axis_angle(omath.Vec3.new(0, 0, 1), 0)
    assert(approx(q.x, 0) and approx(q.y, 0) and approx(q.z, 0) and approx(q.w, 1))
end

function Quaternion_Addition()
    local q = omath.Quaternion.new(1, 2, 3, 4) + omath.Quaternion.new(4, 3, 2, 1)
    assert(q.x == 5 and q.y == 5 and q.z == 5 and q.w == 5)
end

function Quaternion_Multiplication_scalar()
    local q = omath.Quaternion.new(1, 2, 3, 4) * 2
    assert(q.x == 2 and q.y == 4 and q.z == 6 and q.w == 8)
end

function Quaternion_Multiplication_scalar_reversed()
    local q = 2 * omath.Quaternion.new(1, 2, 3, 4)
    assert(q.x == 2 and q.y == 4 and q.z == 6 and q.w == 8)
end

function Quaternion_Multiplication_quaternion()
    local i = omath.Quaternion.new(1, 0, 0, 0)
    local j = omath.Quaternion.new(0, 1, 0, 0)
    local k = i * j
    assert(k.x == 0 and k.y == 0 and k.z == 1 and k.w == 0)
end

function Quaternion_UnaryMinus()
    local q = -omath.Quaternion.new(1, -2, 3, -4)
    assert(q.x == -1 and q.y == 2 and q.z == -3 and q.w == 4)
end

function Quaternion_EqualTo_true()
    assert(omath.Quaternion.new(1, 2, 3, 4) == omath.Quaternion.new(1, 2, 3, 4))
end

function Quaternion_EqualTo_false()
    assert(not (omath.Quaternion.new(1, 2, 3, 4) == omath.Quaternion.new(1, 2, 3, 5)))
end

function Quaternion_ToString()
    assert(tostring(omath.Quaternion.new(1, 2, 3, 4)) == "Quaternion(1, 2, 3, 4)")
end

function Quaternion_Conjugate()
    local q = omath.Quaternion.new(1, 2, 3, 4):conjugate()
    assert(q.x == -1 and q.y == -2 and q.z == -3 and q.w == 4)
end

function Quaternion_Dot()
    assert(omath.Quaternion.new(1, 0, 0, 0):dot(omath.Quaternion.new(0, 1, 0, 0)) == 0)
end

function Quaternion_Length()
    assert(approx(omath.Quaternion.new(1, 1, 1, 1):length(), 2))
end

function Quaternion_LengthSqr()
    assert(omath.Quaternion.new(1, 2, 3, 4):length_sqr() == 30)
end

function Quaternion_Normalized()
    local q = omath.Quaternion.new(1, 1, 1, 1):normalized()
    assert(approx(q:length(), 1))
    assert(approx(q.x, 0.5) and approx(q.y, 0.5) and approx(q.z, 0.5) and approx(q.w, 0.5))
end

function Quaternion_Inverse()
    local q = omath.Quaternion.from_axis_angle(omath.Vec3.new(0, 0, 1), math.pi / 3)
    local identity = q * q:inverse()
    assert(approx(identity.x, 0) and approx(identity.y, 0) and approx(identity.z, 0) and approx(identity.w, 1))
end

function Quaternion_Rotate()
    local q = omath.Quaternion.from_axis_angle(omath.Vec3.new(0, 0, 1), math.pi / 2)
    local v = q:rotate(omath.Vec3.new(1, 0, 0))
    assert(approx(v.x, 0, 1e-4) and approx(v.y, 1, 1e-4) and approx(v.z, 0, 1e-4))
end

function Quaternion_ToRotationMatrix3()
    local m = omath.Quaternion.new():to_rotation_matrix3()
    assert(m:at(0, 0) == 1 and m:at(1, 1) == 1 and m:at(2, 2) == 1)
end

function Quaternion_ToRotationMatrix4()
    local m = omath.Quaternion.new():to_rotation_matrix4()
    assert(m:at(0, 0) == 1 and m:at(1, 1) == 1 and m:at(2, 2) == 1 and m:at(3, 3) == 1)
end

function Quaternion_AsTable()
    local t = omath.Quaternion.new(1, 2, 3, 4):as_table()
    assert(t.x == 1 and t.y == 2 and t.z == 3 and t.w == 4)
end
