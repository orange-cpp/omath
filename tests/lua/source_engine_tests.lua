local function approx(a, b, eps) return math.abs(a - b) < (eps or 1e-4) end

local function make_camera()
    local pos    = omath.Vec3.new(0, 0, 0)
    local pitch  = omath.source.PitchAngle.from_degrees(0)
    local yaw    = omath.source.YawAngle.from_degrees(0)
    local roll   = omath.source.RollAngle.from_degrees(0)
    local angles = omath.source.ViewAngles.new(pitch, yaw, roll)
    local vp     = omath.opengl.ViewPort.new(1920, 1080)
    local fov    = omath.source.FieldOfView.from_degrees(90)
    return omath.source.Camera.new(pos, angles, vp, fov, 0.1, 1000)
end

-- PitchAngle
function Source_PitchAngle_from_degrees()
    assert(omath.source.PitchAngle.from_degrees(45):as_degrees() == 45)
end

function Source_PitchAngle_clamping_max()
    assert(omath.source.PitchAngle.from_degrees(100):as_degrees() == 89)
end

function Source_PitchAngle_clamping_min()
    assert(omath.source.PitchAngle.from_degrees(-100):as_degrees() == -89)
end

function Source_PitchAngle_from_radians()
    assert(approx(omath.source.PitchAngle.from_radians(math.pi / 4):as_degrees(), 45))
end

function Source_PitchAngle_as_radians()
    assert(approx(omath.source.PitchAngle.from_degrees(0):as_radians(), 0))
end

function Source_PitchAngle_sin()
    assert(approx(omath.source.PitchAngle.from_degrees(30):sin(), 0.5))
end

function Source_PitchAngle_cos()
    assert(approx(omath.source.PitchAngle.from_degrees(60):cos(), 0.5))
end

function Source_PitchAngle_tan()
    assert(approx(omath.source.PitchAngle.from_degrees(45):tan(), 1.0))
end

function Source_PitchAngle_addition()
    local c = omath.source.PitchAngle.from_degrees(20) + omath.source.PitchAngle.from_degrees(15)
    assert(c:as_degrees() == 35)
end

function Source_PitchAngle_addition_clamped()
    local c = omath.source.PitchAngle.from_degrees(80) + omath.source.PitchAngle.from_degrees(20)
    assert(c:as_degrees() == 89)
end

function Source_PitchAngle_subtraction()
    local c = omath.source.PitchAngle.from_degrees(50) - omath.source.PitchAngle.from_degrees(20)
    assert(c:as_degrees() == 30)
end

function Source_PitchAngle_unary_minus()
    assert((-omath.source.PitchAngle.from_degrees(45)):as_degrees() == -45)
end

function Source_PitchAngle_equal_to()
    local a = omath.source.PitchAngle.from_degrees(45)
    assert(a == omath.source.PitchAngle.from_degrees(45))
    assert(not (a == omath.source.PitchAngle.from_degrees(30)))
end

function Source_PitchAngle_to_string()
    assert(tostring(omath.source.PitchAngle.from_degrees(45)) == "45deg")
end

-- YawAngle
function Source_YawAngle_from_degrees()
    assert(omath.source.YawAngle.from_degrees(90):as_degrees() == 90)
end

function Source_YawAngle_normalization()
    assert(approx(omath.source.YawAngle.from_degrees(200):as_degrees(), -160))
end

-- RollAngle
function Source_RollAngle_from_degrees()
    assert(omath.source.RollAngle.from_degrees(45):as_degrees() == 45)
end

-- FieldOfView
function Source_FieldOfView_from_degrees()
    assert(omath.source.FieldOfView.from_degrees(90):as_degrees() == 90)
end

function Source_FieldOfView_clamping()
    assert(omath.source.FieldOfView.from_degrees(200):as_degrees() == 180)
end

-- ViewAngles
function Source_ViewAngles_new()
    local angles = omath.source.ViewAngles.new(
        omath.source.PitchAngle.from_degrees(30),
        omath.source.YawAngle.from_degrees(90),
        omath.source.RollAngle.from_degrees(0))
    assert(angles.pitch:as_degrees() == 30)
    assert(angles.yaw:as_degrees() == 90)
    assert(angles.roll:as_degrees() == 0)
end

function Source_ViewAngles_field_mutation()
    local angles = omath.source.ViewAngles.new(
        omath.source.PitchAngle.from_degrees(0),
        omath.source.YawAngle.from_degrees(0),
        omath.source.RollAngle.from_degrees(0))
    angles.pitch = omath.source.PitchAngle.from_degrees(45)
    assert(angles.pitch:as_degrees() == 45)
end

-- Camera
function Source_Camera_constructor()
    assert(make_camera() ~= nil)
end

function Source_Camera_get_set_origin()
    local cam = make_camera()
    cam:set_origin(omath.Vec3.new(1, 2, 3))
    local o = cam:get_origin()
    assert(approx(o.x, 1) and approx(o.y, 2) and approx(o.z, 3))
end

function Source_Camera_get_set_near_plane()
    local cam = make_camera()
    cam:set_near_plane(0.5)
    assert(approx(cam:get_near_plane(), 0.5))
end

function Source_Camera_get_set_far_plane()
    local cam = make_camera()
    cam:set_far_plane(500)
    assert(approx(cam:get_far_plane(), 500))
end

function Source_Camera_get_set_fov()
    local cam = make_camera()
    cam:set_field_of_view(omath.source.FieldOfView.from_degrees(60))
    assert(approx(cam:get_field_of_view():as_degrees(), 60))
end

function Source_Camera_get_set_view_angles()
    local cam = make_camera()
    cam:set_view_angles(omath.source.ViewAngles.new(
        omath.source.PitchAngle.from_degrees(30),
        omath.source.YawAngle.from_degrees(90),
        omath.source.RollAngle.from_degrees(0)))
    assert(approx(cam:get_view_angles().pitch:as_degrees(), 30))
    assert(approx(cam:get_view_angles().yaw:as_degrees(), 90))
end

function Source_Camera_look_at()
    local cam = make_camera()
    cam:look_at(omath.Vec3.new(10, 0, 0))
    assert(cam:get_view_angles() ~= nil)
end

function Source_Camera_get_forward()
    local fwd = make_camera():get_forward()
    assert(approx(fwd:length(), 1.0))
end

function Source_Camera_get_right()
    assert(approx(make_camera():get_right():length(), 1.0))
end

function Source_Camera_get_up()
    assert(approx(make_camera():get_up():length(), 1.0))
end

function Source_Camera_world_to_screen_success()
    local cam = make_camera()
    cam:look_at(omath.Vec3.new(1, 0, 0))
    local screen, err = cam:world_to_screen(omath.Vec3.new(5, 0, 0))
    assert(screen ~= nil, "expected screen pos, got: " .. tostring(err))
end

function Source_Camera_world_to_screen_error()
    local cam = make_camera()
    cam:look_at(omath.Vec3.new(1, 0, 0))
    local screen, err = cam:world_to_screen(omath.Vec3.new(-100, 0, 0))
    assert(screen == nil and err ~= nil)
end

function Source_Camera_screen_to_world()
    local cam = make_camera()
    cam:look_at(omath.Vec3.new(1, 0, 0))
    local world, err = cam:screen_to_world(omath.Vec3.new(960, 540, 1))
    assert(world ~= nil, "expected world pos, got: " .. tostring(err))
end
