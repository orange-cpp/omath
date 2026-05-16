local function approx(a, b, eps) return math.abs(a - b) < (eps or 1e-5) end

function Mat4_Constructor_default()
    local m = omath.Mat4.new()
    assert(m:row_count() == 4 and m:columns_count() == 4)
    assert(m:at(0, 0) == 0 and m:at(3, 3) == 0)
end

function Mat4_FromRows()
    local m = omath.Mat4.from_rows({
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16},
    })
    assert(m:at(0, 0) == 1 and m:at(3, 3) == 16)
end

function Mat4_SetAt()
    local m = omath.Mat4.new()
    m:set_at(2, 3, 42)
    assert(m:at(2, 3) == 42)
end

function Mat4_SetAndClear()
    local m = omath.Mat4.new()
    m:set(2)
    assert(m:sum() == 32)
    m:clear()
    assert(m:sum() == 0)
end

function Mat4_Multiplication_matrix()
    local identity = omath.Mat4.from_rows({
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    })
    local m = omath.Mat4.from_rows({
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16},
    })
    local result = m * identity
    assert(result == m)
end

function Mat4_Multiplication_scalar()
    local m = omath.Mat4.from_rows({
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    }) * 2
    assert(m:at(0, 0) == 2 and m:at(3, 3) == 2)
end

function Mat4_Multiplication_scalar_reversed()
    local m = 2 * omath.Mat4.from_rows({
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    })
    assert(m:at(0, 0) == 2 and m:at(3, 3) == 2)
end

function Mat4_Division_scalar()
    local m = omath.Mat4.from_rows({
        {2, 0, 0, 0},
        {0, 2, 0, 0},
        {0, 0, 2, 0},
        {0, 0, 0, 2},
    }) / 2
    assert(m:at(0, 0) == 1 and m:at(3, 3) == 1)
end

function Mat4_Transposed()
    local m = omath.Mat4.from_rows({
        {1, 2, 0, 0},
        {3, 4, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    }):transposed()
    assert(m:at(0, 1) == 3 and m:at(1, 0) == 2)
end

function Mat4_Determinant()
    local m = omath.Mat4.from_rows({
        {1, 0, 0, 0},
        {0, 2, 0, 0},
        {0, 0, 3, 0},
        {0, 0, 0, 4},
    })
    assert(m:determinant() == 24)
end

function Mat4_Inverted_success()
    local m = omath.Mat4.from_rows({
        {2, 0, 0, 0},
        {0, 2, 0, 0},
        {0, 0, 2, 0},
        {0, 0, 0, 2},
    })
    local inv = m:inverted()
    assert(inv ~= nil)
    assert(approx(inv:at(0, 0), 0.5) and approx(inv:at(3, 3), 0.5))
end

function Mat4_Inverted_singular()
    local m = omath.Mat4.new()
    assert(m:inverted() == nil)
end

function Mat4_ToScreenMat()
    local m = omath.Mat4.to_screen_mat(800, 600)
    assert(m:at(0, 0) == 400 and m:at(1, 1) == -300 and m:at(3, 1) == 300)
end

function Mat4_Translation()
    local m = omath.Mat4.translation(omath.Vec3.new(1, 2, 3))
    assert(m:at(0, 3) == 1 and m:at(1, 3) == 2 and m:at(2, 3) == 3)
end

function Mat4_Scale()
    local m = omath.Mat4.scale(omath.Vec3.new(2, 3, 4))
    assert(m:at(0, 0) == 2 and m:at(1, 1) == 3 and m:at(2, 2) == 4)
end

function Mat4_Perspective()
    local m = omath.Mat4.perspective_left_handed_vertical_fov(90, 16 / 9, 0.1, 1000)
    assert(m:at(0, 0) > 0 and m:at(1, 1) > 0)
end

function Mat4_AsTable()
    local m = omath.Mat4.from_rows({
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16},
    })
    local t = m:as_table()
    assert(t[1][1] == 1 and t[4][4] == 16)
end

function Mat4_ToString()
    local m = omath.Mat4.from_rows({
        {1, 2, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    })
    assert(string.find(tostring(m), "%[%[") ~= nil)
end

function Mat3_FromRows()
    local m = omath.Mat3.from_rows({
        {1, 2, 3},
        {0, 1, 4},
        {5, 6, 0},
    })
    assert(m:determinant() == 1)
end

function Mat4ColumnMajor_FromRows()
    local m = omath.Mat4ColumnMajor.from_rows({
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16},
    })
    assert(m:at(0, 1) == 2 and m:at(3, 2) == 15)
end

function Mat4ColumnMajor_ToScreenMat()
    local m = omath.Mat4ColumnMajor.to_screen_mat(800, 600)
    assert(m:at(0, 0) == 400 and m:at(1, 1) == -300 and m:at(3, 1) == 300)
end

function Mat4ColumnMajor_Translation()
    local m = omath.Mat4ColumnMajor.translation(omath.Vec3.new(1, 2, 3))
    assert(m:at(0, 3) == 1 and m:at(1, 3) == 2 and m:at(2, 3) == 3)
end

function Mat4d_FromRows()
    local m = omath.Mat4d.from_rows({
        {1, 0, 0, 0},
        {0, 2, 0, 0},
        {0, 0, 3, 0},
        {0, 0, 0, 4},
    })
    assert(m:determinant() == 24)
end
