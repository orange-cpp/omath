local function approx(a, b, eps) return math.abs(a - b) < (eps or 1e-4) end

function Color_Constructor_float()
    local c = omath.Color.new(1, 0.5, 0.25, 1)
    assert(approx(c.r, 1) and approx(c.g, 0.5) and approx(c.b, 0.25) and approx(c.a, 1))
end

function Color_Constructor_default()
    local c = omath.Color.new()
    assert(c ~= nil)
end

function Color_Constructor_clamping()
    local c = omath.Color.new(2, -1, 0.5, 1)
    assert(approx(c.r, 1) and approx(c.g, 0) and approx(c.b, 0.5))
end

function Color_from_rgba()
    local c = omath.Color.from_rgba(255, 128, 0, 255)
    assert(approx(c.r, 1) and approx(c.g, 128/255) and approx(c.b, 0) and approx(c.a, 1))
end

function Color_from_hsv_components()
    local c = omath.Color.from_hsv(0, 1, 1)
    assert(approx(c.r, 1) and approx(c.g, 0) and approx(c.b, 0))
end

function Color_from_hsv_struct()
    local hsv = omath.Hsv.new()
    hsv.hue = 0
    hsv.saturation = 1
    hsv.value = 1
    local c = omath.Color.from_hsv(hsv)
    assert(approx(c.r, 1) and approx(c.g, 0) and approx(c.b, 0))
end

function Color_red()
    local c = omath.Color.red()
    assert(approx(c.r, 1) and approx(c.g, 0) and approx(c.b, 0) and approx(c.a, 1))
end

function Color_green()
    local c = omath.Color.green()
    assert(approx(c.r, 0) and approx(c.g, 1) and approx(c.b, 0) and approx(c.a, 1))
end

function Color_blue()
    local c = omath.Color.blue()
    assert(approx(c.r, 0) and approx(c.g, 0) and approx(c.b, 1) and approx(c.a, 1))
end

function Color_to_hsv()
    local hsv = omath.Color.red():to_hsv()
    assert(approx(hsv.hue, 0) and approx(hsv.saturation, 1) and approx(hsv.value, 1))
end

function Color_set_hue()
    local c = omath.Color.red()
    c:set_hue(1/3)
    assert(approx(c.g, 1, 1e-3))
end

function Color_set_saturation()
    local c = omath.Color.red()
    c:set_saturation(0)
    assert(approx(c.r, c.g) and approx(c.g, c.b))
end

function Color_set_value()
    local c = omath.Color.red()
    c:set_value(0)
    assert(approx(c.r, 0) and approx(c.g, 0) and approx(c.b, 0))
end

function Color_blend()
    local c = omath.Color.red():blend(omath.Color.blue(), 0.5)
    assert(approx(c.r, 0.5) and approx(c.b, 0.5))
end

function Color_blend_clamped_ratio()
    local c = omath.Color.red():blend(omath.Color.blue(), 2.0)
    assert(approx(c.r, 0) and approx(c.b, 1))
end

function Color_to_string()
    local s = tostring(omath.Color.red())
    assert(s == "[r:255, g:0, b:0, a:255]")
end

function Hsv_fields()
    local hsv = omath.Hsv.new()
    hsv.hue = 0.5
    hsv.saturation = 0.8
    hsv.value = 0.9
    assert(approx(hsv.hue, 0.5) and approx(hsv.saturation, 0.8) and approx(hsv.value, 0.9))
end
