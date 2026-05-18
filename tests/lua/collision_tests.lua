local function approx(a, b, eps) return math.abs(a - b) < (eps or 1e-4) end

local function cube_points()
    return {
        omath.Vec3.new(-1, -1, -1),
        omath.Vec3.new(1, -1, -1),
        omath.Vec3.new(-1, 1, -1),
        omath.Vec3.new(1, 1, -1),
        omath.Vec3.new(-1, -1, 1),
        omath.Vec3.new(1, -1, 1),
        omath.Vec3.new(-1, 1, 1),
        omath.Vec3.new(1, 1, 1),
    }
end

function Collision_Aabb_constructor_and_fields()
    local aabb = omath.primitives.Aabb.new(omath.Vec3.new(-1, -2, -3), omath.Vec3.new(1, 2, 3))
    assert(aabb.min.x == -1 and aabb.max.z == 3)
    aabb.max = omath.Vec3.new(2, 3, 4)
    assert(aabb.max.x == 2 and aabb.max.y == 3 and aabb.max.z == 4)
end

function Collision_Aabb_center_extents()
    local aabb = omath.primitives.Aabb.new(omath.Vec3.new(-1, -2, -3), omath.Vec3.new(3, 6, 9))
    local center = aabb:center()
    local extents = aabb:extents()
    assert(center.x == 1 and center.y == 2 and center.z == 3)
    assert(extents.x == 2 and extents.y == 4 and extents.z == 6)
end

function Collision_Aabb_top_bottom_default_axis()
    local aabb = omath.primitives.Aabb.new(omath.Vec3.new(-1, -2, -3), omath.Vec3.new(3, 6, 9))
    assert(aabb:top().y == 6)
    assert(aabb:bottom().y == -2)
end

function Collision_Aabb_top_bottom_explicit_axis()
    local aabb = omath.primitives.Aabb.new(omath.Vec3.new(-1, -2, -3), omath.Vec3.new(3, 6, 9))
    assert(aabb:top(omath.primitives.UpAxis.X).x == 3)
    assert(aabb:bottom(omath.primitives.UpAxis.Z).z == -3)
end

function Collision_Aabb_is_collide()
    local a = omath.primitives.Aabb.new(omath.Vec3.new(-1, -1, -1), omath.Vec3.new(1, 1, 1))
    local b = omath.primitives.Aabb.new(omath.Vec3.new(0, 0, 0), omath.Vec3.new(2, 2, 2))
    local c = omath.primitives.Aabb.new(omath.Vec3.new(3, 3, 3), omath.Vec3.new(4, 4, 4))
    assert(a:is_collide(b))
    assert(not a:is_collide(c))
end

function Collision_Aabb_as_table()
    local aabb = omath.primitives.Aabb.new(omath.Vec3.new(-1, -2, -3), omath.Vec3.new(1, 2, 3))
    local t = aabb:as_table()
    assert(t.min.x == -1 and t.max.z == 3)
end

