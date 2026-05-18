local function approx(a, b, eps) return math.abs(a - b) < (eps or 1e-4) end

local function color() return omath.Color.new(1, 0, 0, 1) end
local function fill() return omath.Color.new(0, 0, 0, 0.5) end
local function outline() return omath.Color.new(0, 0, 0, 1) end
local function bg() return omath.Color.new(0.2, 0.2, 0.2, 1) end

local function renderer_with(commands)
    return omath.hud.Renderer.new({
        add_line = function(a, b, c, thickness)
            table.insert(commands, { kind = "line", a = a, b = b, color = c, thickness = thickness })
        end,
        add_polyline = function(points, c, thickness)
            table.insert(commands, { kind = "polyline", points = points, color = c, thickness = thickness })
        end,
        add_filled_polyline = function(points, c)
            table.insert(commands, { kind = "filled_polyline", points = points, color = c })
        end,
        add_rectangle = function(min, max, c)
            table.insert(commands, { kind = "rectangle", min = min, max = max, color = c })
        end,
        add_filled_rectangle = function(min, max, c)
            table.insert(commands, { kind = "filled_rectangle", min = min, max = max, color = c })
        end,
        add_circle = function(center, radius, c, thickness, segments)
            table.insert(commands, {
                kind = "circle",
                center = center,
                radius = radius,
                color = c,
                thickness = thickness,
                segments = segments,
            })
        end,
        add_filled_circle = function(center, radius, c, segments)
            table.insert(commands, { kind = "filled_circle", center = center, radius = radius, color = c, segments = segments })
        end,
        add_arc = function(center, radius, a_min, a_max, c, thickness, segments)
            table.insert(commands, {
                kind = "arc",
                center = center,
                radius = radius,
                a_min = a_min,
                a_max = a_max,
                color = c,
                thickness = thickness,
                segments = segments,
            })
        end,
        add_image = function(texture, min, max, tint)
            table.insert(commands, { kind = "image", texture = texture, min = min, max = max, tint = tint })
        end,
        add_text = function(pos, c, text)
            table.insert(commands, { kind = "text", pos = pos, color = c, text = text })
        end,
        calc_text_size = function(text)
            return omath.Vec2.new(#text * 6, 10)
        end,
    })
end

local function overlay(commands)
    return omath.hud.EntityOverlay.new(omath.Vec2.new(100, 10), omath.Vec2.new(100, 50), renderer_with(commands))
end

function Hud_CanvasBox_default_ratio()
    local box = omath.hud.CanvasBox.new(omath.Vec2.new(100, 10), omath.Vec2.new(100, 50))
    assert(approx(box.top_left_corner.x, 90) and approx(box.top_left_corner.y, 10))
    assert(approx(box.top_right_corner.x, 110) and approx(box.bottom_right_corner.y, 50))
end

function Hud_CanvasBox_custom_ratio()
    local box = omath.hud.CanvasBox.new(omath.Vec2.new(100, 10), omath.Vec2.new(100, 50), 2)
    assert(approx(box.top_left_corner.x, 80) and approx(box.bottom_right_corner.x, 120))
end

function Hud_CanvasBox_as_table()
    local points = omath.hud.CanvasBox.new(omath.Vec2.new(100, 10), omath.Vec2.new(100, 50)):as_table()
    assert(#points == 4)
    assert(approx(points[1].x, 90) and approx(points[3].x, 110))
end

function Hud_Renderer_callbacks()
    local commands = {}
    local renderer = renderer_with(commands)
    renderer:add_line(omath.Vec2.new(1, 2), omath.Vec2.new(3, 4), color(), 2)
    renderer:add_text(omath.Vec2.new(5, 6), color(), "hp")
    local size = renderer:calc_text_size("abcd")
    assert(#commands == 2)
    assert(commands[1].kind == "line" and commands[2].text == "hp")
    assert(size.x == 24 and size.y == 10)
end

function Hud_Renderer_polyline_table()
    local commands = {}
    local renderer = renderer_with(commands)
    renderer:add_polyline({ omath.Vec2.new(1, 1), omath.Vec2.new(2, 2) }, color(), 3)
    assert(commands[1].kind == "polyline")
    assert(#commands[1].points == 2 and commands[1].points[2].x == 2)
end

function Hud_Renderer_circle_defaults()
    local commands = {}
    local renderer = renderer_with(commands)
    renderer:add_circle(omath.Vec2.new(1, 2), 5, color(), 2)
    renderer:add_filled_circle(omath.Vec2.new(3, 4), 6, color())
    assert(commands[1].kind == "circle" and commands[1].segments == 0)
    assert(commands[2].kind == "filled_circle" and commands[2].segments == 0)
end

function Hud_EntityOverlay_add_2d_box()
    local commands = {}
    overlay(commands):add_2d_box(color(), fill(), 2)
    assert(#commands == 2)
    assert(commands[1].kind == "polyline" and #commands[1].points == 4)
    assert(commands[2].kind == "filled_polyline")
end

function Hud_EntityOverlay_add_cornered_2d_box()
    local commands = {}
    overlay(commands):add_cornered_2d_box(color(), fill(), 0.25, 2)
    assert(#commands == 10)
    assert(commands[1].kind == "polyline" and commands[10].kind == "line")
end

function Hud_EntityOverlay_add_dashed_box()
    local commands = {}
    overlay(commands):add_dashed_box(color(), 8, 5, 1)
    assert(#commands > 4)
    assert(commands[1].kind == "line")
end

function Hud_EntityOverlay_add_bar()
    local commands = {}
    overlay(commands):add_right_bar(color(), outline(), bg(), 4, 0.5)
    assert(#commands == 3)
    assert(commands[1].kind == "filled_rectangle" and commands[2].kind == "filled_rectangle")
    assert(commands[3].kind == "rectangle")
end

function Hud_EntityOverlay_add_dashed_bar()
    local commands = {}
    overlay(commands):add_bottom_dashed_bar(color(), outline(), bg(), 4, 0.5, 8, 5)
    assert(#commands >= 3)
    assert(commands[1].kind == "filled_rectangle")
end

function Hud_EntityOverlay_add_label()
    local commands = {}
    overlay(commands):add_right_label(color(), 5, false, "HP")
    assert(#commands == 1)
    assert(commands[1].kind == "text" and commands[1].text == "HP")
    assert(approx(commands[1].pos.x, 115))
end

function Hud_EntityOverlay_add_outlined_label()
    local commands = {}
    overlay(commands):add_left_label(color(), 5, true, "HP")
    assert(#commands == 9)
    assert(commands[9].kind == "text" and commands[9].text == "HP")
end

function Hud_EntityOverlay_add_centered_label()
    local commands = {}
    overlay(commands):add_centered_top_label(color(), 5, false, "HP")
    assert(#commands == 1)
    assert(commands[1].kind == "text")
    assert(approx(commands[1].pos.x, 94))
end

function Hud_EntityOverlay_add_spaces()
    local commands = {}
    overlay(commands):add_right_space_vertical(10):add_right_label(color(), 5, false, "HP")
    assert(#commands == 1)
    assert(approx(commands[1].pos.y, 20))
end

function Hud_EntityOverlay_add_progress_ring()
    local commands = {}
    overlay(commands):add_right_progress_ring(color(), bg(), 6, 0.25, 2, 5, 16)
    assert(#commands == 2)
    assert(commands[1].kind == "circle" and commands[2].kind == "arc")
    assert(commands[2].segments == 16)
end

function Hud_EntityOverlay_add_progress_ring_defaults()
    local commands = {}
    overlay(commands):add_right_progress_ring(color(), bg(), 6, 0.25)
    assert(#commands == 2)
    assert(commands[1].thickness == 2 and commands[1].segments == 0)
end

function Hud_EntityOverlay_add_icon()
    local commands = {}
    overlay(commands):add_right_icon("texture-id", 8, 6, color(), 5)
    assert(#commands == 1)
    assert(commands[1].kind == "image" and commands[1].texture == "texture-id")
end

function Hud_EntityOverlay_add_snap_line()
    local commands = {}
    overlay(commands):add_snap_line(omath.Vec2.new(0, 0), color(), 2)
    assert(#commands == 1)
    assert(commands[1].kind == "line")
    assert(approx(commands[1].b.x, 100) and approx(commands[1].b.y, 50))
end

function Hud_EntityOverlay_add_skeleton()
    local commands = {}
    overlay(commands):add_skeleton(color())
    assert(#commands == 15)
    assert(commands[1].kind == "line")
end

function Hud_EntityOverlay_add_scan_marker()
    local commands = {}
    overlay(commands):add_scan_marker(color(), outline(), 2)
    assert(#commands == 2)
    assert(commands[1].kind == "filled_polyline" and #commands[1].points == 3)
    assert(commands[2].kind == "polyline" and commands[2].thickness == 2)
end

function Hud_EntityOverlay_add_aim_dot()
    local commands = {}
    overlay(commands):add_aim_dot(omath.Vec2.new(5, 6), color())
    assert(#commands == 1)
    assert(commands[1].kind == "filled_circle" and commands[1].radius == 3)
end

function Hud_EntityOverlay_add_projectile_aim_circle()
    local commands = {}
    overlay(commands):add_projectile_aim(omath.Vec2.new(5, 6), color(), 4, 2, omath.hud.ProjectileAimFigure.CIRCLE)
    assert(#commands == 2)
    assert(commands[1].kind == "line")
    assert(commands[2].kind == "filled_circle" and commands[2].radius == 4)
end

function Hud_EntityOverlay_add_projectile_aim_square_default()
    local commands = {}
    overlay(commands):add_projectile_aim(omath.Vec2.new(5, 6), color())
    assert(#commands == 2)
    assert(commands[1].kind == "line")
    assert(commands[2].kind == "filled_rectangle")
end