function Collision_Obb_constructor_and_vertices()
    local obb = omath.primitives.Obb.new(
        omath.Vec3.new(0, 0, 0),
        omath.Vec3.new(1, 0, 0),
        omath.Vec3.new(0, 1, 0),
        omath.Vec3.new(0, 0, 1),
        omath.Vec3.new(1, 2, 3)
    )
    local vertices = obb:vertices()
    assert(#vertices == 8)
    assert(vertices[1].x == -1 and vertices[1].y == -2 and vertices[1].z == -3)
    assert(vertices[8].x == 1 and vertices[8].y == 2 and vertices[8].z == 3)
end

function Collision_Ray_constructor_and_direction()
    local ray = omath.collision.Ray.new(omath.Vec3.new(1, 2, 3), omath.Vec3.new(4, 6, 3), true)
    local dir = ray:direction_vector()
    local normal = ray:direction_vector_normalized()
    assert(ray.infinite_length)
    assert(dir.x == 3 and dir.y == 4 and dir.z == 0)
    assert(approx(normal:length(), 1))
end

function Collision_LineTracer_triangle_hit()
    local tri = omath.Triangle.new(omath.Vec3.new(0, 0, 0), omath.Vec3.new(2, 0, 0), omath.Vec3.new(0, 2, 0))
    local ray = omath.collision.Ray.new(omath.Vec3.new(0.5, 0.5, -1), omath.Vec3.new(0.5, 0.5, 1))
    local hit = omath.collision.LineTracer.get_ray_hit_point(ray, tri)
    assert(omath.collision.LineTracer.ray_hits_triangle(ray, tri))
    assert(not omath.collision.LineTracer.can_trace_line(ray, tri))
    assert(approx(hit.x, 0.5) and approx(hit.y, 0.5) and approx(hit.z, 0))
end

function Collision_LineTracer_triangle_miss()
    local tri = omath.Triangle.new(omath.Vec3.new(0, 0, 0), omath.Vec3.new(1, 0, 0), omath.Vec3.new(0, 1, 0))
    local ray = omath.collision.Ray.new(omath.Vec3.new(2, 2, -1), omath.Vec3.new(2, 2, 1))
    local hit = omath.collision.LineTracer.get_ray_hit_point(ray, tri)
    assert(not omath.collision.LineTracer.ray_hits_triangle(ray, tri))
    assert(omath.collision.LineTracer.can_trace_line(ray, tri))
    assert(hit == ray["end"])
end

function Collision_LineTracer_aabb_hit()
    local aabb = omath.primitives.Aabb.new(omath.Vec3.new(-1, -1, -1), omath.Vec3.new(1, 1, 1))
    local ray = omath.collision.Ray.new(omath.Vec3.new(0, 0, -3), omath.Vec3.new(0, 0, 3))
    local hit = omath.collision.LineTracer.get_ray_hit_point(ray, aabb)
    assert(omath.collision.LineTracer.ray_hits_aabb(ray, aabb))
    assert(approx(hit.x, 0) and approx(hit.y, 0) and approx(hit.z, -1))
end

function Collision_LineTracer_aabb_miss()
    local aabb = omath.primitives.Aabb.new(omath.Vec3.new(-1, -1, -1), omath.Vec3.new(1, 1, 1))
    local ray = omath.collision.Ray.new(omath.Vec3.new(0, 3, -3), omath.Vec3.new(0, 3, 3))
    local hit = omath.collision.LineTracer.get_ray_hit_point(ray, aabb)
    assert(not omath.collision.LineTracer.ray_hits_aabb(ray, aabb))
    assert(hit == ray["end"])
end

function Collision_LineTracer_obb_hit()
    local obb = omath.primitives.Obb.new(
        omath.Vec3.new(0, 0, 0),
        omath.Vec3.new(1, 0, 0),
        omath.Vec3.new(0, 1, 0),
        omath.Vec3.new(0, 0, 1),
        omath.Vec3.new(1, 1, 1)
    )
    local ray = omath.collision.Ray.new(omath.Vec3.new(0, 0, -3), omath.Vec3.new(0, 0, 3))
    local hit = omath.collision.LineTracer.get_ray_hit_point(ray, obb)
    assert(omath.collision.LineTracer.ray_hits_obb(ray, obb))
    assert(approx(hit.z, -1))
end

function Collision_ConvexCollider_vertices_and_support()
    local collider = omath.collision.ConvexCollider.new(cube_points(), omath.Vec3.new(2, 0, 0))
    assert(collider:vertex_count() == 8)
    assert(#collider:vertices() == 8)
    local support = collider:find_abs_furthest_vertex_position(omath.Vec3.new(1, 0, 0))
    assert(support.x == 3)
end

function Collision_Gjk_support_vertex()
    local a = omath.collision.ConvexCollider.new(cube_points())
    local b = omath.collision.ConvexCollider.new(cube_points())
    local support = omath.collision.gjk_support_vertex(a, b, omath.Vec3.new(1, 0, 0))
    assert(support.x == 2 and support.y == 0 and support.z == 0)
end

function Collision_Gjk_collide_true()
    local a = omath.collision.ConvexCollider.new(cube_points())
    local b = omath.collision.ConvexCollider.new(cube_points(), omath.Vec3.new(1, 0, 0))
    assert(omath.collision.gjk_collide(a, b))
end

function Collision_Gjk_collide_false()
    local a = omath.collision.ConvexCollider.new(cube_points())
    local b = omath.collision.ConvexCollider.new(cube_points(), omath.Vec3.new(5, 0, 0))
    assert(not omath.collision.gjk_collide(a, b))
end

function Collision_Epa_solve_hit()
    local a = omath.collision.ConvexCollider.new(cube_points())
    local b = omath.collision.ConvexCollider.new(cube_points(), omath.Vec3.new(1.5, 0, 0))
    local result = omath.collision.epa_solve(a, b)
    assert(result ~= nil)
    assert(result.depth > 0)
    assert(approx(result.normal:length(), 1))
    assert(approx(result.penetration_vector:length(), result.depth))
end

function Collision_Epa_solve_miss()
    local a = omath.collision.ConvexCollider.new(cube_points())
    local b = omath.collision.ConvexCollider.new(cube_points(), omath.Vec3.new(5, 0, 0))
    assert(omath.collision.epa_solve(a, b) == nil)
end
